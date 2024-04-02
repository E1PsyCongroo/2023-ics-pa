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
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM, TK_NEG,
  TK_REG, TK_DEREF,
  TK_NEQ, TK_AND,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},                              // spaces
  {"\\+", '+'},                                   // plus
  {"-", '-'},                                     // minus
  {"\\*", '*'},                                   // times
  {"/", '/'},                                     // divide
  {"\\(", '('},                                   // left bracket
  {"\\)", ')'},                                   // right bracket
  {"==", TK_EQ},                                  // equal
  {"!=", TK_NEQ},                                 // not equal
  {"&&", TK_AND},                                 // anD
  {"0(x[0-9a-f]+|X[0-9A-F]+)|[0-9]+", TK_NUM},    // number
  {"\\$[$a-zA-Z]+[a-zA-Z0-9]*", TK_REG},          // register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

#define MAXTOKEN 32
static Token tokens[MAXTOKEN] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '+': case '-': case '*': case '/':
          case '(': case ')': case TK_EQ: case TK_NEQ:
          case TK_AND:
            nr_token++;
            break;
          case TK_NUM: case TK_REG:
            if (substr_len >= 32) { TODO(); }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token++].str[substr_len] = '\0';
            break;
          default: TODO();
        }
        if (nr_token > MAXTOKEN) {
          printf("too many tokens\n");
          return false;
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') { return false; }
  int waiting_matched_backet = 0;
  for (int i = p + 1; i < q; i++) {
    if (tokens[i].type == '(') {
      waiting_matched_backet++;
    }
    else if (tokens[i].type == ')') {
      if (waiting_matched_backet == 0) { return false; }
      waiting_matched_backet--;
    }
  }
  return !waiting_matched_backet;
}

static struct PriorityMap {
    int type;
    int priority;
} priority_map[] = {
  {TK_AND, 4},
  {TK_EQ, 3}, {TK_NEQ, 3},
  {'+', 2}, {'-', 2},
  {'*', 1}, {'/', 1},
};

static bool priority_change(int token_type, int *current_priority) {
  for (int i = 0; i < ARRLEN(priority_map); i++) {
    if (token_type == priority_map[i].type && *current_priority <= priority_map[i].priority) {
      *current_priority = priority_map[i].priority;
      return true;
    }
  }
  return false;
}

static int find_pivot(int left, int right, bool *success) {
  int waiting_matched_backet = 0, pivot_index = 0;
  int current_priority = 0;
  for (int i = left; i <= right; i++) {
    switch (tokens[i].type) {
    case TK_NEG: case TK_NUM: case TK_REG: case TK_DEREF:
      continue;
    case TK_AND:
    case TK_EQ: case TK_NEQ:
    case '+': case '-':
    case '*': case '/':
      if (!waiting_matched_backet && priority_change(tokens[i].type, &current_priority)) {
        pivot_index = i;
      }
      break;
    case '(':
      waiting_matched_backet++;
      break;
    case ')':
      waiting_matched_backet--;
      break;
    default: TODO();
    }
  }
  *success = !waiting_matched_backet && pivot_index;
  return pivot_index;
}

static word_t eval(int left, int right, bool *success) {
  if (!*success) { return 0; }
  if (left > right) { return 0; }
  while(check_parentheses(left, right)) { left++, right--; }
  if (left == right) {
    word_t number;
    switch (tokens[left].type) {
      case TK_NUM:
      {
        const char* str = tokens[left].str;
        if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
          sscanf(str, "%" MUXDEF(CONFIG_ISA64, PRIx64, PRIx32), &number);
        } else {
          sscanf(str, "%" MUXDEF(CONFIG_ISA64, PRIu64, PRIu32), &number);
        }
        return number;
      }
      case TK_REG: return isa_reg_str2val(tokens[left].str+1, success);
      default:
        *success = false;
        return 0;
    }
  }
  else if (left == right - 1) {
    switch (tokens[left].type) {
      case TK_NEG: return -eval(left+1, right, success);
      case TK_DEREF:
        vaddr_t paddr = eval(left+1, right, success);
        if (*success) {
          return vaddr_read(paddr, MUXDEF(CONFIG_ISA64, 8, 4));
        }
      default:
        *success = false;
        return 0;
    }
  }
  else {
    int op_index = find_pivot(left, right, success);
    word_t val1, val2;
    val1 = eval(left, op_index - 1, success);
    val2 = eval(op_index + 1, right, success);
    if (!*success) { return 0; }
    switch (tokens[op_index].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          *success = false;
          return 0;
        }
        return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default:
        *success = false;
        return 0;
    }
  }
}

static bool is_unary(int i, int search_type) {
  return tokens[i].type == search_type &&
  (i == 0 || (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')' && tokens[i-1].type != TK_REG));
}

static void check_unary(int search_type, int replace_type) {
  for (int i = 0; i < nr_token; i++) {
    if (is_unary(i, search_type)) {
      tokens[i].type = replace_type;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  check_unary('-', TK_NEG);
  check_unary('*', TK_DEREF);
  *success = true;
  return eval(0, nr_token-1, success);
}
