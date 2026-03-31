/* test-doloop-pattern.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150:
 * SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Test 1: do-while loop with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0x1);  /* Simple non-empty body */
        global_sum += (counter & 0x1);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while loop with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0x3);
        global_sum += (counter & 0x3);
    }
    
    return local_sum;
}

/* Test 3: Nested loops - outer loop fixed, inner uses pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += (i * j) & 0xF;
            global_sum += (i * j) & 0xF;
        } while (--j != 0);  /* Inner loop with pre-decrement */
    }
    
    return local_sum;
}

/* Test 4: Multiple decrement patterns in same function */
int test_multiple_patterns(int n) {
    int local_sum = 0;
    int counter1 = n;
    unsigned int counter2 = n * 2;
    
    /* First pattern */
    while (counter1-- != 0) {
        local_sum += 1;
        global_sum += 1;
    }
    
    /* Second pattern */
    do {
        local_sum += 2;
        global_sum += 2;
    } while (--counter2 != 0);
    
    return local_sum;
}

/* Test 5: Function parameter as counter with simple decrement */
int test_param_decrement(unsigned int counter) {
    int local_sum = 0;
    
    /* Force the pattern by using counter directly */
    unsigned int c = counter;
    while (c != 0) {
        local_sum += (c & 0x7);
        global_sum += (c & 0x7);
        c--;  /* Decrement in body, but compiler might combine with compare */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variable but reasonable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000; /* Reasonable limit */
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Execute all test patterns */
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_multiple_patterns(base_iterations / 2);
    int result5 = test_param_decrement(base_iterations);
    
    /* Calculate and print predictable result */
    int total = result1 + result2 + result3 + result4 + result5;
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Total from returns: %d\n", total);
    printf("Global sum: %d\n", global_sum);
    
    /* Return success if global_sum > 0 (all loops executed) */
    return (global_sum > 0) ? 0 : 1;
}
