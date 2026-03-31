/* test-doloop-pattern.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150:
 * SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Test 1: do-while with pre-decrement */
int test1_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple computation */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Test 2: while with post-decrement */
int test2_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple computation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: nested loops - inner loop with decrement pattern */
int test3_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += i * j;
            global_sum += 3;
        } while (--j != 0);
    }
    
    return local_sum;
}

/* Test 4: mixed signed/unsigned counters */
int test4_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter = (unsigned int)n;
    
    /* Force the pattern with explicit comparison */
    while (counter != 0) {
        local_sum += counter;
        global_sum += 4;
        counter = counter - 1;  /* Explicit decrement */
    }
    
    return local_sum;
}

/* Test 5: simple counted loop that should generate the pattern */
int test5_simple_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Direct decrement in condition */
    while (n-- != 0) {
        local_sum += n;
        global_sum += 5;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000; /* Reasonable limit */
    }
    
    printf("Testing doloop pattern with base iterations: %d\n", base_iterations);
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Run all test patterns */
    int result1 = test1_do_while_predec(base_iterations);
    int result2 = test2_while_postdec(base_iterations);
    int result3 = test3_nested_loops(5, base_iterations / 5);
    int result4 = test4_mixed_types(base_iterations);
    int result5 = test5_simple_countdown(base_iterations);
    
    /* Compute final result to ensure all loops executed */
    int final_result = result1 + result2 + result3 + result4 + result5 + global_sum;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Global sum: %d\n", global_sum);
    printf("Final result: %d\n", final_result);
    
    /* Return predictable value for verification */
    return (final_result > 0) ? 0 : 1;
}
