#include "trap.h"

const char test[] = "Hello\tWorld\n";
#define N (sizeof test)
char data[N];

void reset() {
  for (int i = 0; i < N; i++) {
    data[i] = test[i];
  }
}

int main() {
    reset();
    check(strncmp(data, test, N) == 0);
    for (int i = 0; i < N - 1; i++) {
        for (int j = 1; j < 128; j++) {
            for (int k = 0; k < N; k++) {
                reset();
                if (data[i] + j < 128) {
                    data[i] += j;
                    if (k > i) {
                        check(strncmp(data, test, k) > 0);
                    }
                    else {
                        check(strncmp(data, test, k) == 0);
                    }
                }
                reset();
                if (data[i] - j > 0) {
                    data[i] -= j;
                    if (k > i) {
                        check(strncmp(data, test, k) < 0);
                    }
                    else {
                        check(strncmp(data, test, k) == 0);
                    }
                }
            }
        }
    }
    return 0;
}