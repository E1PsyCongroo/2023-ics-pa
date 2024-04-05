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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *EXPR;
  word_t VAL;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

static WP* new_wp();
static void free_wp(WP *wp);

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].EXPR = NULL;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
static WP* new_wp() {
  if (free_ == NULL) { TODO(); }
  WP *new = free_;
  free_ = free_->next;
  new->next = NULL;
  return new;
}

static void free_wp(WP *wp) {
  wp->next = free_;
  free(wp->EXPR);
  wp->EXPR = NULL;
  free_ = wp;
}

int delete_wp(int number) {
  WP **ptr = &head;
  while (*ptr) {
    if ((*ptr)->NO == number) {
      WP *to_delete = *ptr;
      *ptr = to_delete->next;
      free_wp(to_delete);
      return 0;
    }
    ptr = &(*ptr)->next;
  }
  return -1;
}

bool add_wp(char *e) {
  WP* new = new_wp();
  bool success;
  new->VAL = expr(e, &success);
  if (!success) {
    free_wp(new);
  }
  else {
    new->EXPR = malloc(strlen(e) + 1);
    strcpy(new->EXPR, e);
    new->next = head;
    head = new;
  }
  return success;
}

void wp_display() {
  if (!head) {
    printf("No watchpoints\n");
    return;
  }
  printf("Num\tWhat\n");
  for (WP *t = head; t != NULL; t = t->next) {
    printf("%d\t%s\n", t->NO, t->EXPR);
  }
}

bool scan_wp() {
  bool change = false, success;
  for (WP *t = head; t != NULL; t = t->next) {
    word_t new_val = expr(t->EXPR, &success);
    if (new_val != t->VAL) {
      printf("watchpoint %d: %s\n", t->NO, t->EXPR);
      printf("Old value = " FMT_WORD "\nNew value = " FMT_WORD "\n", t->VAL, new_val);
      t->VAL = new_val;
      change = true;
    }
  }
  return change;
}
