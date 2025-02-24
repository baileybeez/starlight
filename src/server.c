#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "std.h"
#include "server.h"
#include "ini.h"

int initializeSocketServer(struct SocketServer *server, struct IniSettings *ini, void (*requestHandler)(const char *, SSL*))
{
    // clear server obj
    memset(server, 0, sizeof(server));

    // create socket
    server->hServer = socket(AF_INET, SOCK_STREAM, 0);
    if (server->hServer == -1) {
        printf("Socket creation failed. \n");
        return -1;
    }

    // define server addy
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(ini->port);

    // bind socket
    if (bind(server->hServer, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Bind failed.\n");
        closeServer(server);
        return -1;
    }

    // listen
    if (listen(server->hServer, kBacklog) < 0) {
        printf("Failed to start listening. \n");
        closeServer(server);
        return -1;
    }
    printf("Listening on port %d ... \n", ini->port);

    server->polling[0].fd = server->hServer;
    server->polling[0].events = POLLIN;
    server->polling[0].revents = 0;

    // setup SSL
    server->ctx = SSL_CTX_new(TLS_server_method());
    server->ssl = SSL_new(server->ctx);
    
    // "./starlight.crt"
    if (SSL_use_certificate_chain_file(server->ssl, ini->certPath) != 1) {
        printf("SSL_use_certificate_chain_file. \n");
        closeServer(server);
        return -1;
    }
    
    // "./starlight.key"
    if (SSL_use_PrivateKey_file(server->ssl, ini->keyPath, SSL_FILETYPE_PEM) != 1) {
        printf("SSL_use_PrivateKey_file. \n");
        closeServer(server);
        return -1;
    }

    server->requestHandler = requestHandler;
    return 1;
}

int pollServer(struct SocketServer *server)
{
    int ret = poll(server->polling, 1, kPollTimout);
    if (ret < 0)
        return -1;  // Error / Term Signal
    else if (ret == 0)
        return 0;

    if (serverHasIncomingConnection(server)) {
        // accept incoming
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int client_fd = accept(server->hServer, (struct sockaddr *)&clientAddr, &clientLen);
        if (client_fd < 0) {
            printf("Accept failed. \n");
            return 0;
        }
        printf("Client connected. \n");

        // clear mem
        memset(server->buffer, 0, kGeminiURIMaxLen);

        SSL_set_fd(server->ssl, client_fd);
        ret = SSL_accept(server->ssl);
        if (ret != 1) {
            int err = SSL_get_error(server->ssl, ret);
            printf("SSL_accept, %d \n", err);
            goto CLOSE_CONNECTION;
        }

        ret = SSL_read(server->ssl, server->buffer, kGeminiURIMaxLen);
        if (ret < 0) {
            printf("SSL_read \n");
            goto CLOSE_CONNECTION;
        }

        printf("RECV: %d bytes: %s \n", ret, server->buffer);
        server->requestHandler(server->buffer, server->ssl);

CLOSE_CONNECTION:
        close(client_fd);
    }

    return 1;
}

int serverHasIncomingConnection(struct SocketServer *server)
{
    return server->polling[0].revents & POLLIN;
}

void closeServer(struct SocketServer *server)
{
    if (server->ssl != nil) {
        SSL_shutdown(server->ssl);
        SSL_free(server->ssl);
        server->ssl = nil;
    }

    if (server->ctx != nil) {
        SSL_CTX_free(server->ctx);
        server->ctx = nil;
    }

    close(server->hServer);
    server->hServer = nil;
}