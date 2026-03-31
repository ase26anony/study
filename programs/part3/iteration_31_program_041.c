/* test-doloop-pattern.c
 * Target compilation: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -fdump-rtl-all test-doloop-pattern.c -o test-doloop
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement compare against 0 */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement compare against 0 */
        local_sum += (counter & 3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            total += (i * j);
            global_sum += 3;
            j = counter;
        } while (--counter != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int sum = 0;
    unsigned int u_counter = (unsigned int)n;
    int s_counter = n;
    
    /* Unsigned counter loop */
    while (u_counter-- != 0) {
        sum += 1;
        global_sum += 4;
    }
    
    /* Signed counter loop */
    do {
        sum += 2;
        global_sum += 5;
    } while (--s_counter != 0);
    
    return sum;
}

/* Variant 5: Simple decrementing loop with function parameter */
int test_param_loop(unsigned int iterations) {
    int result = 0;
    unsigned int count = iterations;
    
    while (count != 0) {
        result += (count & 0xF);
        global_sum += 6;
        count--;  /* Decrement in body, compare at top */
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variable loop bounds */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) {
            base_iterations = 50;
        }
        if (base_iterations > 10000) {
            base_iterations = 10000;
        }
    }
    
    printf("Testing doloop patterns with base iterations = %d\n", base_iterations);
    
    int total = 0;
    
    /* Execute all test variants */
    total += test_do_while_predec(base_iterations);
    total += test_while_postdec(base_iterations);
    total += test_nested_loops(5, base_iterations / 5);
    total += test_mixed_types(base_iterations / 2);
    total += test_param_loop(base_iterations);
    
    printf("Total computation result: %d\n", total);
    printf("Global side-effect sum: %d\n", global_sum);
    
    /* Verify expected results */
    int expected_global = 
        base_iterations +                     /* test_do_while_predec */
        2 * base_iterations +                 /* test_while_postdec */
        3 * 5 * (base_iterations / 5) +       /* test_nested_loops */
        (4 * base_iterations / 2) +           /* test_mixed_types (unsigned part) */
        (5 * base_iterations / 2) +           /* test_mixed_types (signed part) */
        6 * base_iterations;                  /* test_param_loop */
    
    if (global_sum == expected_global) {
        printf("SUCCESS: All loops executed correctly\n");
        return 0;
    } else {
        printf("WARNING: Loop execution count mismatch\n");
        return 1;
    }
}
