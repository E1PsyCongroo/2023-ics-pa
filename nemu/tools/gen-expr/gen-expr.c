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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char expr[65536] = {};
static int left_token = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t choose(uint32_t n) {
  if (n == 0) {
      assert(0);
  }
  return ((uint32_t)rand()) % n;
}

static void gen(char op) {
  size_t len = strlen(buf);
  buf[len] = op;
  buf[len + 1] = '\0';
  size_t expr_len = strlen(expr);
  expr[expr_len] = op;
  expr[expr_len + 1] = '\0';
}

static void gen_space(void) {
  switch (choose(2)) {
    case 1: gen(' '); break;
    default: break;
  }
}

static void gen_num(void) {
  static char num_buf[20];
  uint32_t num = (uint32_t)rand();
  switch (choose(3)) {
    case 0:
      sprintf(num_buf, "%u", num);
      strcat(buf, num_buf);
      sprintf(num_buf, "%uu", num);
      strcat(expr,num_buf);
      break;
    case 1:
      sprintf(num_buf, "0x%x", num);
      strcat(buf, num_buf);
      sprintf(num_buf, "0x%xu", num);
      strcat(expr,num_buf);
      break;
    default:
      sprintf(num_buf, "0X%X", num);
      strcat(buf, num_buf);
      sprintf(num_buf, "0X%Xu", num);
      strcat(expr,num_buf);
      break;
  }
}

static void gen_rand_op(void) {
  switch (choose(4)) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    default: gen('/'); break;
  }
}

static void gen_rand_expr_helper(void) {
  if (left_token < 3) {
    left_token--;
    gen_space();
    gen_num();
    gen_space();
    return;
  }
  switch (choose(3)) {
    case 0:
      left_token--;
      gen_space();
      gen_num();
      gen_space();
      break;
    case 1:
      left_token -= 2;
      gen_space();
      gen('(');
      gen_space();
      gen_rand_expr_helper();
      gen_space();
      gen(')');
      gen_space();
      break;
    default:
      gen_space();
      gen_rand_expr_helper();
      if (left_token > 1) {
        left_token--;
        gen_space();
        gen_rand_op();
        gen_space();
        gen_rand_expr_helper();
        gen_space();
      }
      break;
  }
}

static void gen_rand_expr() {
  memset(buf, 0, sizeof buf);
  memset(expr, 0, sizeof expr);
  left_token = 32;
  gen_rand_expr_helper();
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, expr);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -Wall -Werror -o /tmp/.expr");
    if (ret != 0) {
      i--;
      continue;
    }

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
