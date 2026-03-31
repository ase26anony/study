/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another noinline function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int x) {
    /* Simple operation that won't trap */
    return x * 2;
}

/* Function with attribute to prevent certain optimizations */
__attribute__((optimize("O0")))
static void test_pattern1(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int result = 0;
    
    /* Use goto to create a simple jump */
    if (a < b) {
        goto target_label1;
    }
    
    /* Some code that won't be executed */
    result = 100;
    
target_label1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap and doesn't conflict with jump */
    asm volatile ("" : "+r"(a) : : "memory");  /* Compiler barrier */
    
    /* Use inline asm that modifies only a general purpose register */
    int temp = a;
    asm volatile ("addl $1, %0" : "+r"(temp) :: "cc");
    a = temp;
    
    /* Use the result to prevent elimination */
    result = a + b;
    printf("Pattern1 result: %d\n", result);
}

/* Another test pattern with function call as candidate */
__attribute__((optimize("O0")))
static void test_pattern2(void) {
    volatile int x = 5;
    volatile int y = 0;
    
    /* Create simple jump structure */
    if (x > 0) {
        goto compute_label;
    }
    
    /* Unreachable code */
    y = -1;
    
compute_label:
    /* Compiler barrier to prevent merging */
    asm volatile ("" ::: "memory");
    
    /* Function call as delay slot candidate - must not be inlinable */
    y = safe_operation(x);
    
    /* Use result */
    printf("Pattern2 result: %d\n", y);
}

/* Test with memory operation that shouldn't fault */
__attribute__((optimize("O0")))
static void test_pattern3(void) {
    /* Use stack variables to ensure safe memory access */
    int array[4] = {1, 2, 3, 4};
    volatile int index = 0;
    volatile int sum = 0;
    
    /* Create jump */
    if (index >= 0) {
        goto process_label;
    }
    
    /* Dead code */
    sum = -100;
    
process_label:
    /* Barrier */
    asm volatile ("" ::: "memory");
    
    /* Safe memory access - array[index] is always valid */
    int val = array[index];
    
    /* Simple arithmetic */
    asm volatile ("addl %1, %0" : "+r"(sum) : "r"(val) : "cc");
    
    printf("Pattern3 result: %d\n", sum);
}

/* Test with multiple jumps in same function */
__attribute__((optimize("O0")))
static void test_pattern4(void) {
    volatile int counter = 0;
    volatile int total = 0;
    
    /* First jump */
    if (counter == 0) {
        goto first_target;
    }
    
first_target:
    /* First candidate */
    asm volatile ("" ::: "memory");
    counter = simple_operation(counter);
    
    /* Second jump */
    if (counter < 10) {
        goto second_target;
    }
    
second_target:
    /* Second candidate */
    asm volatile ("" ::: "memory");
    total += counter;
    
    printf("Pattern4: counter=%d, total=%d\n", counter, total);
}

/* Main orchestrator */
int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    
    return 0;
}
