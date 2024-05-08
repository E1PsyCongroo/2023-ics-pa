#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *ret = pf;
  pf = (void*)((uintptr_t)pf + PGSIZE * nr_page);
  return ret;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *ret = new_page(n / PGSIZE);
  memset(ret, 0, n);
  return ret;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
extern PCB *current;
  if (current->max_brk == 0) {
    current->max_brk = ROUNDUP(brk, PGSIZE);
    DEBUG("Init max_brk -> %p(%p)", (void*)brk, (void*)current->max_brk);
  }
  assert(current->max_brk % PGSIZE == 0);
  while (brk > current->max_brk) {
    void *page = new_page(1);
    map(&current->as, (void*)current->max_brk, page, MMAP_READ | MMAP_WRITE);
    DEBUG("Heap Map va(%p) -> pa(%p)", (void*)current->max_brk, page);
    current->max_brk += PGSIZE;
  }
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
