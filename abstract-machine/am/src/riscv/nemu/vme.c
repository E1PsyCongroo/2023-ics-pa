#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      #ifdef AMDEBUG
      printf("kernel map va(%p) -> pa(%p)\n", va, va);
      #endif
      map(&kas, va, va, 0);
    }
  }
  set_satp(kas.ptr);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
  #ifdef AMDEBUG
    static void *last_pdir = NULL;
    if (last_pdir != c->pdir) {
      printf("switch satp -> %p\n", c->pdir);
      last_pdir = c->pdir;
    }
  #endif
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  if (as == NULL) { return; }
  const uint32_t vpn[2] = {
    ((uintptr_t)va >> 12) & 0x3ff,
    ((uintptr_t)va >> 22),
  };
  PTE32 *pte = &((PTE32*)(as->ptr))[vpn[1]];
  if (pte->V) {
    pte = (PTE32 *)(pte->PPN << 12);
  }
  else {
    void *new_page = pgalloc_usr(PGSIZE);
    pte->PPN = (uintptr_t)new_page >> 12;
    pte->V = 1;
    pte = new_page;
  }
  pte = &pte[vpn[0]];
  pte->PPN = (uintptr_t)pa >> 12;
  pte->V = 1;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context*)kstack.end - 1;
  c->mepc = (uintptr_t)entry - 4;
  c->mcause = 0;
#if __riscv_xlen == 32
  c->mstatus = 0x1880;
#else
  c->mstatus = 0xa00001880;
#endif
  c->pdir = as->ptr;
  c->np = USER;
  return c;
}
