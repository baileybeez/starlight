#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <openssl/ssl.h>

#include "std.h"
#include "uri.h"
#include "ini.h"

#define kBacklog 5
#define kPollTimout 5
#define kStarlightIni "./starlight.ini"

#define kResponse_GeneralError   "50\r\n"

static int g_running = 1;
void handleSignal(int signal) 
{
    printf("Received signal %d, shutting down.\n", signal);
    g_running = 0;
}

int initializeSocketServer(int port)
{
    // create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed. \n");
        return -1;
    }

    // define server addy
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // bind socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Bind failed.\n");
        close(server_fd);
        return -1;
    }

    // listen
    if (listen(server_fd, kBacklog) < 0) {
        printf("Failed to start listening. \n");
        close(server_fd);
        return -1;
    }
    printf("Listening on port %d ... \n", port);

    return server_fd;
}

void handleRequest(const char *req, SSL *ssl) 
{
    struct Uri uri;
    memset(&uri, 0, sizeof(uri));

    int ret = parseUriFromRequest(req, &uri);
    if (ret != 1) {
        SSL_write(ssl, kResponse_GeneralError, (int)strlen(kResponse_GeneralError));
        return;
    }

    // locate requested info

    // debug msg
    const char *msg = "51 Unable to locate request.\r\n";
    SSL_write(ssl, msg, (int)strlen(msg));
}

int main(int argc, char **argv)
{
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    SSL_CTX *ctx;
    SSL *ssl;
    char buffer[kGeminiURIMaxLen + 1] = {0};
    int ret = 0;

    struct IniSettings ini; 
    ret = loadIniSettings(&ini, kStarlightIni);
    if (ret != 1) {
        printf("Failed to load INI file (%d).\n", ret);
        return -1;
    }

    int server_fd = initializeSocketServer(ini.port);
    if (server_fd == -1)
        return -1;
    
    // setup polling
    struct pollfd fd[1];
    fd[0].fd = server_fd;
    fd[0].events = POLLIN;
    fd[0].revents = 0;

    while (g_running == 1) {
        // poll socket for activity
        int ret = poll(fd, 1, kPollTimout);
        if (ret < 0)        // Error / Term
            break;
        else if (ret == 0)  // No work
            continue;
        
        if (fd[0].revents & POLLIN) {
            // accept incoming
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientLen);
            if (client_fd < 0) {
                printf("Accept failed. \n");
                continue;
            }
            printf("Client connected. \n");

            // clear mem
            memset(buffer, 0, kGeminiURIMaxLen);

            // SSL
            ctx = SSL_CTX_new(TLS_server_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client_fd);

            // "./starlight.crt"
            if (SSL_use_certificate_chain_file(ssl, ini.certPath) != 1) {
                printf("SSL_use_certificate_chain_file. \n");
                goto CLOSE_CONNECTION;
            }
            
            // "./starlight.key"
            if (SSL_use_PrivateKey_file(ssl, ini.keyPath, SSL_FILETYPE_PEM) != 1) {
                printf("SSL_use_PrivateKey_file. \n");
                goto CLOSE_CONNECTION;
            }

            ret = SSL_accept(ssl);
            if (ret != 1) {
                int err = SSL_get_error(ssl, ret);
                printf("SSL_accept, %d \n", err);
                goto CLOSE_CONNECTION;
            }

            ret = SSL_read(ssl, buffer, kGeminiURIMaxLen);
            if (ret < 0) {
                printf("SSL_read \n");
                goto CLOSE_CONNECTION;
            }

            printf("RECV: %d bytes: %s \n", ret, buffer);
            handleRequest(buffer, ssl);
            close(client_fd);

CLOSE_CONNECTION:
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(client_fd);
        }
    }     

    // close out server
    close(server_fd);
    return 0;
}