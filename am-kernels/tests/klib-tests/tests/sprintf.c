#include "trap.h"
#include <limits.h>

struct {
    const char *format;
    int data;
    const char *result;
} int_test_case[] = {
    {"%d", 0, "0"},
    {"%.0d", 0, ""},
    {"%d", INT_MAX / 17, "126322567"},
    {"%10d", INT_MAX / 17, " 126322567"},
    {"%-10d", INT_MAX / 17, "126322567 "},
    {"%010d", INT_MAX / 17, "0126322567"},
    {"%+d", INT_MAX / 17, "+126322567"},
    {"% d", INT_MAX / 17, " 126322567"},
    {"%d", INT_MAX, "2147483647"},
    {"%d", INT_MIN, "-2147483648"},
    {"%d", INT_MIN+1, "-2147483647"},
    {"%d", UINT_MAX, "-1"},
    {"%d", INT_MAX / 17, "126322567"},
    {"%d", UINT_MAX, "-1"}
};
#define INTCASENUM ((sizeof int_test_case) / (sizeof int_test_case[0]))
int main() {
    char buf[128];
    for (int i = 0; i < INTCASENUM; i++) {
        sprintf(buf, int_test_case[i].format, int_test_case[i].data);
        assert(strcmp(buf, int_test_case[i].result) == 0);
    }
}
