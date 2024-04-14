#include "trap.h"

#define N 32
uint8_t data[N];
uint8_t test[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
    test[i] = i + 2;
  }
}

// 检查[l,r)区间中的值是否依次为val, val + 1, val + 2...
void check_seq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    check(data[i] == val + i - l);
  }
}

// 检查[l,r)区间中的值是否均为val
void check_eq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    check(data[i] == val);
  }
}

// 检查[l,r)区间中的值是否均为test[l,r)
void check_test(int l, int r) {
  int i;
  for (i = l; i < r; i ++) {
    check(data[i] == test[i]);
  }
}

// 检查[l,r)区间中的值是否是空结尾的字符串
void check_str(char* str, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (str[i] == '\0') { return; }
  }
  check(0);
}

void test_memset() {
  int l, r;
  for (l = 0; l < N; l++) {
    for (r = l + 1; r <= N; r++) {
      reset();
      uint8_t val = (l + r) / 2;
      memset(data + l, val, r - l);
      check_seq(0, l, 1);
      check_eq(l, r, val);
      check_seq(r, N, r + 1);
    }
  }
}

void test_memcpy() {
  int l, r;
  for (l = 0; l < N; l++) {
    for (r = l + 1; r <= N; r++) {
      reset();
      memcpy(data + l, test + l, r - l);
      check_seq(0, l, 1);
      check_test(l, r);
      check_seq(r, N, r + 1);
    }
  }
}

void test_memmove() {
  int l, r;
  for (l = 0; l < N; l++) {
    for (r = l + 1; r <= N; r++) {
      reset();
      memmove(data + l, data, r - l);
      check_seq(0, l, 1);
      check_seq(l, r, 1);
      check_seq(r, N, r + 1);
    }
  }
}

void test_strcpy() {
  char str[N];
  char test_str[N] = "abcdefg";
  for (int i = 0; i < N; i++) {
    str[i] = i + 1;
  }
  strcpy(str, test_str);
  check_str(str, N);
  for (char *p1 = str, *p2 = test_str; *p1 && *p2; p1++, p2++) {
    check(*p1 == *p2);
  }
}

void test_strcat() {
  char str[N] = "hello";
  char test_str[N] = " world";
  strcat(str, test_str);
  check(!strcmp(str, "hello world"));
}

int main() {
  test_memset();
  test_memcpy();
  test_memmove();
  test_strcpy();
  return 0;
}