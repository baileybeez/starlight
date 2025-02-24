#include <string.h>
#include <stdlib.h>

#include "uri.h"

#define kParseState_Scheme      1
#define kParseState_Domain      2
#define kParseState_Port        3
#define kParseState_Path        4
#define kParseState_UserState   5
#define kParseState_Done        6

// req should appear as `gemini://domain:port/path/to/folder/document.gmi?user_state`
int parseUriFromRequest(const char *req, struct Uri *uri)
{
    int lenRequest = strlen(req);
    if (lenRequest > kMaxUri_TotalLength) {
        return kUriParseError_InvalidLen;
    }

    int index = 0, start = 0, cc = 0;
    int parseState = kParseState_Scheme;
    char cb = '\0';
    char strPort[kMaxUri_Port + 1] = {0};
    while (++index < lenRequest && req[index] != '\0') {
        cb = req[index];
        cc = index - start;
        switch (parseState) {
            case kParseState_Scheme:
                // pulling all content prior to `://` as `scheme` of URI
                if (cc > kMaxUri_Scheme) {
                    return kUriParseError_InvalidSchemeLen;
                } else if (cb == ':' && index + 2 < lenRequest) {
                    if (req[index + 1] == '/' && req[index + 2] == '/') {
                        memcpy(uri->scheme, req + start, index);
                        uri->scheme[cc] = '\0';
                        index += 2;
                        start = index + 1;
                        parseState = kParseState_Domain;
                    }
                } 
                break;
            case kParseState_Domain:
                // pulling content until either a `:` (transfer to port) or `/` (transfer to path)
                if (cc > kMaxUri_Domain) {
                    return kUriParseError_InvalidDomainLen;
                } else if (cb == ':' || cb == '/') {
                    memcpy(uri->domain, req + start, cc);
                    uri->domain[cc] = '\0';
                    start = index + 1;
                    if (cb == ':') {
                        parseState = kParseState_Port;
                    } else {
                        parseState = kParseState_Path;
                        uri->port = kGemini_DefaultPort;
                    }
                }
                break;
            case kParseState_Port:
                // pulling content until a '/'
                if (cc > kMaxUri_Port) {
                    return kUriParseError_InvalidPortLen;
                } else if (cb == '/') {
                    memcpy(strPort, req + start, cc);
                    strPort[cc] = '\0';
                    uri->port = atoi(strPort);
                    if (uri->port < 1 || uri->port > 65535) {
                        return kUriParseError_InvalidPort;
                    }
                    start = index + 1;
                    parseState = kParseState_Path;
                }
                break;
            case kParseState_Path:
                // pulling content until either a '?' or the end of string
                if (cc > kMaxUri_Path) {
                    return kUriParseError_InvalidPathLen;
                } else if (cb == '?') {
                    memcpy(uri->path, req + start, cc);
                    uri->path[cc] = '\0';
                    start = index + 1;
                    parseState = kParseState_UserState;
                }
                break;
            case kParseState_UserState:
                // pull content until end of string
                if (cc > kMaxUri_UserState) {
                    return kUriParseError_InvalidUserLen;
                }
                break;
        }
    }

    int canPullAtEndOfLine = 0;
    if (parseState == kParseState_Path || parseState == kParseState_UserState)
        canPullAtEndOfLine = 1;

    if (canPullAtEndOfLine && index <= lenRequest) {
        cc = index - start;
        if (parseState == kParseState_Path) {
            memcpy(uri->path, req + start, cc);
            uri->path[cc] = '\0';
        } else if (parseState == kParseState_UserState) {
            memcpy(uri->user, req + start, cc);
            uri->user[cc] = '\0';
        }
        parseState = kParseState_Done;
    }

    return (parseState == kParseState_Done) ? 1 : 0;
}