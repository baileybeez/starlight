#ifndef INI_H
#define INI_H

#include "std.h"

#define kIniKey_Port        "PORT"
#define kIniKey_CertPath    "CERT"
#define kIniKey_KeyPath     "KEY"
#define kIniKey_ContentRoot "ROOT"

struct IniSettings {
    int port;
    char certPath[kMaxPath];
    char keyPath[kMaxPath];
    char contentRoot[kMaxPath];
};

int loadIniSettings(struct IniSettings *ini, const char *iniFilePath);

#endif//INI_H