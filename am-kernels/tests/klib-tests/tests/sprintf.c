#include "trap.h"
#include <limits.h>

struct {
    const char *format;
    int data;
    const char *result;
    int ret;
} int_test_case[] = {
    {"%d", 0, "0", 1},
    {"%.0d", 0, "", 0},
    {"%d", INT_MAX / 17, "126322567", 9},
    {"%10d", INT_MAX / 17, " 126322567", 10},
    {"%-10d", INT_MAX / 17, "126322567 ", 10},
    {"%010d", INT_MAX / 17, "0126322567", 10},
    {"%+d", INT_MAX / 17, "+126322567", 10},
    {"% d", INT_MAX / 17, " 126322567", 10},
    {"% 10d", INT_MAX / 17, " 126322567", 10},
    {"%- d", INT_MAX / 17, " 126322567", 10},
    {"%- 12d", INT_MAX / 17, " 126322567  ", 12},
    {"%d", INT_MAX, "2147483647", 10},
    {"%d", INT_MIN, "-2147483648", 11},
    {"%d", INT_MIN+1, "-2147483647", 11},
    {"%d", UINT_MAX, "-1", 2},
    {"%d", INT_MAX / 17, "126322567", 9},
    {"%d", UINT_MAX, "-1", 2},
    {"%10d", UINT_MAX, "        -1", 10},
    {"%-10d", UINT_MAX, "-1        ", 10},
    {"%010d", UINT_MAX, "-000000001", 10}
};
#define INTCASENUM ((sizeof int_test_case) / (sizeof int_test_case[0]))
void int_test() {
    char buf[128];
    for (int i = 0; i < INTCASENUM; i++) {
        int ret = sprintf(buf, int_test_case[i].format, int_test_case[i].data);
        assert(ret == int_test_case[i].ret);
        for (int j = 0; j < ret; j++) {
            assert(buf[j] == int_test_case[i].result[j]);
        }
    }
}

struct {
    const char *format;
    unsigned int data;
    const char *result;
    int ret;
} uint_test_case[] = {
    {"%u", 0, "0", 1},
    {"%.0u", 0, "", 0},
    {"%u", UINT_MAX / 17, "252645135", 9},
    {"%15u", UINT_MAX / 17, "      252645135", 15},
    {"%-15u", UINT_MAX / 17, "252645135      ", 15},
    {"%015u", UINT_MAX / 17, "000000252645135", 15},
    {"%.15u", UINT_MAX / 17, "000000252645135", 15},
    {"%-015u", UINT_MAX / 17, "252645135      ", 15},
    {"%u", UINT_MAX, "4294967295", 10},
    {"%-020.15u", UINT_MAX, "000004294967295     ", 20},

    {"%o", 0, "0", 1},
    {"%.0o", 0, "", 0},
    {"%o", UINT_MAX / 17, "1703607417", 10},
    {"%15o", UINT_MAX / 17, "     1703607417", 15},
    {"%-15o", UINT_MAX / 17, "1703607417     ", 15},
    {"%015o", UINT_MAX / 17, "000001703607417", 15},
    {"%.15o", UINT_MAX / 17, "000001703607417", 15},
    {"%-015o", UINT_MAX / 17, "1703607417     ", 15},
    {"%o", UINT_MAX, "37777777777", 11},
    {"%-020.15o", UINT_MAX, "000037777777777     ", 20},

    {"%#o", 0, "0", 1},
    {"%#.0o", 0, "0", 1},
    {"%#o", UINT_MAX / 17, "01703607417", 11},
    {"%#15o", UINT_MAX / 17, "    01703607417", 15},
    {"%#-15o", UINT_MAX / 17, "01703607417    ", 15},
    {"%#015o", UINT_MAX / 17, "000001703607417", 15},
    {"%#.15o", UINT_MAX / 17, "000001703607417", 15},
    {"%#-015o", UINT_MAX / 17, "01703607417    ", 15},
    {"%#o", UINT_MAX, "037777777777", 12},
    {"%#-020.15o", UINT_MAX, "000037777777777     ", 20},

    {"%x", 0, "0", 1},
    {"%.0x", 0, "", 0},
    {"%x", UINT_MAX / 17, "f0f0f0f", 7},
    {"%15x", UINT_MAX / 17, "        f0f0f0f", 15},
    {"%-15x", UINT_MAX / 17, "f0f0f0f        ", 15},
    {"%015x", UINT_MAX / 17, "00000000f0f0f0f", 15},
    {"%.15x", UINT_MAX / 17, "00000000f0f0f0f", 15},
    {"%-015x", UINT_MAX / 17, "f0f0f0f        ", 15},
    {"%x", UINT_MAX, "ffffffff", 8},
    {"%-020.15x", UINT_MAX, "0000000ffffffff     ", 20},

    {"%#x", 0, "0", 1},
    {"%#.0x", 0, "", 0},
    {"%#x", UINT_MAX / 17, "0xf0f0f0f", 9},
    {"%#15x", UINT_MAX / 17, "      0xf0f0f0f", 15},
    {"%#-15x", UINT_MAX / 17, "0xf0f0f0f      ", 15},
    {"%#015x", UINT_MAX / 17, "0x000000f0f0f0f", 15},
    {"%#.15x", UINT_MAX / 17, "0x00000000f0f0f0f", 17},
    {"%#-015x", UINT_MAX / 17, "0xf0f0f0f      ", 15},
    {"%#x", UINT_MAX, "0xffffffff", 10},
    {"%#-020.15x", UINT_MAX, "0x0000000ffffffff   ", 20},
    {"%#-020.15x", 0x89abcdef, "0x000000089abcdef   ", 20},

    {"%#X", 0, "0", 1},
    {"%#.0X", 0, "", 0},
    {"%#X", UINT_MAX / 17, "0XF0F0F0F", 9},
    {"%#15X", UINT_MAX / 17, "      0XF0F0F0F", 15},
    {"%#-15X", UINT_MAX / 17, "0XF0F0F0F      ", 15},
    {"%#015X", UINT_MAX / 17, "0X000000F0F0F0F", 15},
    {"%#.15X", UINT_MAX / 17, "0X00000000F0F0F0F", 17},
    {"%#-015X", UINT_MAX / 17, "0XF0F0F0F      ", 15},
    {"%#X", UINT_MAX, "0XFFFFFFFF", 10},
    {"%#-020.15X", UINT_MAX, "0X0000000FFFFFFFF   ", 20},
    {"%#-020.15X", 0x89abcdef, "0X000000089ABCDEF   ", 20},
};
#define UINTCASENUM ((sizeof uint_test_case) / (sizeof uint_test_case[0]))
void uint_test() {
    char buf[128];
    for (int i = 0; i < UINTCASENUM; i++) {
        int ret = sprintf(buf, uint_test_case[i].format, uint_test_case[i].data);
        assert(ret == uint_test_case[i].ret);
        for (int j = 0; j < ret; j++) {
            assert(buf[j] == uint_test_case[i].result[j]);
        }
    }
}

