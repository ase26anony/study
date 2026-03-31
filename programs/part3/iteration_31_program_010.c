/* test-loop-doloop.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 0x1);  /* Simple non-empty body */
        global_sum++;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
        local_sum += (n & 0x3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            total += (counter & 0x7);
            global_sum += 3;
        } while (--counter != 0);  /* Should generate the target pattern */
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int iterations) {
    int sum = 0;
    unsigned int u_counter = (unsigned int)iterations;
    int s_counter = iterations;
    
    /* Unsigned counter loop */
    while (u_counter-- != 0) {
        sum += 1;
        global_sum += 4;
    }
    
    /* Signed counter loop */
    do {
        sum -= 1;
        global_sum += 5;
    } while (--s_counter != 0);
    
    return sum;
}

/* Main driver that uses all variants */
int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but keep it reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]) % 1000;
        if (base_iterations <= 0) base_iterations = 100;
    }
    
    int result = 0;
    
    /* Test each variant with different iteration counts */
    result += test_do_while_predec(base_iterations);
    result += test_while_postdec(base_iterations / 2);
    result += test_nested_loops(5, base_iterations / 5);
    result += test_mixed_types(base_iterations / 4);
    
    /* Print results to ensure execution */
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    return (result != 0) ? 0 : 1;  /* Non-zero return if all loops were optimized away */
}
