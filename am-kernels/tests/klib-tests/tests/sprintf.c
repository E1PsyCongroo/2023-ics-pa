#include "trap.h"
#include <limits.h>

struct {
    int data;
    const char *result;
} test_case[] = {
    {0, "0"},
    {INT_MAX / 17, "126322567"},
    {INT_MAX, "2147483647"},
    {INT_MIN, "-2147483648"},
    {INT_MIN+1, "-2147483647"},
    {UINT_MAX, "-1"},
    {INT_MAX / 17, "126322567"},
    {UINT_MAX, "-1"}
};
#define CASENUM ((sizeof test_case) / (sizeof test_case[0]))
int main() {
    char buf[128];
    for (int i = 0; i < CASENUM; i++) {
        sprintf(buf, "%d", test_case[i].data);
        // putstr(buf);
        // putch('\n');
        check(strcmp(buf, test_case[i].result) == 0);
    }
}