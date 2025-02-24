#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <openssl/ssl.h>

#include "uri.h"

#define kBacklog 5
#define kPollTimout 5
#define kGeminiURIMaxLen 1024
#define kMaxPath 256
#define kStarlightIni "./starlight.ini"

#define kIniKey_Port        "PORT"
#define kIniKey_CertPath    "CERT"
#define kIniKey_KeyPath     "KEY"
#define kIniKey_ContentRoot "ROOT"

#define kResponse_GeneralError   "50\r\n"

struct IniSettings {
    int port;
    char certPath[kMaxPath];
    char keyPath[kMaxPath];
    char contentRoot[kMaxPath];
};

static int g_running = 1;
void handleSignal(int signal) 
{
    printf("Received signal %d, shutting down.\n", signal);
    g_running = 0;
}

int loadIniSettings(struct IniSettings *ini) {
    char *line = NULL;
    size_t len = 0;
    char *key = NULL;
    char *val = NULL;
    int ret = 0;

    FILE *file = fopen(kStarlightIni, "r");
    if (file == NULL) {
        printf ("Unable to locate Ini file. Is it missing? \n");
        return -1;
    }

    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) {
        if (line[read - 1] == '\n')
            line[read - 1] = '\0';

        key = strtok(line, "=");
        val = strtok(NULL, "=");

        if (strcmp(key, kIniKey_Port) == 0) {
            int p = atoi(val);
            if (p < 0 || p > 65535) {
                printf("Invalid port specified: %s \n", val);
                goto INI_ERROR;
            }
            
            ini->port = p;
            printf("   > port set to '%d' \n", ini->port);
        } else if (strcmp(key, kIniKey_CertPath) == 0) {
            if (access(val, F_OK) == -1) {
                printf("Unable to locate certificate: %s \n", val);
                goto INI_ERROR;
            }
            strcpy(ini->certPath, val);
            printf("   > cert path set to '%s' \n", ini->certPath);
        } else if (strcmp(key, kIniKey_KeyPath) == 0) {
            if (access(val, F_OK) == -1) {
                printf("Unable to locate keyfile: %s \n", val);
                goto INI_ERROR;
            }
            strcpy(ini->keyPath, val);
            printf("   > key path set to '%s' \n", ini->keyPath);
        } else if (strcmp(key, kIniKey_ContentRoot) == 0) {
            struct stat statbuf;
            if (stat(val, &statbuf) == 0) {
                if (!S_ISDIR(statbuf.st_mode)) {
                    printf("Content Root '%s' does not appear to be a directory. \n", val);
                    goto INI_ERROR;
                }

                strcpy(ini->contentRoot, val);
                printf("   > content root set to '%s' \n", ini->contentRoot);
            } else {
                printf("Could not find Content Root: %s \n", val);
                goto INI_ERROR;
            }
        } else {
            printf("Unknown key in Ini settings: %s \n", key);
        }
    }

    ret = 1;
INI_ERROR:
    fclose(file);
    return ret;
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
    ret = loadIniSettings(&ini);
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