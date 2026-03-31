/* test_reorg.c - Program to trigger specific delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* Prevent function inlining to preserve control flow */
__attribute__((noinline, optimize("O0")))
static int simple_arithmetic(int a, int b) {
    return a + b;
}

/* Another non-inlineable function for delay slot candidate */
__attribute__((noinline, optimize("O0")))
static void safe_operation(int *ptr) {
    *ptr += 1;
}

/* Function with a simple jump to label pattern */
__attribute__((optimize("O2")))
static int test_jump_pattern_1(int x) {
    int result = 0;
    int temp1 = x;
    int temp2 = x * 2;
    
    /* Use volatile to prevent optimization */
    volatile int barrier = 0;
    
    /* Create a simple goto pattern that should generate a simplejump */
    if (x > 100) {
        goto target_label_1;
    }
    
    /* Some intermediate code to separate the jump from target */
    result = x * 3;
    barrier = 1;
    
    return result;
    
target_label_1:
    /* Compiler barrier to prevent merging with jump */
    asm volatile("" ::: "memory");
    
    /* Candidate for delay slot: simple arithmetic that doesn't trap */
    /* This should be the 'next_trial' instruction */
    temp1 = temp2 + 7;  /* Simple arithmetic, no memory access */
    
    /* Use the result to prevent dead code elimination */
    result = temp1 + barrier;
    return result;
}

/* Another test with different pattern */
__attribute__((optimize("O2")))
static int test_jump_pattern_2(int x) {
    int a = x;
    int b = x + 1;
    int c = 0;
    
    volatile int flag = 0;
    
    /* Force a simple unconditional jump later */
    if (x < 0) {
        goto compute;
    }
    
    /* Some code here */
    a = b * 2;
    flag = 1;
    
    /* This goto should generate a simplejump_p */
    if (a > 50) {
        goto target_label_2;
    }
    
compute:
    c = a + b;
    return c;
    
target_label_2:
    /* Memory barrier to prevent sequence formation */
    asm volatile("" ::: "memory");
    
    /* Good delay slot candidate: register-only operation */
    /* Use asm to ensure specific instruction generation */
    int r = b;
    asm volatile("addl $5, %0" : "+r"(r) :: "cc");
    
    /* Use result */
    return r + flag;
}

/* Test with function call as delay slot candidate */
__attribute__((optimize("O2")))
static int test_jump_pattern_3(int x) {
    int values[4] = {x, x+1, x+2, x+3};
    int sum = 0;
    
    /* Create control flow that leads to simple jump */
    if (x % 2 == 0) {
        goto process;
    }
    
    sum = x * 10;
    
    if (sum > 1000) {
        goto final_label;
    }
    
process:
    for (int i = 0; i < 4; i++) {
        sum += values[i];
    }
    return sum;
    
final_label:
    /* Barrier to prevent sequence */
    asm volatile("" ::: "memory");
    
    /* Function call that doesn't conflict with jump resources */
    /* This could be eligible if it doesn't set needed resources */
    int local = x;
    safe_operation(&local);
    
    return local + sum;
}

/* Test with memory operation that shouldn't trap */
__attribute__((optimize("O2")))
static int test_jump_pattern_4(int x) {
    /* Stack variable - guaranteed safe address */
    int stack_var = x;
    int *safe_ptr = &stack_var;
    
    volatile int control = 0;
    
    /* Multiple basic blocks to create jump opportunity */
    switch (x % 3) {
        case 0:
            control = 1;
            goto jump_target;
        case 1:
            control = 2;
            break;
        default:
            control = 3;
    }
    
    stack_var = x * 2;
    
    if (stack_var > 100) {
        goto jump_target;
    }
    
    return stack_var + control;
    
jump_target:
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Safe memory operation on stack variable */
    /* Should not trap and has known resource usage */
    int val = *safe_ptr;
    val += 42;
    *safe_ptr = val;
    
    return val + control;
}

/* Main function to run all tests */
int main(int argc, char **argv) {
    int test_value = 150;  /* Value that triggers various paths */
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all test patterns */
    int r1 = test_jump_pattern_1(test_value);
    printf("Pattern 1 result: %d\n", r1);
    
    int r2 = test_jump_pattern_2(test_value);
    printf("Pattern 2 result: %d\n", r2);
    
    int r3 = test_jump_pattern_3(test_value);
    printf("Pattern 3 result: %d\n", r3);
    
    int r4 = test_jump_pattern_4(test_value);
    printf("Pattern 4 result: %d\n", r4);
    
    /* Additional test with different values to explore more paths */
    for (int i = -10; i < 200; i += 30) {
        test_jump_pattern_1(i);
        test_jump_pattern_2(i);
        test_jump_pattern_3(i);
        test_jump_pattern_4(i);
    }
    
    return 0;
}
