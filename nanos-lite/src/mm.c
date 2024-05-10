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

uintptr_t get_pa(void *pdir, uintptr_t va) {
#ifdef HAS_VME
  typedef union {
    struct {
      uint32_t V    : 1;
      uint32_t R    : 1;
      uint32_t W    : 1;
      uint32_t X    : 1;
      uint32_t U    : 1;
      uint32_t G    : 1;
      uint32_t A    : 1;
      uint32_t D    : 1;
      uint32_t RSW  : 2;
      uint32_t PPN0 : 10;
      uint32_t PPN1 : 12;
    };
    struct {
      uint32_t      : 10;
      uint32_t PPN  : 22;
    };
    uint32_t pte;
  } PTE32;

  if(pdir == NULL) { return va; }
  PTE32 *ptr = pdir;
  const uintptr_t page_offset = va & 0xfff;
  const uintptr_t vpn[2] = {
    (va >> 12) & 0x3ff,
    (va >> 22),
  };
  ptr = (PTE32 *)(uintptr_t)&ptr[vpn[1]];
  if (!ptr->V) {
    panic("(vaddr: 0x%08x)first pte fail", va);
  }
  ptr = (PTE32 *)((uintptr_t)ptr->PPN << 12);
  ptr = (PTE32 *)(uintptr_t)&ptr[vpn[0]];
  if (!ptr->V) {
    panic("(vaddr: 0x%08x)second pte fail", va);
  }
  return (ptr->PPN << 12) | page_offset;
#else
  return va;
#endif
}


/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
#ifdef HAS_VME
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
#endif
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
