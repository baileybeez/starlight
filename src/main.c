#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>

#include "std.h"
#include "uri.h"
#include "util.h"
#include "ini.h"
#include "server.h"

#define kStarlightIni "./starlight.ini"
#define kReadBufferSize 1024

#define kResponse_GeneralError   "50\r\n"

static int g_running = 1;
void handleSignal(int signal) 
{
    printf("Received signal %d, shutting down.\n", signal);
    g_running = 0;
}

void handleRequest(const char *req, const char *contentRoot, SSL *ssl);

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

int parseRequestIntoPath(const char *req, const char *contentRoot, /*OUT*/ char *parsedPath)
{
    char *chunk = nil;
    char temp[kMaxUri_TotalLength];

    strcpy(temp, req);
    
    strcpy(parsedPath, contentRoot);
    chunk = strtok(temp, "/");
    while (chunk != nil) {
        strcat(parsedPath, chunk);

        struct stat buffer;
        if (stat(parsedPath, &buffer) != 0)
            return -1;

        strcat(parsedPath, "/");
        chunk = strtok(nil, "/");
    }

    // check if last chunk was a file
    if (!strEndsWith(parsedPath, ".gmi")) {
        if (!strEndsWith(parsedPath, "/"))
            strcat(parsedPath, "/");

        strcat(parsedPath, "index.gmi");
    }

    return 0;
}

void respondWithNotFound(SSL *ssl)
{
    // 51 (file not found)
    const char *msg = "51 Unable to locate request.\r\n";
    SSL_write(ssl, msg, (int)strlen(msg));
}

void handleRequest(const char *req, const char *contentRoot, SSL *ssl) 
{
    int ret = 0;
    struct Uri uri;
    char localPath[kMaxPath];

    memset(&uri, 0, sizeof(uri));
    ret = parseUriFromRequest(req, &uri);
    if (ret != 1) {
        SSL_write(ssl, kResponse_GeneralError, (int)strlen(kResponse_GeneralError));
        return;
    }

    ret = parseRequestIntoPath(uri.path, contentRoot, localPath);
    printf("parsed request: %s\n", localPath);
    if (ret < 0) {
        respondWithNotFound(ssl);
        return;
    }

    FILE *file = fopen(localPath, "r");
    if (file == nil) {
        respondWithNotFound(ssl);
        return;
    } 

    char buff[kReadBufferSize + 1];
    size_t read = 0;
    int wrote = 0;
    while ((read = fread(buff, 1, kReadBufferSize, file)) > 0) {
        wrote = SSL_write(ssl, buff, read);
    }

    fclose(file);
}
