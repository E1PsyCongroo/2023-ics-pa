#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  uint32_t us_old = NDL_GetTicks();
  while (1) {
    uint32_t us_new = NDL_GetTicks();
    if (us_new - us_old > 500000) {
      us_old =  us_new;
      printf("Timer-test: after 0.5s\n");
    }
  }
  NDL_Quit();
  return 0;
}
