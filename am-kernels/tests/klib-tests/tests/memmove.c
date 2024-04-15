#include "trap.h"

#define N 32
uint8_t data[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
  }
}

// 检查[l,r)区间中的值是否依次为val, val + 1, val + 2...
void check_seq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    check(data[i] == val + i - l);
  }
}

int main() {
  int l, r;
  for (l = 0; l < N; l++) {
    for (r = l + 1; r <= N; r++) {
      reset();
      memmove(data + l, data, r - l);
      check_seq(0, l, 1);
      check_seq(l, r, 1);
      check_seq(r, N, r + 1);

      reset();
      memmove(data, data + l, r - l);
      check_seq(0, r - l, l + 1);
      check_seq(r - l, N, r - l + 1);
    }
  }
}
