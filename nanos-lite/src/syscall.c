#include <common.h>
#include "syscall.h"
static int SYS_yield() {
  yield();
  return 0;
}
static void SYS_exit(int status) {
  halt(status);
}
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case 0: SYS_exit(a[1]); break;
    case 1: c->GPRx = SYS_yield(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
