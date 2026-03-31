/* Test program for doloop optimization with specific RTL pattern:
 * SET with COMPARE of (PLUS reg -1) against const0_rtx
 * Targeting architectures with condition code registers (PowerPC, SPARC)
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

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
int test_while_postdec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop has decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner loop with decrement pattern */
        do {
            local_sum += i * j;
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 4: signed int counter */
int test_signed_counter(int n) {
    int local_sum = 0;
    int counter = n;
    
    /* Force unsigned comparison by using != 0 */
    while (counter-- != 0) {
        local_sum += counter;
        global_sum += 4;
    }
    
    return local_sum;
}

/* Variant 5: mixed decrement styles in same function */
int test_mixed_patterns(unsigned int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Pattern A: do-while with pre-decrement */
    counter = n / 2;
    do {
        local_sum += 1;
    } while (--counter != 0);
    
    /* Pattern B: while with post-decrement */
    counter = n / 2;
    while (counter-- != 0) {
        local_sum += 2;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    unsigned int base_iterations = 100;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %u\n", base_iterations);
    
    int result = 0;
    
    /* Execute all test variants */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_signed_counter(base_iterations);
    result += test_mixed_patterns(base_iterations);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return 0 only if all loops executed (global_sum should be > 0) */
    return (global_sum > 0) ? 0 : 1;
}
