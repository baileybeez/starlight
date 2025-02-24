#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

#include "std.h"
#include "server.h"

static struct pollfd g_poll[1];

int initializeSocketServer(struct SocketServer *server, int port)
{
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
    addr.sin_port = htons(port);

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
    printf("Listening on port %d ... \n", port);

    g_poll[0].fd = server->hServer;
    g_poll[0].events = POLLIN;
    g_poll[0].revents = 0;

    return 1;
}

int pollServer(struct SocketServer *server)
{
    return poll(g_poll, 1, kPollTimout);
}

int serverHasIncomingConnection()
{
    return g_poll[0].revents & POLLIN;
}

void closeServer(struct SocketServer *server)
{
    close(server->hServer);
    server->hServer = nil;
}