#include <proc.h>

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void naive_uload(PCB *pcb, const char *filename);

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  DEBUG("switch current(pcb[%d]) -> pcb_boot", current - &pcb[0]);
  current = &pcb_boot;
}

void context_kload(PCB *p, void (*entry)(void *), void *arg) {
  DEBUG("Load %p @ pcb(%p)", entry, pcb);
  p->cp = kcontext((Area) { p->stack, p + 1 }, entry, arg);
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");
  // load program here
  // context_kload(&pcb[0], hello_fun, "hello world");
  // context_kload(&pcb[1], hello_fun, "hello world");
  // context_uload(&pcb[1], "/bin/hello", (char *const[2]){ "/bin/hello", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[0], "/bin/typing-game", (char *const[2]){"/bin/typing-game", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[0], "/bin/bird", (char *const[2]){"/bin/bird", NULL }, (char *const[1]){ NULL });
  context_uload(&pcb[0], "/bin/fceux", (char *const[3]){"/bin/fceux", "/share/games/nes/mario.nes", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[0], "/bin/pal", (char *const[3]){"/bin/pal", "--skip", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[0], "/bin/exec-test", (char *const[2]){ "/bin/exec-test", NULL }, (char *const[1]) { NULL });
  // context_uload(&pcb[0], "/bin/nterm", (char *const[2]){ "/bin/nterm", NULL }, (char *const[1]) { NULL });
  context_uload(&pcb[1], "/bin/hello", (char *const[2]){ "/bin/hello", NULL }, (char *const[1]) { NULL });
}

Context* schedule(Context *prev) {
  current->cp = prev;
  if (current == &pcb_boot) {
    DEBUG("pcb_boot.context store @ pcb_boot[%p].stack", current->cp);
  }
  else {
    DEBUG("pcb[%d].context store @ user stack[%p]", current - &pcb[0], current->cp);
  }
  PCB* p = current;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  uintptr_t get_pa(void *pdir, uintptr_t va);
#ifdef KDEBUG
  Context* prev_context_pa = (Context *)get_pa(p->cp->pdir, (uintptr_t)p->cp);
  DEBUG("at pdir(%p): prev_context_va(0x%08x) -> prev_context_pa(0x%08x)", p->cp->pdir, (uintptr_t)p->cp, prev_context_pa);
#endif
  Context* current_context_pa = (Context *)get_pa(current->as.ptr, (uintptr_t)current->cp);
  DEBUG("at pdir(%p): current_context_va(0x%08x) -> current_context_pa(0x%08x)", current->as.ptr, (uintptr_t)current->cp, current_context_pa);
  if (p == &pcb_boot) {
    DEBUG("switch current(pcb_boot.context(entry: 0x%08x, pdir: %p, GPRx: 0x%08x)) -> pcb[%d].context(entry: 0x%08x, pdir: %p, GPRx: 0x%08x)", prev_context_pa->mepc + 4, prev_context_pa->pdir, prev_context_pa->GPRx, current - &pcb[0],  current_context_pa->mepc + 4, current_context_pa->pdir, current_context_pa->GPRx);
  }
  else {
    DEBUG("switch current(pcb[%d].context(entry: 0x%08x, pdir: %p, GPRx: 0x%08x)) -> pcb[%d].context(entry: 0x%08x, pdir: %p, GPRx: 0x%08x)", p - &pcb[0], prev_context_pa->mepc + 4, prev_context_pa->pdir, prev_context_pa->GPRx, current - &pcb[0],  current_context_pa->mepc + 4, current_context_pa->pdir, current_context_pa->GPRx);
  }
  return current_context_pa;
}
