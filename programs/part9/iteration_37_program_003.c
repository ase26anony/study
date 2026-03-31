/* test_reorg_delay_slot.c
 * 
 * This program is designed to trigger GCC's delay slot filling optimization
 * specifically targeting the uncovered lines 2135-2149 in reorg.cc.
 * 
 * Compilation recommendations:
 * 1. For MIPS delay slots: gcc -O2 -march=mips32 -fdump-rtl-reorg test.c -o test
 * 2. For x86 reorg analysis: gcc -O3 -m32 -fno-gcse -fno-crossjumping -c test.c
 * 3. For debugging: gcc -O1 -m32 -fdump-rtl-all -da -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

__attribute__((noinline))
static void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test 1: Simple goto with arithmetic operation after label */
__attribute__((optimize("O2")))
static int test_simple_goto_arithmetic(void) {
    volatile int a = 5;
    volatile int b = 10;
    volatile int result = 0;
    
    /* Use volatile to prevent optimization */
    if (a > 0) {
        goto target_label;
    }
    
    /* This should never execute */
    result = 999;
    return result;
    
target_label:
    /* Candidate for delay slot filling:
     * - Simple arithmetic operation
     * - No memory access that could fault
     * - Sets only local variable
     */
    memory_barrier(); /* Prevent merging with label */
    result = b + 1;   /* Simple arithmetic - should not trap */
    
    /* Use result to prevent dead code elimination */
    return result;
}

/* Test 2: goto with inline asm that doesn't conflict with jump resources */
__attribute__((optimize("O2")))
static int test_goto_with_asm(void) {
    int x = 42;
    int y = 0;
    
    /* Force a simple jump */
    if (x != 0) {
        goto asm_target;
    }
    
    y = -1;
    return y;
    
asm_target:
    memory_barrier();
    
    /* Inline asm that:
     * - Only modifies a general purpose register (eax)
     * - Doesn't touch condition codes (no "cc" clobber)
     * - Doesn't access memory
     * - Should be eligible for delay slot
     */
    asm volatile (
        "movl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (y)
        : 
        : "%eax"
    );
    
    return y;
}

/* Test 3: Nested goto pattern to create scheduling opportunities */
__attribute__((optimize("O3")))
static int test_nested_goto_pattern(void) {
    volatile int counter = 0;
    volatile int value = 1;
    int i;
    
    for (i = 0; i < 10; i++) {
        /* Create multiple simple jumps */
        if (value > 0) {
            goto compute;
        }
        
        counter++;
        continue;
        
    compute:
        /* Candidate instruction after label */
        memory_barrier();
        counter = simple_operation(counter); /* Function call - won't be inlined */
        
        /* Another jump to create more scheduling opportunities */
        if (counter < 5) {
            goto continue_point;
        }
        
        value = counter * 2;
        continue;
        
    continue_point:
        memory_barrier();
        value = counter + 1;
    }
    
    return counter;
}

/* Test 4: goto with safe memory access (stack variable) */
__attribute__((optimize("O2")))
static int test_goto_with_safe_memaccess(void) {
    int array[4] = {1, 2, 3, 4};
    volatile int index = 2;
    int result = 0;
    
    /* Create simple jump */
    if (array[0] > 0) {
        goto mem_access;
    }
    
    result = -1;
    return result;
    
mem_access:
    memory_barrier();
    
    /* Safe memory access - stack variable, won't fault */
    result = array[index];
    
    /* Simple arithmetic to create more instruction scheduling opportunities */
    result = result * 2 + 1;
    
    return result;
}

/* Test 5: Complex pattern with multiple labels and gotos */
__attribute__((optimize("O1")))  /* O1 to keep structure simple */
static int test_complex_goto_pattern(void) {
    volatile int a = 10, b = 20, c = 30;
    int temp;
    
    /* First simple jump */
    if (a < b) {
        goto label1;
    }
    
    temp = a;
    goto end;
    
label1:
    memory_barrier();
    /* First candidate: simple register operation */
    temp = b + 5;
    
    /* Second jump */
    if (temp < c) {
        goto label2;
    }
    
    temp = c;
    goto end;
    
label2:
    memory_barrier();
    /* Second candidate: another simple operation */
    temp = temp * 2;
    
    /* Third jump - creates chain */
    if (temp > 50) {
        goto label3;
    }
    
    temp = 50;
    goto end;
    
label3:
    memory_barrier();
    /* Third candidate */
    asm volatile (
        "addl $7, %0"
        : "+r" (temp)
        :
        : /* No cc clobber to avoid resource conflict */
    );
    
end:
    return temp;
}

/* Test 6: Function call as delay slot candidate */
__attribute__((optimize("O2")))
static int test_goto_with_func_call(void) {
    volatile int trigger = 1;
    int result = 0;
    
    if (trigger) {
        goto call_site;
    }
    
    result = -1;
    return result;
    
call_site:
    memory_barrier();
    /* Function call that won't be inlined */
    result = simple_operation(42);
    
    return result;
}

/* Main function to run all tests */
int main(void) {
    int results[6];
    
    printf("Testing GCC reorg delay slot optimization...\n");
    
    results[0] = test_simple_goto_arithmetic();
    printf("Test 1 result: %d\n", results[0]);
    
    results[1] = test_goto_with_asm();
    printf("Test 2 result: %d\n", results[1]);
    
    results[2] = test_nested_goto_pattern();
    printf("Test 3 result: %d\n", results[2]);
    
    results[3] = test_goto_with_safe_memaccess();
    printf("Test 4 result: %d\n", results[3]);
    
    results[4] = test_complex_goto_pattern();
    printf("Test 5 result: %d\n", results[4]);
    
    results[5] = test_goto_with_func_call();
    printf("Test 6 result: %d\n", results[5]);
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    
    printf("Total sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
