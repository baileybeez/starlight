#include <stdio.h>
#include <string.h>
#include "../src/uri.h"

void testUri(const char *name, const char *str) {
    printf("********************************\nTEST: %s\nURI : %s\n\n", name, str);

    int len = strlen(str);
    if (len > kMaxUri_TotalLength) {
        printf("    !! URI too long. Length is %d, Max is %d.\n", len, kMaxUri_TotalLength);
    }
    
    struct Uri uri;
    memset(&uri, 0, sizeof(uri));
    int ret = parseUriFromRequest(str, &uri);
    if (ret != 1) {
        printf("    !! parseUriFromRequest failed! %d \n", ret);
    }

    printf("> Scheme :: %s \n", uri.scheme);
    printf("> Domain :: %s \n", uri.domain);
    printf("> Port   :: %d \n", uri.port);
    printf("> Path   :: %s \n", uri.path);
    printf("> User   :: %s \n", uri.user);
}

int main (int argc, char **argv) {
    const char *properUri = "gemini://mydomain.com:80/path/to/folder/file.gmi?user_state=1234";
    const char *simpleUri = "gemini://mydomain.com/";
    const char *localUri = "gemini://localhost";
    const char *htmlUri = "https://www.mydomain.com/path/to/folder/file.html?search";
    const char *invalidScheme = "this_is_an_invalid_scheme://mydomain.com:80/path/to/folder/file.gmi?user_state=1234";
    
    testUri("Proper Format", properUri);
    testUri("Simple Url", simpleUri);
    testUri("HTML Url", htmlUri);
    testUri("Localhost", localUri);
    testUri("Invalid Scheme", invalidScheme);
    
    return 0;
}