#include <elf.h>
#include <stdlib.h>
#include <debug.h>
typedef struct {
  char *fname;
  word_t fsize;
  word_t faddr;
} FtraceTable;

static FtraceTable *ftrace_table = NULL;
static size_t ftrace_table_num = 0;
static char *elf_str = NULL;

#define N MUXDEF(CONFIG_ISA64, 64, 32)

void init_ftrace(const char* elf_file) {
  if (elf_file == NULL) { return; }
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);
  /* read ELF header */
  concat3(Elf,N,_Ehdr) elf_ehdr;
  Assert(fread(&elf_ehdr, sizeof elf_ehdr, 1, fp) != sizeof elf_ehdr, "%s is not a elf file\n", elf_file);
  Assert(*(uint32_t*)elf_ehdr.e_ident == 0x464c457f, "%s is not a elf file\n", elf_file);
  /* read Section header */
  fseek(fp, elf_ehdr.e_shoff, SEEK_SET);
  concat3(Elf,N,_Shdr) elf_shdr[elf_ehdr.e_shnum];
  fread(&elf_shdr, elf_ehdr.e_shentsize, elf_ehdr.e_shnum, fp);
  concat3(Elf,N,_Off) sym_offset = 0, str_offset = 0;
  concat3(Elf,N,_Word) sym_size = 0, str_size = 0;
  for (size_t i = 0; i < elf_ehdr.e_shnum; i++) {
    if (elf_shdr[i].sh_type == SHT_SYMTAB && !sym_offset) {
      sym_offset = elf_shdr[i].sh_offset;
      sym_size = elf_shdr[i].sh_size;
    }
    else if (elf_shdr[i].sh_type == SHT_STRTAB && !str_offset) {
      str_offset = elf_shdr[i].sh_offset;
      str_size = elf_shdr[i].sh_size;
    }
  }
  /* read Symbol table */
  size_t elf_symentnum = sym_size / sizeof(concat3(Elf,N,_Sym));
  concat3(Elf,N,_Sym) elf_sym[elf_symentnum];
  fseek(fp, sym_offset, SEEK_SET);
  fread(elf_sym, sym_size, 1, fp);
  /* read String table */
  elf_str = malloc((sizeof *elf_str) * str_size);
  fseek(fp, str_offset, SEEK_SET);
  fread(elf_str, str_size, 1, fp);
  /* generating ftrace table*/
  for (size_t i = 0; i < elf_symentnum; i++) {
    if (concat3(ELF,N,_ST_TYPE)(elf_sym[i].st_info) == STT_FUNC) {
      ftrace_table_num++;
    }
  }
  if (!ftrace_table_num) { return; }
  ftrace_table = malloc((sizeof *ftrace_table) * ftrace_table_num);
  FtraceTable *tp = ftrace_table;
  for (size_t i = 0; i < elf_symentnum; i++) {
    if (concat3(ELF,N,_ST_TYPE)(elf_sym[i].st_info) == STT_FUNC) {
      tp->faddr = elf_sym[i].st_value;
      tp->fsize = elf_sym[i].st_size;
      tp->fname = &elf_str[elf_sym[i].st_name];
      tp++;
    }
  }
}

char* trace_fun(word_t dnpc) {
  if (ftrace_table == NULL) { return NULL; }
  for (size_t i = 0; i < ftrace_table_num; i++) {
    if (dnpc >= ftrace_table[i].faddr && dnpc < ftrace_table[i].faddr + ftrace_table[i].fsize) {
      return ftrace_table[i].fname;
    }
  }
  return NULL;
}

void ftrace(uint32_t instruction, word_t pc, word_t dnpc) {
  static char *current_fun = NULL;
  static int num_space = 0;
  char *trace_name = trace_fun(dnpc);
  if (trace_name == NULL) { trace_name = "???"; }
  if (trace_name != current_fun) {
    if (BITS(instruction, 6, 0) == 0b1100111 && BITS(instruction, 19, 15) == 1) {
      /* jalr ret type */
      num_space = num_space == 0 ? 0 : num_space - 1;
      log_write(ANSI_FMT(FMT_WORD ": %*sret[%s->%s@" FMT_WORD "]\n", ANSI_FG_YELLOW), pc, num_space, "", current_fun, trace_name, dnpc);
    } else {
      /* jal call type */
      log_write(ANSI_FMT(FMT_WORD ": %*scall[%s@" FMT_WORD "]\n", ANSI_FG_YELLOW), pc, num_space, "", trace_name, dnpc);
      num_space += 1;
    }
    current_fun = trace_name;
  }
}