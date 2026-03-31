/* Test program for doloop optimization with specific RTL pattern:
   SET with COMPARE of (PLUS reg -1) against const0_rtx
   Targeting architectures with condition code registers (PowerPC, SPARC) */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 0x1);  /* Simple operation using counter */
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

/* Variant 3: Simple counted loop */
int test_simple_loop(unsigned int n) {
    int local_sum = 0;
    unsigned int i = n;
    
    /* Force the pattern: while (i-- != 0) */
    while (i != 0) {
        local_sum += i;
        global_sum += 3;
        i--;  /* Decrement at end */
    }
    
    return local_sum;
}

/* Variant 4: Nested loops - inner loop should show the pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        /* Inner loop with decrement pattern */
        while (j-- != 0) {
            local_sum += (i * j);
            global_sum += 4;
        }
    }
    
    return local_sum;
}

/* Variant 5: Different integer type - unsigned short */
int test_short_loop(unsigned short n) {
    int local_sum = 0;
    unsigned short counter = n;
    
    do {
        local_sum += counter;
        global_sum += 5;
    } while (--counter != 0);
    
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
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_simple_loop(base_iterations);
    int result4 = test_nested_loops(5, base_iterations / 5);
    int result5 = test_short_loop(base_iterations % 256);
    
    /* Combine results to prevent optimization */
    int total = result1 + result2 + result3 + result4 + result5 + global_sum;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Global sum: %d\n", global_sum);
    printf("Total: %d\n", total);
    
    /* Return predictable value for verification */
    return (total > 0) ? 0 : 1;
}
