#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_IA_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_IA_64
#elif defined(__ISA_MIPS32__)
#define EXPECT_TYPE EM_MIPS
#elif defined(__riscv)
#define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_LOONGARCH32R__)
#define EXPECT_TYPE EM_LOONGARCH
#else
# error unsupported ISA __ISA__
#endif
size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr elf_ehdr;
  ramdisk_read(&elf_ehdr, 0, sizeof elf_ehdr);
  assert(*(uint32_t*)(elf_ehdr.e_ident) == 0x464c457f);
  assert(elf_ehdr.e_machine == EXPECT_TYPE);
  assert(elf_ehdr.e_phnum);
  Elf_Phdr elf_phdr[elf_ehdr.e_phnum];
  ramdisk_read(&elf_phdr, elf_ehdr.e_phoff, elf_ehdr.e_phentsize * elf_ehdr.e_phnum);
  size_t mem_size = 0;
  for (size_t i = 0; i < elf_ehdr.e_phnum; i++) {
    mem_size += (elf_phdr[i].p_type == PT_LOAD) ? elf_phdr[i].p_memsz : 0;
  }
  for (size_t i = 0; i < elf_ehdr.e_phnum; i++) {
    if (elf_phdr[i].p_type == PT_LOAD) {
      void *loader_ptr = (void*)elf_phdr[i].p_vaddr;
      ramdisk_read(loader_ptr, elf_phdr[i].p_offset, elf_phdr[i].p_filesz);
      memset(loader_ptr + elf_phdr[i].p_filesz, 0, elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
    }
  }
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

