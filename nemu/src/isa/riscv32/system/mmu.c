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
#include <memory/paddr.h>

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

#ifndef isa_mmu_check
int isa_mmu_check(vaddr_t vaddr, int len, int type) {
  Assert((vaddr + len) <= ((vaddr & ~PAGE_MASK) + PAGE_SIZE), "isa_mmu_check(vaddr: 0x%08x, len: %d) more than one page", vaddr, len);
  PTE32 *ptr = (PTE32 *)(((uintptr_t)cpu.satp & 0x3fffff) << 12);
  const uintptr_t vpn[2] = {
    (vaddr >> 12) & 0x3ff,
    (vaddr >> 22),
  };
  ptr = (PTE32 *)guest_to_host((uintptr_t)&ptr[vpn[1]]);
  if (!ptr->V) {
    panic("first pte fail");
    return MMU_FAIL;
  }
  ptr = (PTE32 *)((uintptr_t)ptr->PPN << 12);
  ptr = (PTE32 *)guest_to_host((uintptr_t)&ptr[vpn[0]]);
  if (!ptr->V) {
    panic("second pte fail");
    return MMU_FAIL;
  }
  if (ptr->PPN == (vaddr >> 12)) { return MMU_DIRECT; }
  panic("kernel should be direct mapted");
  return MMU_TRANSLATE;
}
#endif

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  Assert((vaddr + len) <= ((vaddr & ~PAGE_MASK) + PAGE_SIZE), "isa_mmu_translate(vaddr: 0x%08x, len: %d) more than one page", vaddr, len);
  PTE32 *ptr = (PTE32 *)(((uintptr_t)cpu.satp & 0x3fffff) << 12);
  const uintptr_t page_offset = vaddr & PAGE_MASK;
  const uintptr_t vpn[2] = {
    (vaddr >> 12) & 0x3ff,
    (vaddr >> 22),
  };
  ptr = (PTE32 *)guest_to_host((uintptr_t)&ptr[vpn[1]]);
  if (!ptr->V) {
    panic("first pte fail");
    return MEM_RET_FAIL;
  }
  ptr = (PTE32 *)((uintptr_t)ptr->PPN << 12);
  ptr = (PTE32 *)guest_to_host((uintptr_t)&ptr[vpn[0]]);
  if (!ptr->V) {
    panic("second pte fail");
    return MEM_RET_FAIL;
  }
  Assert(ptr->PPN == (vaddr >> 12), "kernel should be direct maped, but PPN: 0x%08x, vaddr: 0x%08x", ptr->PPN << 12, vaddr);
  return (ptr->PPN << 12) | page_offset;
}
