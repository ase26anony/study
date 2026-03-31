/* test-doloop.cc - Test for doloop optimization pattern matching */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop.cc */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 0x1);  /* Simple operation using counter */
        global_sum++;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
        local_sum += (n & 0x3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop should show the pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        /* Inner loop with decrement pattern */
        while (j-- != 0) {
            local_sum += i * j;
            global_sum += 3;
        }
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int iterations) {
    int local_sum = 0;
    unsigned int counter;
    
    /* Convert to unsigned for the decrementing loop */
    counter = (unsigned int)iterations;
    
    do {
        local_sum += (counter % 5);
        global_sum += 4;
    } while (--counter != 0);
    
    return local_sum;
}

/* Variant 5: simple countdown loop */
int test_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* Explicit countdown loop */
    while (n) {
        local_sum += n;
        global_sum += 5;
        n--;  /* Decrement in body, but condition checks n != 0 */
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    int result = 0;
    
    /* Use command line argument for variable but predictable iteration count */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
        if (base_iterations > 10000) base_iterations = 10000; /* Reasonable limit */
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    /* Test all variants with different iteration counts to stress the pattern matcher */
    result += test_dowhile_predec(base_iterations);
    result += test_while_postdec(base_iterations / 2);
    result += test_nested_loops(10, base_iterations / 10);
    result += test_mixed_types(base_iterations);
    result += test_countdown(base_iterations);
    
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    /* Return deterministic result for verification */
    return (result > 0) ? 0 : 1;
}
