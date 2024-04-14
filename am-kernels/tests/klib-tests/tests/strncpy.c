#include "trap.h"

const char test[] = "Hello World\n";
#define N (sizeof test)
char data[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
  }
  data[N-1] = '\0';
}

int main() {
    reset();
    check(strncmp(data, test, N) != 0);
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N; j++) {
            reset();
            strncpy(data, test, N);
            check(strncmp(data, test, N) == 0);
        }
    }
    return 0;
}