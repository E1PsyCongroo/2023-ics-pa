#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS_LO16(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15)
#ifndef __riscv_e
#define REGS_HI16(f) \
                                    f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)
#define NR_REGS 32
#else
#define REGS_HI16(f)
#define NR_REGS 16
#endif

#define REGS(f) REGS_LO16(f) REGS_HI16(f)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((NR_REGS + 5) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  ((NR_REGS + 0) * XLEN)
#define OFFSET_STATUS ((NR_REGS + 1) * XLEN)
#define OFFSET_EPC    ((NR_REGS + 2) * XLEN)
#define OFFSET_SATP   ((NR_REGS + 3) * XLEN)
#define OFFSET_NP     ((NR_REGS + 4) * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
  csrrw sp, mscratch, sp      // (1) atomically exchange sp and mscratch
  bnez sp, user_save          // (2) take the branch if we trapped from user
  csrr sp, mscratch           // (3) if we trapped from kernel, restore the original sp
  addi sp, sp, -CONTEXT_SIZE  // pp == KERNEL
  STORE zero, OFFSET_NP(sp)   // c->np = KERNEL
  j save_context

user_save:                    // pp == USER
  addi sp, sp, -CONTEXT_SIZE
  PUSH(5)
  addi t0, zero, 1
  STORE t0, OFFSET_NP(sp)     // c->np = USER
  POP(5)

save_context:
  MAP(REGS, PUSH)             // c->gpr = gpr
  csrr t0, mscratch
  STORE t0, OFFSET_SP(sp)     // c->gpr[sp] = $sp
  csrw mscratch, zero         // ksp = 0

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)  // c->mcause = $mcause
  STORE t1, OFFSET_STATUS(sp) // c->mstatus = $mstatus
  STORE t2, OFFSET_EPC(sp)    // c->mepc = $mepc

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle
  mv sp, a0

  LOAD t0, OFFSET_NP(sp)
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  addi t2, t2, 4
  csrw mstatus, t1            // $mstatus = c->mstatus
  csrw mepc, t2               // $mepc = c->mepc + 4
  beqz t0, skip_save_ksp
  addi t0, sp, CONTEXT_SIZE
  csrw mscratch, t0           // ksp = $sp

skip_save_ksp:
  MAP(REGS, POP)              // gpr = c->gpr

  addi sp, sp, CONTEXT_SIZE
  LOAD sp, (-CONTEXT_SIZE+OFFSET_SP)(sp)    // $sp = c->gpr[sp]
  mret
