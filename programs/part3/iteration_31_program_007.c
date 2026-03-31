/* test-doloop-pattern.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0x1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0x3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        if (counter != 0) {
            do {
                local_sum += i * j;
                global_sum += 3;
                j = counter;  /* Use counter in computation */
            } while (--counter != 0);
        }
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Use unsigned for the decrement pattern */
    counter = (n > 0) ? n : 1;
    
    do {
        local_sum += (int)counter;
        global_sum += 4;
    } while (--counter != 0);
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Explicit countdown pattern */
    while (n != 0) {
        local_sum += n;
        global_sum += 5;
        n--;  /* Decrement at end */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variability, but keep it reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Test all variants */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_mixed_types(base_iterations);
    result += test_countdown(base_iterations);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Validate correctness */
    int expected_global = 
        base_iterations +                    /* variant 1 */
        2 * base_iterations +                /* variant 2 */
        3 * 5 * (base_iterations / 5) +      /* variant 3 */
        4 * ((base_iterations > 0) ? base_iterations : 1) + /* variant 4 */
        5 * base_iterations;                 /* variant 5 */
    
    if (global_sum == expected_global) {
        printf("PASS: Global sum matches expected value\n");
        return 0;
    } else {
        printf("FAIL: Global sum %d != expected %d\n", global_sum, expected_global);
        return 1;
    }
}
