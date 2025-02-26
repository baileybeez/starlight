#include <stdio.h>
#include <string.h>
#include "../src/uri.h"
#include "../src/util.h"

#include "simpleTests.h"

void endsWithTest(const char *testName, const char *str, const char *sfx, int expected)
{
    int ret = strEndsWith(str, sfx);
    if (expected == 0) {
        assertAreEqual(ret, expected, testName, "EndsWith passed but was expected to fail");
    } else {
        assertAreEqual(ret, expected, testName, "EndsWith didn't find expected end");
    }
    testPassed(testName);
}

void testUri(const char *testName, const char *request, const char *scheme, const char *domain, int port, const char *path, const char *user) 
{
    assertLessThan(strlen(request), kMaxUri_TotalLength, testName, "request string too long!");

    struct Uri uri;
    memset(&uri, 0, sizeof(uri));

    int ret = parseUriFromRequest(request, &uri);
    assertAreEqual(ret, 1, nil, "Failed to parse URI from request");
    
    assertAreEqual(uri.scheme, scheme, testName, "wrong scheme");
    assertAreEqual(uri.domain, domain, testName, "wrong domain");
    assertAreEqual(uri.port, port, testName, "wrong port");
    assertAreEqual(uri.path, path, testName, "wrong path");
    assertAreEqual(uri.user, user, testName, "wrong user state");

    testPassed(testName);
}

void testUriFailParse(const char *testName, const char *request)
{
    assertLessThan(strlen(request), kMaxUri_TotalLength, testName, "request string too long!");

    struct Uri uri;
    memset(&uri, 0, sizeof(uri));

    int ret = parseUriFromRequest(request, &uri);
    assertLessThan(ret, 0, nil, "Parse URI was expected to fail.");
    testPassed(testName);
}

int main (int argc, char **argv) {
    const char *properUri = "gemini://mydomain.com:80/path/to/folder/file.gmi?user_state=1234";
    const char *simpleUri = "gemini://mydomain.com/";
    const char *localUri = "gemini://localhost";
    const char *noSuffixUri = "gemini://mydomain.com";
    const char *httpsUri = "https://www.mydomain.com/path/to/folder/file.html?search";
    const char *invalidScheme = "this_is_an_invalid_scheme://mydomain.com:80/path/to/folder/file.gmi?user_state=1234";
    
    testUri("URI - Proper", properUri, "gemini", "mydomain.com", 80, "path/to/folder/file.gmi", "user_state=1234");
    testUri("URI - Simple", simpleUri, "gemini", "mydomain.com", kGemini_DefaultPort, "", "");
    testUri("URI - Https", httpsUri, "https", "www.mydomain.com", kGemini_DefaultPort, "path/to/folder/file.html", "search");
    testUri("URI - Local", localUri, "gemini", "localhost", kGemini_DefaultPort, "", "");
    testUri("URI - No Suffix", noSuffixUri, "gemini", "mydomain.com", kGemini_DefaultPort, "", "");
    testUriFailParse("URI - Invalid Scheme", invalidScheme);
 
    endsWithTest("strEndsWith - True", simpleUri, "/", 1);
    endsWithTest("strEndsWith - False", noSuffixUri, "/", 0);
    
    finishTesting();
    return 0;
}