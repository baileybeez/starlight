
#ifndef SIMPLE_TESTS_H
#define SIMPLE_TESTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef nil
#define nil 0
#endif//nil

#define kMaxMessage 256

char outcome[kMaxMessage] = {0};

void finishTesting()
{
    puts("All tests passed.");
}

void testPassed(const char *testName)
{
    printf("> Test ");
    if (testName != nil)
        printf("`%s` ", testName);

    puts("passed!");
}

void exitOnFailedTest(const char *testName, const char *failMessage, const char *testOutcome)
{
    if (failMessage != nil)
        printf("%s\n", failMessage);
        
    if (testOutcome != nil) {
        printf("Test ");
        if (testName != nil)
            printf("`%s` ", testName);

        puts(testOutcome);
    }

    exit(-1);
}

void assertFailure(const char *failMessage)
{
    exitOnFailedTest(nil, failMessage, nil);
}

void assertAreEqual(int a, int b, const char *testName, const char *failMessage)
{
    if (a != b) {
        sprintf(outcome, "failed: '%d' is not equal to expected value '%d'\n", a, b);
        exitOnFailedTest(testName, failMessage, outcome);
    }
}

void assertAreEqual(const char* a, const char * b, const char *testName, const char *failMessage)
{
    if (strcmp(a, b) != 0) {
        sprintf(outcome, "failed: '%s' is not equal to expected value '%s'\n", a, b);
        exitOnFailedTest(testName, failMessage, outcome);
    }
}

void assertLessThan(int a, int b, const char *testName, const char *failMessage)
{
    if (a >= b) {
        sprintf(outcome, "failed: '%d' is not less than expected value '%d'\n", a, b);
        exitOnFailedTest(testName, failMessage, outcome);
    }
}

#endif//SIMPLE_TESTS_H
