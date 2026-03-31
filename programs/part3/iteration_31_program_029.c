/* Test program for doloop optimization pattern matching */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += 1;           /* Simple body to avoid optimization */
        global_sum += 1;
    } while (--n != 0);           /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {            /* Post-decrement in condition */
        local_sum += 2;
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: Nested loops - inner loop should match pattern */
int test_nested(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            local_sum += 3;
            global_sum += 3;
        } while (--j != 0);       /* Inner do-while with pre-decrement */
    }
    
    return local_sum;
}

/* Variant 4: Mixed signed/unsigned */
int test_mixed_types(int iterations) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Force counter to be positive */
    counter = (iterations > 0) ? iterations : 10;
    
    while (counter-- != 0) {      /* Post-decrement with unsigned */
        local_sum += 4;
        global_sum += 4;
    }
    
    return local_sum;
}

/* Variant 5: Simple decrementing loop with different condition */
int test_simple_decrement(unsigned int n) {
    int local_sum = 0;
    
    if (n == 0) return 0;
    
    do {
        local_sum += 5;
        global_sum += 5;
        n--;                      /* Decrement in body */
    } while (n != 0);             /* Compare against 0 */
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Test all variants */
    result += test_dowhile_predec(base_iterations);
    result += test_while_postdec(base_iterations);
    result += test_nested(10, base_iterations / 10);
    result += test_mixed_types(base_iterations);
    result += test_simple_decrement(base_iterations);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Verify correctness */
    int expected = base_iterations * 1 +      /* variant 1 */
                   base_iterations * 2 +      /* variant 2 */
                   (10 * (base_iterations / 10)) * 3 +  /* variant 3 */
                   (base_iterations > 0 ? base_iterations : 10) * 4 + /* variant 4 */
                   base_iterations * 5;       /* variant 5 */
    
    if (result == expected) {
        printf("PASS: All loops executed correctly\n");
        return 0;
    } else {
        printf("FAIL: Expected %d, got %d\n", expected, result);
        return 1;
    }
}
