#include <common.h>

Context* schedule(Context *prev);
void do_syscall(Context *c);
static Context* do_event(Event e, Context* c) {
  Context *ret = c;
  switch (e.event) {
    case EVENT_IRQ_TIMER: DEBUG("timer"); Log("receive timer-interputer"); ret = schedule(c); break;
    case EVENT_YIELD: DEBUG("yield"); ret = schedule(c); break;
    case EVENT_SYSCALL: DEBUG("syscalls"); do_syscall(c); break;
    // default: break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return ret;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
