#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "std.h"
#include "ini.h"

int loadIniSettings(struct IniSettings *ini, const char *iniFilePath) {
    char *line = NULL;
    size_t len = 0;
    char *key = NULL;
    char *val = NULL;
    int ret = 0;

    FILE *file = fopen(iniFilePath, "r");
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