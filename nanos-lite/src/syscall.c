#include <common.h>
// #include <sys/types.h>
#include "syscall.h"
#define STRACE_EN 1
#if STRACE_EN
#define syscall_log(format, ...) printf("\33[1;37m" format "\33[0m", ## __VA_ARGS__)
static void strace(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit: syscall_log("%s(%d)", "exit", a[1]); break;
    case SYS_yield: syscall_log("%s()", "yield"); break;
    case SYS_write: syscall_log("%s(%d, %p, %zu)", "write", a[1], (void*)a[2], a[3]); break;
    case SYS_brk: syscall_log("%s(%p)", "brk", (void*)a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
#endif
static int _yield() {
  yield();
  return 0;
}
static void _exit(int status) {
  halt(status);
}
static int _write(int fd, void* buf, size_t count) {
  if (fd == 1 || fd == 2) {
    for (size_t i = 0; i < count; i++) { putch(((char*)buf)[i]); }
    return count;
  }
  return -1;
}
static int _brk(void *addr) {
  return 0;
}
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  #if STRACE_EN
  strace(c);
  #endif
  switch (a[0]) {
    case SYS_exit:
    #if STRACE_EN
      syscall_log(" = ?\n");
    #endif
      _exit(a[1]); break;
    case SYS_yield: c->GPRx = _yield(); break;
    case SYS_write: c->GPRx = _write(a[1], (void*)a[2], a[3]); break;
    case SYS_brk: c->GPRx = _brk((void*)a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #if STRACE_EN
  syscall_log(" = %zd\n", (intptr_t)c->GPRx);
  #endif
}
