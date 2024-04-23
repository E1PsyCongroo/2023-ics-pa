#include <debug.h>

IFDEF(CONFIG_FTRACE, char* trace_fun(word_t dnpc));
void etrace(uint32_t instruction, word_t pc, word_t info) {
  static int num_space = 0;
  if (instruction == 0b00110000001000000000000001110011u) {
    char *trace_name = MUXDEF(CONFIG_FTRACE, trace_fun(info), "???");
    if (trace_name == NULL) { trace_name = "???"; }
    /* exception return type */
    num_space = num_space == 0 ? 0 : num_space - 1;
    log_write(ANSI_FMT(FMT_WORD ": %*sException return: %s@" FMT_WORD "\n", ANSI_FG_WHITE), pc, num_space, "", trace_name, info);
  } else {
    /* exception throw type */
    log_write(ANSI_FMT(FMT_WORD ": %*sException throw: " FMT_WORD "\n", ANSI_FG_WHITE), pc, num_space, "", info);
    num_space += 1;
  }
}
