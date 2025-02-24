#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "std.h"
#include "uri.h"
#include "ini.h"
#include "server.h"

#define kStarlightIni "./starlight.ini"

#define kResponse_GeneralError   "50\r\n"

static int g_running = 1;
void handleSignal(int signal) 
{
    printf("Received signal %d, shutting down.\n", signal);
    g_running = 0;
}

void handleRequest(const char *req, SSL *ssl);

int main(int argc, char **argv)
{
    struct IniSettings ini; 
    struct SocketServer server;
    int ret = 0;

    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);

    ret = loadIniSettings(&ini, kStarlightIni);
    if (ret != 1) {
        printf("Failed to load INI file (%d).\n", ret);
        return -1;
    }
    
    ret = initializeSocketServer(&server, &ini, handleRequest);
    if (ret != 1)
        return -1;

    while (g_running == 1) {
        int ret = pollServer(&server); // if < 0 .. error or request to exist
        if (ret < 0)
            break;

        sleep(0);
    }     

    closeServer(&server);
    return 0;
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
