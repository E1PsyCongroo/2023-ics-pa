#include "trap.h"

#define N 32
uint8_t data[N];
uint8_t test[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
    test[i] = i + 1;
  }
}

int main() {
    reset();
    check(memcmp(data, test, N) == 0);
    for (int i = 0; i < N; i++) {
        for (int j = 1; j < 128; j++) {
            for (int k = 0; k < N; k++) {
                reset();
                if ((int)data[i] + j < 128) {
                    data[i] += j;
                    if (k > i) {
                        check(memcmp(data, test, k) > 0);
                    }
                    else {
                        check(memcmp(data, test, k) == 0);
                    }
                }
                reset();
                if ((int)data[i] - j > 0) {
                    data[i] -= j;
                    if (k > i) {
                        check(memcmp(data, test, k) < 0);
                    }
                    else {
                        check(memcmp(data, test, k) == 0);
                    }
                }
            }
        }
    }
    return 0;
}