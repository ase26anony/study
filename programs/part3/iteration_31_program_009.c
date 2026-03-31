/* test-doloop-pattern.c
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c
 * Or: powerpc64-linux-gnu-gcc -O2 -fdump-rtl-doloop -S test-doloop-pattern.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    if (counter == 0) return 0;
    
    do {
        local_sum += (counter & 0xFF);  /* Simple non-empty body */
        global_sum += (counter & 0x1);
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 0xFF);
        global_sum ^= (counter & 0x1);
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        /* Inner loop should generate the target pattern */
        while (j-- != 0) {
            total += (i * j);
            global_sum += (i + j) & 1;
        }
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int sum = 0;
    unsigned int u_counter = (unsigned int)n;
    int s_counter = n;
    
    /* First loop: unsigned decrement */
    while (u_counter-- != 0) {
        sum += u_counter * 2;
    }
    
    /* Second loop: signed decrement */
    do {
        sum -= s_counter;
        global_sum += sum & 1;
    } while (--s_counter > 0);  /* Note: > 0 instead of != 0 for variety */
    
    return sum;
}

/* Variant 5: simple countdown loop that should generate clean pattern */
int test_countdown(unsigned int iterations) {
    int result = 0;
    unsigned int count = iterations;
    
    /* This should generate the cleanest (PLUS reg -1) pattern */
    if (count == 0) return 0;
    
    do {
        result += count;
        global_sum = (global_sum + 1) & 0xFF;
    } while (--count != 0);
    
    return result;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but ensure it's reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 1000) base_iterations = 1000;
    }
    
    int total_result = 0;
    
    /* Execute all test variants */
    total_result += test_do_while_predec(base_iterations);
    total_result += test_while_postdec(base_iterations);
    total_result += test_nested_loops(5, base_iterations / 5);
    total_result += test_mixed_types(base_iterations);
    total_result += test_countdown(base_iterations);
    
    /* Print result to prevent optimization and verify execution */
    printf("Result: %d (Global: %d)\n", total_result, global_sum);
    
    return total_result != 0 ? 0 : 1;
}
