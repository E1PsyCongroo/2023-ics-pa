#include <proc.h>
#include <elf.h>
#include <fs.h>
#include <memory.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
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
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr elf_ehdr;
  fs_read(fd, &elf_ehdr, sizeof elf_ehdr);
  assert(*(uint32_t*)(elf_ehdr.e_ident) == 0x464c457f);
  assert(elf_ehdr.e_machine == EXPECT_TYPE);
  assert(elf_ehdr.e_phnum);
  Elf_Phdr elf_phdr[elf_ehdr.e_phnum];
  fs_lseek(fd, elf_ehdr.e_phoff, SEEK_SET);
  fs_read(fd, &elf_phdr, elf_ehdr.e_phentsize * elf_ehdr.e_phnum);
  for (size_t i = 0; i < elf_ehdr.e_phnum; i++) {
    if (elf_phdr[i].p_type == PT_LOAD) {
      void *loader_ptr = (void*)elf_phdr[i].p_vaddr;
      fs_lseek(fd, elf_phdr[i].p_offset, SEEK_SET);
      fs_read(fd, loader_ptr, elf_phdr[i].p_filesz);
      memset(loader_ptr + elf_phdr[i].p_filesz, 0, elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
    }
  }
  fs_close(fd);
  return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  void *u_heap = new_page(8);
  void *u_heap_end = (char *)u_heap + 8 * PGSIZE;
  size_t string_size = 0;
  int argc = 0;
  for (; argv[argc]; argc++) { string_size += strlen(argv[argc]) + 1; }
  int envpc = 0;
  for (; envp[envpc]; envpc++) { string_size += strlen(envp[envpc]) + 1; }
  char *string_area = (char*)u_heap_end - string_size;
  char **u_envp = (char**)string_area - envpc - 1;
  char **u_argv = (char**)u_envp - argc - 1;
  int *u_argc = (int*)u_argv - 1;
  *u_argc = argc;
  for (int i = 0; i < argc; i++) {
    strcpy(string_area, argv[i]);
    u_argv[i] = string_area;
    string_area += strlen(string_area) + 1;
  }
  u_argv[argc] = NULL;
  for (int i = 0; i < envpc; i++) {
    strcpy(string_area, envp[i]);
    u_envp[i] = string_area;
    string_area += strlen(string_area) + 1;
  }
  u_envp[envpc] = NULL;
  uintptr_t entry = loader(pcb, filename);
  pcb->cp = ucontext(NULL, (Area){.start=pcb->stack, .end=pcb+1}, (void*)entry);
  pcb->cp->GPRx = (uintptr_t)u_argc;
  Log("Load %s @ %p, user stack top @ 0x%08x", filename, entry, (void*)pcb->cp->GPRx);
}