/* test-loop-doloop.c
 * Target compilation: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter * 2);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter * 3);
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
        if (counter > 0) {
            do {
                total += (i * j);
                global_sum += 3;
            } while (--counter != 0);  /* Should generate the target pattern */
        }
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int sum = 0;
    unsigned int u_counter = (n > 0) ? n : 1;
    int s_counter = n;
    
    /* Unsigned do-while */
    if (u_counter > 0) {
        do {
            sum += 1;
            global_sum += 4;
        } while (--u_counter != 0);
    }
    
    /* Signed while */
    while (s_counter-- != 0) {
        sum += 2;
        global_sum += 5;
    }
    
    return sum;
}

/* Main driver that uses argc to provide loop bounds */
int main(int argc, char *argv[]) {
    int base_count;
    int result = 0;
    
    /* Use command line argument for loop bound, with defaults */
    if (argc > 1) {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 5;
        if (base_count > 100) base_count = 100; /* Prevent excessive iteration */
    } else {
        base_count = 7;
    }
    
    printf("Testing with base_count = %d\n", base_count);
    
    /* Execute all test variants */
    result += test_do_while_predec(base_count);
    result += test_while_postdec(base_count);
    result += test_nested_loops(3, base_count);
    result += test_mixed_types(base_count);
    
    printf("Result: %d (global_sum: %d)\n", result, global_sum);
    
    /* Return deterministic result for verification */
    return (result > 0) ? 0 : 1;
}
