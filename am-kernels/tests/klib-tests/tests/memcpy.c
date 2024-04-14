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

// 检查[l,r)区间中的值是否均为test[l,r)
void check_test(int l, int r) {
  int i;
  for (i = l; i < r; i ++) {
    check(data[i] == test[i]);
  }
}

int main() {
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
  return 0;
}
