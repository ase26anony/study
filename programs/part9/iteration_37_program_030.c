/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    int result;
    /* Simple arithmetic that won't trap */
    result = x * 2;
    return result;
}

/* Function with optimization disabled to prevent instruction merging */
__attribute__((optimize("O0")))
static void test_case_1(void) {
    int a = 10, b = 20, c = 0;
    
    /* Create a simple goto that will become a simplejump_p */
    if (a < b) {
        goto target_label;
    }
    
    /* Some code to prevent optimization merging */
    a = b + c;
    
target_label:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic (won't trap)
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    c = a + 5;
    
    /* Use result to prevent dead code elimination */
    printf("Test 1 result: %d\n", c);
}

/* Function with inline asm to control resource usage */
__attribute__((optimize("O1")))
static void test_case_2(void) {
    volatile int x = 100, y = 200;
    int result;
    
    /* Simple conditional to create goto */
    if (x != 0) {
        goto asm_target;
    }
    
    /* Compiler barrier to prevent merging */
    asm volatile("" ::: "memory");
    
asm_target:
    /* Inline asm candidate for delay slot:
       - Only modifies general purpose register
       - No memory access (won't fault)
       - No condition code clobber to avoid conflict
    */
    asm volatile (
        "addl $1, %0"
        : "+r" (x)
        : /* no inputs */
        : /* no clobbers - important! */
    );
    
    /* Use the result */
    result = x + y;
    printf("Test 2 result: %d\n", result);
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O2")))
static void test_case_3(void) {
    int val = 42;
    int modified;
    
    /* Create goto pattern */
    if (val > 0) {
        goto call_target;
    }
    
    /* Prevent optimization */
    asm volatile("" ::: "memory");
    
call_target:
    /* Function call that might be eligible for delay slot
       if it doesn't conflict with jump resources */
    modified = simple_operation(val);
    
    printf("Test 3 result: %d\n", modified);
}

/* Complex test with multiple jumps and labels */
__attribute__((optimize("O2", "no-gcse", "no-crossjumping")))
static void test_case_4(void) {
    int i, sum = 0;
    int array[4] = {1, 2, 3, 4};
    
    for (i = 0; i < 4; i++) {
        /* Create multiple goto opportunities */
        if (array[i] % 2 == 0) {
            goto process_even;
        } else {
            goto process_odd;
        }
        
    process_even:
        /* Simple arithmetic after label - good delay slot candidate */
        sum += array[i] * 2;
        continue;
        
    process_odd:
        /* Another candidate */
        sum += array[i] * 3;
        continue;
    }
    
    printf("Test 4 result: %d\n", sum);
}

/* Test with memory operations that shouldn't trap */
__attribute__((optimize("O2")))
static void test_case_5(void) {
    int local_var = 10;
    int *safe_ptr = &local_var;  /* Stack address - won't fault */
    
    /* Create goto */
    if (local_var > 5) {
        goto memory_op;
    }
    
    /* Barrier */
    asm volatile("" ::: "memory");
    
memory_op:
    /* Memory operation on stack variable - should be safe */
    *safe_ptr = *safe_ptr + 1;
    
    printf("Test 5 result: %d\n", local_var);
}

/* Main orchestrator */
int main(void) {
    printf("Testing reorg delay slot filling logic...\n");
    
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    return 0;
}
