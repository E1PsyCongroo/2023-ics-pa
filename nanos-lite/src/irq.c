#include <common.h>

Context* schedule(Context *prev);
void do_syscall(Context *c);
static Context* do_event(Event e, Context* c) {
  Context *ret = c;
  switch (e.event) {
    case EVENT_YIELD: DEBUG("yield"); ret = schedule(c); break;
    case EVENT_SYSCALL: DEBUG("syscalls"); do_syscall(c); break;
    // default: break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  if (c != ret) {
    DEBUG("Change Context(%p) -> Context(%p, entry: 0x%08x, pidr: %p, &argc: 0x%08x)", c, ret, ret->mepc + 4, ret->pdir, ret->GPRx);
  }
  return ret;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
