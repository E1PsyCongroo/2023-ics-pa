#include "trap.h"

const char *test_str = "Hello World\n";

int main() {
    char test1[32] = "1";
    strcat(test1, test_str);
    check(strcmp(test1, "1Hello World\n") == 0);
    char test2[32] = "wow";
    strcat(test2, test_str);
    check(strcmp(test2, "wow""Hello World\n") == 0);
    return 0;
}