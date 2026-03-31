/* Test program for doloop optimization pattern matching */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0xFF);  /* Simple computation */
        global_sum += (counter & 0xFF);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0xFF);
        global_sum += (counter & 0xFF);
    }
    
    return local_sum;
}

/* Variant 3: Simple decrementing for loop */
int test_for_loop(unsigned int n) {
    int local_sum = 0;
    unsigned int i;
    
    for (i = n; i != 0; --i) {  /* Pre-decrement in update */
        local_sum += (i & 0xFF);
        global_sum += (i & 0xFF);
    }
    
    return local_sum;
}

/* Variant 4: Nested loops - inner loop should match pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        while (counter-- != 0) {
            local_sum += ((i + counter) & 0xFF);
            global_sum += ((i + counter) & 0xFF);
        }
    }
    
    return local_sum;
}

/* Variant 5: Mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter = (unsigned int)n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0xFF);
        global_sum += (counter & 0xFF);
    } while (--counter != 0);
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variability, but ensure loops run */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 1000) base_iterations = 1000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Test all variants */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_for_loop(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_mixed_types(base_iterations);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return 0 if all loops executed (global_sum should be > 0) */
    return (global_sum > 0) ? 0 : 1;
}
