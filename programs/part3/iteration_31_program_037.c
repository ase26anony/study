/* test-loop-doloop.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Test 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple computation */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple computation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: nested loops - inner loop should generate the pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            local_sum += (i * j) & 0xF;
            global_sum += 3;
            j = counter;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Test 4: multiple decrementing loops in same function */
int test_multiple_loops(int n) {
    int local_sum = 0;
    int counter1 = n;
    unsigned int counter2 = n * 2;
    
    /* First loop: int counter with pre-decrement */
    do {
        local_sum += counter1;
        global_sum += 4;
    } while (--counter1 != 0);
    
    /* Second loop: unsigned counter with post-decrement */
    while (counter2-- != 0) {
        local_sum += (counter2 & 7);
        global_sum += 5;
    }
    
    return local_sum;
}

/* Test 5: loop with parameter as bound */
int test_param_loop(int iterations) {
    int local_sum = 0;
    int counter = iterations;
    
    /* Use parameter directly in loop condition */
    do {
        local_sum = (local_sum * 13 + 17) & 0xFF;
        global_sum += 6;
    } while (--counter != 0);
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but ensure it's reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000; /* Bound it */
    }
    
    printf("Testing loop patterns with base_iterations = %d\n", base_iterations);
    
    /* Execute all test functions */
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_multiple_loops(base_iterations / 2);
    int result5 = test_param_loop(base_iterations);
    
    /* Combine results in a non-trivial way to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + result4 + result5 + global_sum;
    
    printf("Final result: %d (global_sum: %d)\n", final_result, global_sum);
    
    /* Return predictable value for verification */
    return (final_result > 0) ? 0 : 1;
}
