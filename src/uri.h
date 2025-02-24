#ifndef URI_H
#define URI_H

#define kGemini_DefaultPort 1965

#define kMaxUri_Scheme      16
#define kMaxUri_Domain      128
#define kMaxUri_Port        5
#define kMaxUri_Path        512
#define kMaxUri_UserState   363
#define kMaxUri_TotalLength 1024

#define kUriParseError_InvalidLen   -1
#define kUriParseError_InvalidSchemeLen -2
#define kUriParseError_InvalidDomainLen -3
#define kUriParseError_InvalidPortLen -4
#define kUriParseError_InvalidPathLen -5
#define kUriParseError_InvalidUserLen -6
#define kUriParseError_InvalidPort -7

struct Uri {
    char scheme[kMaxUri_Scheme];
    char domain[kMaxUri_Domain];
    int port;
    char path[kMaxUri_Path];
    char user[kMaxUri_UserState];
};

int parseUriFromRequest(const char *req, struct Uri *uri);

#endif//URI_H
