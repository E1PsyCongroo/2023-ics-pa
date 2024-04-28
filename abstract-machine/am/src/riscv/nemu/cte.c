#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
#if debug_printf
  for (size_t i = 0; i < NR_REGS; i++) {
    printf("x%d\t0x%.8x\t", i, c->gpr[i]);
    if (i % 2) { putch('\n'); }
  }
  printf("mcause\t0x%.8x\n", c->mcause);
  printf("mstatus\t0x%.8x\n", c->mstatus);
  printf("mepc\t0x%.8x\n", c->mepc);
#endif
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case 0: case 1: case 2: case 3:
      case 4: case 5: case 6: case 7:
      case 8: case 9: case 10: case 11:
      case 12: case 13: case 14: case 15:
      case 16: case 17: case 18: case 19:
        ev.event = EVENT_SYSCALL; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context*)kstack.end - 1;
  c->mepc = (uintptr_t)entry;
  c->mcause = 0;
  c->gpr[10] = (uintptr_t)arg;
#if __riscv_xlen == 32
  c->mstatus = 0x1800;
#else
  c->mstatus = 0xa00001800;
#endif
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
