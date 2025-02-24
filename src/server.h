
#ifndef SERVER_H
#define SERVER_H

#include <openssl/ssl.h>
#include <poll.h>
#include "ini.h"

#define kBacklog 5
#define kPollTimout 5

struct SocketServer {
    int hServer;
    SSL_CTX *ctx;
    SSL *ssl;
    char buffer[kGeminiURIMaxLen + 1];
    struct pollfd polling[1];
    void (*requestHandler)(const char *, SSL*);
};

int initializeSocketServer(struct SocketServer *server, struct IniSettings *ini, void (*requestHandler)(const char *, SSL*));
int pollServer(struct SocketServer *server);
int serverHasIncomingConnection(struct SocketServer *server);
void closeServer(struct SocketServer *server);

#endif//SERVER_H
