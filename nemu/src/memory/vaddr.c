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
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  if (cpu.satp >> 31) {
    switch (isa_mmu_check(addr, len, MEM_TYPE_IFETCH)) {
    case MMU_DIRECT:
      return paddr_read(addr, len);
    case MMU_TRANSLATE:
      return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_IFETCH), len);
    case MMU_FAIL:
      panic("isa_mmu_check(addr: 0x%08x, len: %d) fail", addr, len);
    default:
      panic("should not come here");
    }
  }
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
#ifdef CONFIG_MTRACE
  if (MTRACE_COND) { log_write(ANSI_FMT("Read pmem:\t@" FMT_PADDR ", len=%2d\n", ANSI_FG_CYAN), addr, len ); }
#endif
  if (cpu.satp >> 31) {
    switch (isa_mmu_check(addr, len, MEM_TYPE_READ)) {
    case MMU_DIRECT:
      return paddr_read(addr, len);
    case MMU_TRANSLATE:
      return paddr_read(isa_mmu_translate(addr, len, MEM_TYPE_READ), len);
    case MMU_FAIL:
      panic("isa_mmu_check(addr: 0x%08x, len: %d) fail", addr, len);
    default:
      panic("should not come here");
    }
  }
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
#ifdef CONFIG_MTRACE
  if (MTRACE_COND) { log_write(ANSI_FMT("Write pmem:\t@" FMT_PADDR ", len=%2d, data=" FMT_WORD "\n", ANSI_FG_CYAN), addr, len, data); }
#endif
  if (cpu.satp >> 31) {
    switch (isa_mmu_check(addr, len, MEM_TYPE_WRITE)) {
    case MMU_DIRECT:
      paddr_write(addr, len, data);
      break;
    case MMU_TRANSLATE:
      paddr_write(isa_mmu_translate(addr, len, MEM_TYPE_WRITE), len, data);
      break;
    case MMU_FAIL:
      panic("isa_mmu_check(addr: 0x%08x, len: %d) fail", addr, len);
    default:
      panic("should not come here");
    }
  }
  else {
    paddr_write(addr, len, data);
  }
}
