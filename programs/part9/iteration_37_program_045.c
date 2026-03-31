/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping -c test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int safe_computation(int x) {
    return x * 2 + 1;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_simple_jump_with_eligible_insn(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 0;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label;
    }
    
    /* Some code that won't be executed but prevents optimization */
    b = 100;
    
target_label:
    /* Candidate instruction for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - Uses local variables
       - No memory references that could fault
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    c = a + b;  /* Simple arithmetic - good candidate */
    asm volatile("" ::: "memory");  /* Prevent merging */
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", c);
}

/* Another test with function call as candidate */
__attribute__((optimize("O0")))
static void test_jump_with_function_call(void) {
    volatile int x = 5;
    volatile int result = 0;
    
    /* Create simple jump */
    if (x != 0) {
        goto compute;
    }
    
    x = 0;  /* Unreachable but prevents optimization */
    
compute:
    asm volatile("" ::: "memory");
    /* Function call as delay slot candidate */
    result = safe_computation(x);
    asm volatile("" ::: "memory");
    
    printf("Function result: %d\n", result);
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static void test_jump_with_asm_insn(void) {
    register int reg1 asm("t0") = 42;
    register int reg2 asm("t1") = 13;
    register int reg3 asm("t2") = 0;
    
    /* Simple conditional to create jump */
    if (reg1 > 0) {
        goto asm_target;
    }
    
    reg2 = 99;  /* Unreachable */
    
asm_target:
    asm volatile("" ::: "memory");
    /* ASM instruction that:
       - Only modifies specific register (t2)
       - Doesn't touch memory or condition codes
       - Simple operation unlikely to trap
    */
    asm volatile("add %0, %1, %2" 
                 : "=r"(reg3) 
                 : "r"(reg1), "r"(reg2));
    asm volatile("" ::: "memory");
    
    printf("ASM result: %d\n", reg3);
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O0")))
static void test_jump_with_safe_memory_op(void) {
    int array[4] = {1, 2, 3, 4};
    volatile int index = 0;
    volatile int sum = 0;
    
    /* Stack-allocated array access - should not fault */
    if (index >= 0 && index < 4) {
        goto safe_access;
    }
    
    index = 0;  /* Fallback */
    
safe_access:
    asm volatile("" ::: "memory");
    /* Safe memory access to stack variable */
    sum = array[index];
    asm volatile("" ::: "memory");
    
    printf("Safe access: %d\n", sum);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_simple_jump_with_eligible_insn();
    test_jump_with_function_call();
    test_jump_with_asm_insn();
    test_jump_with_safe_memory_op();
    
    return 0;
}
