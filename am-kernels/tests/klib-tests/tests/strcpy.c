#include "trap.h"

#define N 13
const char *test_str = "Hello World\n";
char test[N];
char data[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
    test[i] = test_str[i];
  }
  data[N-1] = '\0';
}

int main() {
    reset();
    check(strcmp(data, test) != 0);
    for (int i = 0; i < N - 1; i++) {
        for (int k = 0; k < N; k++) {
            reset();
            test[k] = '\0';
            strcpy(data, test);
            check(strcmp(data, test) == 0);
        }
    }
    return 0;
}