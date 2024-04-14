#include "trap.h"

#define N 32
char data[N];
const char ch = '1';

void reset() {
  for (int i = 0; i < N - 1; i++) {
    data[i] = ch;
  }
  data[N-1] = '\0';
}

void check_const(int length) {
    for (int i = 0; i < length; i++) {
        check(data[i] == ch);
    }
}

int main() {
  for (int i = 0; i < N; i++) {
    reset();
    data[i] = '\0';
    check(strlen(data) == i);
    check_const(i);
  }
  return 0;
}