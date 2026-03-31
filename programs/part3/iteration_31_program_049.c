/* test-doloop-pattern.c
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
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Test 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Test 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while with pre-decrement */
        do {
            local_sum += (i * j) & 0xF;
            global_sum += 3;
            j = counter;  /* Use counter in computation */
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Test 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Use unsigned for the decrement pattern */
    counter = (unsigned int)n;
    while (counter-- != 0) {
        local_sum += (int)counter * 2;
        global_sum += 4;
    }
    
    return local_sum;
}

/* Test 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Direct countdown from n to 1 */
    while (n != 0) {
        local_sum += n;
        global_sum += 5;
        n--;  /* Decrement in body, but still forms the pattern */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 1000;
    int total = 0;
    
    /* Use command line argument for variability, but ensure minimum iterations */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Execute all test patterns */
    total += test_do_while_predec(base_iterations);
    total += test_while_postdec(base_iterations / 2);
    total += test_nested_loops(5, base_iterations / 5);
    total += test_mixed_types(base_iterations / 3);
    total += test_countdown(base_iterations / 4);
    
    /* Add global_sum to ensure all loops executed */
    total += global_sum;
    
    printf("Result: %d (global_sum: %d)\n", total, global_sum);
    
    /* Return deterministic result for verification */
    return (total > 0) ? 0 : 1;
}
