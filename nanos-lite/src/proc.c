#include <proc.h>

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void naive_uload(PCB *pcb, const char *filename);

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void context_kload(PCB *p, void (*entry)(void *), void *arg) {
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
  // context_uload(&pcb[0], "/bin/hello", (char *const[2]){ "/bin/hello", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[1], "/bin/pal", (char *const[3]){"/bin/pal", "--skip", NULL }, (char *const[1]){ NULL });
  // context_uload(&pcb[0], "/bin/exec-test", (char *const[2]){ "/bin/exec-test", NULL }, (char *const[1]) { NULL });
  context_uload(&pcb[0], "/bin/menu", (char *const[2]){ "/bin/menu", NULL }, (char *const[1]) { NULL });
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[0] : &pcb[0]);
  return current->cp;
}
