/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent function inlining to preserve control flow */
__attribute__((noinline, optimize("O0")))
static int simple_arithmetic(int a, int b) {
    return a + b;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    /* Simple operation that won't trap */
    return x * 2;
}

/* Function with compiler barrier to prevent instruction merging */
__attribute__((noinline, optimize("O0")))
static void test_pattern1(void) {
    volatile int result = 0;
    int a = 10, b = 20, c = 30;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label1;
    }
    
    /* Some code that won't be executed */
    result = -1;
    return;
    
target_label1:
    /* Compiler barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    c = a + b;
    
    /* Use the result to prevent dead code elimination */
    result = c;
    printf("Pattern1 result: %d\n", result);
}

/* Test with asm statement as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void test_pattern2(void) {
    volatile int x = 5, y = 3;
    volatile int result = 0;
    
    /* Create simple jump */
    if (x > 0) {
        goto target_label2;
    }
    
    result = -1;
    return;
    
target_label2:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Use inline asm for precise instruction control
       This modifies only a general purpose register, no memory or CC */
    asm volatile(
        "addl %1, %0" 
        : "+r"(x) 
        : "r"(y)
        : /* no clobbers - don't clobber CC to avoid resource conflicts */
    );
    
    result = x;
    printf("Pattern2 result: %d\n", result);
}

/* Test with function call as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void test_pattern3(void) {
    volatile int a = 7, b = 8;
    volatile int result = 0;
    
    /* Multiple basic blocks to encourage scheduling */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            goto target_label3;
        }
        a += i;
    }
    
    result = -1;
    return;
    
target_label3:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Function call as delay slot candidate */
    result = simple_arithmetic(a, b);
    
    printf("Pattern3 result: %d\n", result);
}

/* Test with memory operation on stack variable (shouldn't fault) */
__attribute__((noinline, optimize("O0")))
static void test_pattern4(void) {
    int stack_var1 = 100;
    int stack_var2 = 200;
    volatile int result = 0;
    
    /* Create control flow that generates a simple jump */
    switch (stack_var1) {
        case 100:
            goto target_label4;
        default:
            result = -1;
            return;
    }
    
target_label4:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Safe memory operation on stack variable */
    stack_var1 = stack_var2 + 50;
    
    result = stack_var1;
    printf("Pattern4 result: %d\n", result);
}

/* Complex test with multiple jumps and labels */
__attribute__((noinline, optimize("O0")))
static void test_pattern5(void) {
    volatile int counters[4] = {0};
    volatile int result = 0;
    
    /* Create multiple basic blocks with simple jumps */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            goto target_label5a;
        }
        counters[0]++;
    }
    
    result = -1;
    return;
    
target_label5a:
    /* First candidate instruction */
    counters[1] = safe_operation(counters[0]);
    
    /* Another jump to create more opportunities */
    if (counters[1] > 0) {
        goto target_label5b;
    }
    
    result = -2;
    return;
    
target_label5b:
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Second candidate instruction - register operation only */
    int temp = counters[1];
    asm volatile(
        "incl %0"
        : "+r"(temp)
        : 
        : /* no clobbers */
    );
    
    counters[2] = temp;
    result = counters[2];
    printf("Pattern5 result: %d\n", result);
}

/* Main function to run all tests */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    printf("All tests completed.\n");
    return 0;
}
