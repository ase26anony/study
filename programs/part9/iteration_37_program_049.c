/* test_reorg_delay_slot.c
 * 
 * This program is designed to trigger GCC's delay slot filling optimization
 * logic in reorg.cc, specifically targeting lines 2135-2149.
 * 
 * Compile with: gcc -O2 -march=mips32 -fdump-rtl-reorg -S test_reorg_delay_slot.c
 * Or for x86:   gcc -O3 -m32 -fno-gcse -fno-crossjumping -fdump-rtl-reorg -S test_reorg_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Function with minimal optimization to prevent instruction merging */
__attribute__((optimize("O0"), noinline))
static void test_simple_jump_with_eligible_insn(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 0;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label;
    }
    
    /* Some code to prevent fall-through optimization */
    b = 30;
    
target_label:
    /* 
     * This instruction should be eligible for delay slot filling:
     * - Non-jump instruction
     * - Simple arithmetic (shouldn't trap)
     * - No resource conflicts with the jump
     * - Not part of a SEQUENCE
     */
    asm volatile("" ::: "memory");  /* Compiler barrier to prevent merging */
    
    /* Simple arithmetic that shouldn't trap */
    c = b + a;
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", c);
}

/* Another test with function call as candidate */
__attribute__((optimize("O0"), noinline))
static void test_jump_with_function_call(void) {
    volatile int x = 42;
    volatile int result = 0;
    
    /* Create multiple basic blocks to encourage jump generation */
    if (x > 0) {
        goto call_target;
    }
    
    x = 100;
    
call_target:
    asm volatile("" ::: "memory");  /* Prevent instruction merging */
    
    /* Function call as delay slot candidate */
    result = simple_operation(x);
    
    printf("Function result: %d\n", result);
}

/* Test with asm statement as candidate instruction */
__attribute__((optimize("O0"), noinline))
static void test_jump_with_asm_insn(void) {
    register int reg_var asm("r8") = 5;
    volatile int trigger = 1;
    
    if (trigger) {
        goto asm_target;
    }
    
    reg_var = 10;
    
asm_target:
    asm volatile("" ::: "memory");  /* Barrier to prevent SEQUENCE formation */
    
    /* 
     * ASM instruction that should be eligible:
     * - Modifies only a general purpose register (r8)
     * - Doesn't touch condition codes or memory
     * - Simple operation unlikely to trap
     */
    asm volatile("addl $7, %0" : "+r"(reg_var) ::);
    
    printf("ASM result: %d\n", reg_var);
}

/* Test with memory operation (stack variable - should be safe) */
__attribute__((optimize("O0"), noinline))
static void test_jump_with_memory_op(void) {
    int stack_var1 = 50;
    int stack_var2 = 25;
    volatile int cond = 1;
    
    if (cond) {
        goto mem_target;
    }
    
    stack_var1 = 100;
    
mem_target:
    asm volatile("" ::: "memory");  /* Barrier */
    
    /* Memory operation on stack variables (shouldn't fault) */
    stack_var1 = stack_var1 + stack_var2;
    
    printf("Memory op result: %d\n", stack_var1);
}

/* Complex test with nested control flow */
__attribute__((optimize("O1"), noinline))  /* O1 for some optimization but not too much */
static void test_complex_pattern(void) {
    volatile int counter = 0;
    volatile int accumulator = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        /* Create pattern of jumps within loop */
        if (i % 3 == 0) {
            goto loop_target;
        }
        
        accumulator += i * 2;
        continue;
        
    loop_target:
        asm volatile("" ::: "memory");
        
        /* Eligible instruction: simple arithmetic, no side effects */
        counter++;
        
        accumulator += i;
    }
    
    printf("Complex pattern: counter=%d, accumulator=%d\n", counter, accumulator);
}

/* Test that avoids trapping instructions */
__attribute__((optimize("O0"), noinline))
static void test_no_trap_instructions(void) {
    volatile int x = 100;
    volatile int y = 3;  /* Non-zero to avoid division by zero */
    int result;
    
    if (x > 50) {
        goto safe_target;
    }
    
    y = 0;  /* This path not taken */
    
safe_target:
    asm volatile("" ::: "memory");
    
    /* Safe operations that don't trap:
     * - Bitwise operations (no division)
     * - Addition/subtraction
     * - Logical operations
     */
    result = (x & 0xFF) | (y << 8);
    
    /* Avoid division which could trap */
    result = result + (x / y);  /* y is non-zero here */
    
    printf("No-trap result: %d\n", result);
}

/* Main function to run all tests */
int main(void) {
    printf("Testing delay slot filling patterns...\n\n");
    
    test_simple_jump_with_eligible_insn();
    test_jump_with_function_call();
    test_jump_with_asm_insn();
    test_jump_with_memory_op();
    test_complex_pattern();
    test_no_trap_instructions();
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
