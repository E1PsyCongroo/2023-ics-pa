/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
#include "sdb.h"
#include "common.h"
#include "utils.h"

static int is_batch_mode = false;
static int difftest_en = true;

static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_detach(char *args);
static int cmd_attach(char *args);
static int cmd_test(char *args);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state =  NEMU_QUIT;
  return -1;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step N instructions exactly, default: N=1", cmd_si },
  { "info", "Generic command for showing things about the program being debugged", cmd_info },
  { "x", "Output N consecutive 4-bytes in hexadecimal at address EXPR", cmd_x },
  { "p", "Print value of expression EXPR", cmd_p },
  { "w", "Set a watchpoint for EXPR", cmd_w },
  { "d", "Delete the watchpoint with serial number N", cmd_d },
  { "test", "Built-in test", cmd_test },
  { "detach", "Detach difftest mode", cmd_detach },
  { "attach", "Attach difftest mode", cmd_attach },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, "");
  uint64_t n = 1;
  if (arg && !sscanf(arg, "%" PRIu64, &n)) {
    printf("Unknown args '%s'\n", arg);
    return 0;
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, "");
  if (!arg) {
    printf("Command info args can't be empty\n");
    return 0;
  }
  if (!strcmp(arg, "r")) {
    isa_reg_display();
  }
  else if (!strcmp(arg, "w")) {
    wp_display();
  }
  else {
    printf("Unknow args '%s'\n", arg);
  }
  return 0;
}

static int cmd_x(char *args) {
  char *argN = strtok(NULL, " ");
  char *argExpr = strtok(NULL, "");
  size_t n = 0;
  vaddr_t expr_val;
  uint32_t read;
  bool success;
  if (!(argN && argExpr)) {
    printf("Command x args can't be empty\n");
  }
  sscanf(argN, "%zu", &n);
  if (!n) {
    printf("N should be integer and greater than 0\n");
  }
  expr_val = expr(argExpr, &success);
  if (!success) {
    printf("Invalid expression\n");
    return 0;
  }
  if (!in_pmem(expr_val)) {
    printf("address = " FMT_WORD " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "]\n",
      expr_val, PMEM_LEFT, PMEM_RIGHT);
    return 0;
  }
  printf(FMT_WORD ":", expr_val);
  for (size_t i = 0; i < n; i++) {
    if (i % 4 == 0) { putchar('\n'); }
    read = (uint32_t)vaddr_read(expr_val, 4);
    printf("0x%08x ", read);
    expr_val += 4;
  }
  putchar('\n');
  return 0;
}

static int cmd_p(char *args) {
  bool success;
  word_t result = expr(args, &success);
  if (success) {
    printf("result: " FMT_WORD "\n", result);
  }
  else {
    printf("Invalid expression\n");
  }
  return 0;
}

static int cmd_w(char *args) {
  char *arg = strtok(NULL, "");
  if (!arg) {
    printf("Argument required (expression to compute).\n");
    return 0;
  }
  if (!add_wp(arg)){
    printf("Invalid expression\n");
  }
  return 0;
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, "");
  int n;
  int scanf_ret = sscanf(arg, "%d", &n);
  if (scanf_ret == EOF) {
    printf("Argument required (watchpoint to delete).\n");
  }
  else if (!scanf_ret) {
    printf("Unknown args '%s'\n", arg);
  }
  else if (delete_wp(n) < 0) {
    printf("Unknown watchpoint '%d'\n", n);
  }
  return 0;
}

static int cmd_detach(char *args) {
  difftest_en = false;
  printf("Difftest disable\n");
  return 0;
}

void difftest_sync();
static int cmd_attach(char *args) {
  difftest_en = true;
  printf("Difftest enable\n");
  difftest_sync();
  return 0;
}

bool difftest_enable() {
  return difftest_en;
}

static int cmd_test(char *args) {
  char buf[65536];
  int pass = 0, total = 0;
  FILE *fp = fopen("/tmp/input", "r");
  assert(fp != NULL);
  while (fgets(buf, 65536, fp)){
    buf[strlen(buf)-1] = '\0';
    total++;
    char* argResult = strtok(buf, " ");
    char* argExpr = strtok(NULL, "");
    uint32_t expected, result;
    bool success;
    sscanf(argResult, "%" PRIu32, &expected);
    result = (uint32_t)expr(argExpr, &success);
    if (success && result == expected) { pass++; }
    else {
      printf("FAIL: expr:%s, expected:%u, result:%u\n", argExpr, expected, result);
      fflush(stdout);
      assert(0);
    }
  }
  fclose(fp);
  printf("PASS: %d/%d\n", pass, total);
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb(const char* elf_file) {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
#ifdef CONFIG_FTRACE
  void init_ftrace(const char* elf_file);
  init_ftrace(elf_file);
#endif
}
