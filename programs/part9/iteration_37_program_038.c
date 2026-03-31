/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_computation(int a, int b) {
    /* Simple arithmetic that won't trap */
    return a * b + 2;
}

/* Function with O0 to prevent premature sequence formation */
__attribute__((optimize("O0")))
static void test_pattern1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use goto to create simple jump */
    if (a < b) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b + c;
    
target_label1:
    /* Candidate instruction for delay slot:
       - Non-jump instruction
       - Simple arithmetic that doesn't trap
       - No resource conflicts with jump
       - Not part of a SEQUENCE
    */
    asm volatile("" ::: "memory");  /* Compiler barrier */
    
    /* Simple arithmetic operation - good delay slot candidate */
    c = a + b;
    
    /* Use result to prevent dead code elimination */
    result = c;
    printf("Pattern1 result: %d\n", result);
}

/* Another test with function call as candidate */
__attribute__((optimize("O0")))
static void test_pattern2(void) {
    volatile int x = 5, y = 3;
    int temp;
    
    /* Create simple jump structure */
    if (x > 0) {
        goto compute_label;
    }
    
    /* Unreachable code to maintain control flow */
    temp = x * y;
    
compute_label:
    asm volatile("" ::: "memory");  /* Prevent merging */
    
    /* Function call as delay slot candidate */
    temp = simple_operation(x);
    
    /* Use the result */
    printf("Pattern2 result: %d\n", temp);
}

/* Test with asm statement as candidate */
__attribute__((optimize("O0")))
static void test_pattern3(void) {
    int reg1 = 100, reg2 = 200;
    volatile int output;
    
    /* Jump to label */
    if (reg1 != 0) {
        goto asm_target;
    }
    
    /* Code that won't execute */
    output = reg1 - reg2;
    
asm_target:
    asm volatile("" ::: "memory");  /* Barrier */
    
    /* Inline asm as delay slot candidate:
       - Modifies only general purpose register
       - No memory access
       - No condition codes if we avoid arithmetic
    */
    asm volatile("movl %1, %0" 
                 : "=r"(reg1) 
                 : "r"(reg2)
                 : /* no clobbers */);
    
    output = reg1;
    printf("Pattern3 result: %d\n", output);
}

/* Complex pattern with multiple basic blocks */
__attribute__((optimize("O0")))
static void test_pattern4(void) {
    int i, sum = 0;
    volatile int array[4] = {1, 2, 3, 4};
    
    for (i = 0; i < 4; i++) {
        /* Conditional that will often be true */
        if (array[i] > 0) {
            goto process;
        }
        
        /* Alternative path */
        sum -= array[i];
        continue;
        
    process:
        asm volatile("" ::: "memory");
        
        /* Good delay slot candidate: simple load and arithmetic */
        int val = array[i];
        sum += val;
    }
    
    printf("Pattern4 sum: %d\n", sum);
}

/* Test with safe memory access (stack variable) */
__attribute__((optimize("O0")))
static void test_pattern5(void) {
    int local_var = 42;
    int *safe_ptr = &local_var;
    volatile int result;
    
    /* Simple jump */
    if (local_var > 0) {
        goto safe_access;
    }
    
    /* Unreachable */
    result = local_var * 2;
    
safe_access:
    asm volatile("" ::: "memory");
    
    /* Safe memory access - won't fault */
    int loaded = *safe_ptr;
    
    result = loaded + 1;
    printf("Pattern5 result: %d\n", result);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    return 0;
}
