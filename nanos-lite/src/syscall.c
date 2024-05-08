#include <common.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fs.h>
#include <proc.h>
#include "syscall.h"

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
int sys_execve(const char *fname, char * const argv[], char *const envp[]);
void switch_boot_pcb();
int mm_brk(uintptr_t brk);

#define STRACE_EN 0
#if STRACE_EN
#define syscall_log(format, ...) printf("\33[1;37m" format "\33[0m", ## __VA_ARGS__)
static void strace(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit:          syscall_log("%s(%d)", "exit", a[1]);                                              break;
    case SYS_yield:         syscall_log("%s()", "yield");                                                     break;
    case SYS_open:          syscall_log("%s(%s, %d, %d)", "open", (char*)a[1], a[2], a[3]);                   break;
    case SYS_read:          syscall_log("%s(%s, %p, %zu)", "read", fd_to_filename(a[1]), (void*)a[2], a[3]);  break;
    case SYS_write:         syscall_log("%s(%s, %p, %zu)", "write", fd_to_filename(a[1]), (void*)a[2], a[3]); break;
    case SYS_close:         syscall_log("%s(%d)", "close", a[1]);                                             break;
    case SYS_lseek:         syscall_log("%s(%s, %zd, %d)", "lseek", fd_to_filename(a[1]), a[2], a[3]);        break;
    case SYS_brk:           syscall_log("%s(%p)", "brk", (void*)a[1]);                                        break;
    case SYS_gettimeofday:  syscall_log("%s(%p, %p)", "gettimeofday", (void*)a[1], (void*)a[2]);              break;
    case SYS_execve:        syscall_log("%s(%s, %p, %p)", "execve", (char*)a[1], (void*)a[2], (void*)a[3]);   break;
    default: panic("Strace: unhandled syscall ID = %d", a[0]);
  }
}
#endif
int sys_yield() {
  yield();
  return 0;
}
void sys_exit(int status) {
  // sys_execve("/bin/nterm", (char *const[2]){ "/bin/nterm", NULL }, (char *const[1]) { NULL });
  halt(status);
}
int sys_brk(void *addr) {
  return mm_brk((uintptr_t)addr);
}
int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us / 1000000;
  tv->tv_usec = us % 1000000;
  return 0;
}
int sys_execve(const char *fname, char * const argv[], char *const envp[]) {
  int fd = fs_open(fname, 0, 0);
  if (fd == -1) { return -2; }
  fs_close(fd);
  context_uload(current, fname, argv, envp);
  switch_boot_pcb();
  yield();
  return -1;
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
      sys_exit(a[1]); break;
    case SYS_yield:         c->GPRx = sys_yield();                                        break;
    case SYS_open:          c->GPRx = fs_open((char*)a[1], a[2], a[3]);                   break;
    case SYS_read:          c->GPRx = fs_read(a[1], (void*)a[2], a[3]);                   break;
    case SYS_write:         c->GPRx = fs_write(a[1], (void*)a[2], a[3]);                  break;
    case SYS_close:         c->GPRx = fs_close(a[1]);                                     break;
    case SYS_lseek:         c->GPRx = fs_lseek(a[1], a[2], a[3]);                         break;
    case SYS_brk:           c->GPRx = sys_brk((void*)a[1]);                               break;
    case SYS_gettimeofday:  c->GPRx = sys_gettimeofday((void*)a[1], (void*)a[2]);         break;
    case SYS_execve:        c->GPRx = sys_execve((void*)a[1], (void*)a[2], (void*)a[3]);  break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #if STRACE_EN
  syscall_log(" = %zd\n", (intptr_t)c->GPRx);
  #endif
}
