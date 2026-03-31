/* test-doloop-pattern.c
 * Designed to trigger specific RTL pattern in loop-doloop.cc lines 136-150
 * Pattern: SET with COMPARE of (PLUS reg -1) against const0_rtx
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
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

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter = (unsigned int)n;
    
    /* Force the pattern with explicit decrement */
    while (counter != 0) {
        local_sum += (int)counter;
        global_sum += 4;
        counter--;
    }
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    while (counter) {
        local_sum += counter;
        global_sum += 5;
        --counter;  /* Separate decrement, but condition checks counter */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but ensure minimum iterations */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_mixed_types(base_iterations);
    int result5 = test_countdown(base_iterations);
    
    int total = result1 + result2 + result3 + result4 + result5;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Total result: %d\n", total);
    printf("Global sum: %d\n", global_sum);
    
    /* Return consistent value for test verification */
    return (total > 0 && global_sum > 0) ? 0 : 1;
}