struct {
    const char *format;
    char data;
    const char *result;
    int ret;
} ch_test_case[] = {
    {"%c", 'a', "a", 1},
    {"%10c", '%', "         %", 10},
    {"%-10c", '0', "0         ", 10},
    {"%c", '\0', "\0", 1}
};
#define CHCASENUM ((sizeof ch_test_case) / (sizeof ch_test_case[0]))
void ch_test() {
    char buf[128];
    for (int i = 0; i < CHCASENUM; i++) {
        int ret = sprintf(buf, ch_test_case[i].format, ch_test_case[i].data);
        assert(ret == ch_test_case[i].ret);
        for (int j = 0; j < ret; j++) {
            assert(buf[j] == ch_test_case[i].result[j]);
        }
    }
}

struct {
    const char *format;
    const char *data;
    const char *result;
    int ret;
} str_test_case[] = {
    {"%s", "", "", 0},
    {"%1s", "", " ", 1},
    {"%s", "Hello World\n", "Hello World\n", 12},
    {"%15s", "Hello World\n", "   Hello World\n", 15},
    {"%-15s", "Hello World\n", "Hello World\n   ", 15},
    {"%.10s", "Hello World\n", "Hello Worl", 10},
    {"%15.10s", "Hello World\n", "     Hello Worl", 15},
    {"%-15.10s", "Hello World\n", "Hello Worl     ", 15},
};
#define STRCASENUM ((sizeof str_test_case) / (sizeof str_test_case[0]))
void str_test() {
    char buf[128];
    for (int i = 0; i < STRCASENUM; i++) {
        int ret = sprintf(buf, str_test_case[i].format, str_test_case[i].data);
        assert(ret == str_test_case[i].ret);
        for (int j = 0; j < ret; j++) {
            assert(buf[j] == str_test_case[i].result[j]);
        }
    }
}

struct {
    const char *format;
    void *data;
    const char *result;
    int ret;
} pointer_test_case[] = {
    {"%p", (void *)0x12345678, "0x12345678", 10},
    {"%p", (void *)0x78abcdef, "0x78abcdef", 10},
    {"%p", (void *)0x123, "0x123", 5},
    {"%p", (void *)0x0, "0x0", 3},
    {"%%%p", (void *)0x123, "%0x123", 6},
};
#define POINTERCASENUM ((sizeof pointer_test_case) / (sizeof pointer_test_case[0]))
void pointer_test() {
    char buf[128];
    for (int i = 0; i < POINTERCASENUM; i++) {
        int ret = sprintf(buf, pointer_test_case[i].format, pointer_test_case[i].data);
        assert(ret == pointer_test_case[i].ret);
        for (int j = 0; j < ret; j++) {
            assert(buf[j] == pointer_test_case[i].result[j]);
        }
    }
}

int main() {
    int_test();
    uint_test();
    ch_test();
    str_test();
    pointer_test();
    return 0;
}
