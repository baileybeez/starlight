
#ifndef SERVER_H
#define SERVER_H

#define kBacklog 5
#define kPollTimout 5

struct SocketServer {
    int hServer;
};

int initializeSocketServer(struct SocketServer *server, int port);
int pollServer(struct SocketServer *server);
int serverHasIncomingConnection();
void closeServer(struct SocketServer *server);

#endif//SERVER_H
