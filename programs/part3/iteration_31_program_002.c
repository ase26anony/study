/* test-doloop-pattern.c
 * Target: PowerPC or other architectures with condition code registers
 * Compile with: -O2 -march=powerpc64 -fdump-rtl-doloop -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
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

/* Variant 3: nested loops - inner loop should show pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    
    for (unsigned int i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            total += (i * counter);
            global_sum += 3;
        } while (--counter != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int sum = 0;
    unsigned int u_counter = (unsigned int)n;
    int s_counter = n;
    
    /* First loop with unsigned */
    while (u_counter-- != 0) {
        sum += 1;
        global_sum += 4;
    }
    
    /* Second loop with signed */
    do {
        sum -= 1;
        global_sum += 5;
    } while (--s_counter != 0);
    
    return sum;
}

/* Main driver with control flow */
int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but keep it reasonable */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 1000) base_iterations = 1000;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    int result1 = test_do_while_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_nested_loops(5, base_iterations / 5);
    int result4 = test_mixed_types(base_iterations);
    
    int final_result = result1 + result2 + result3 + result4 + global_sum;
    
    printf("Results: %d, %d, %d, %d, global=%d\n", 
           result1, result2, result3, result4, global_sum);
    printf("Final checksum: %d\n", final_result);
    
    /* Return predictable value for test verification */
    return (final_result > 0) ? 0 : 1;
}
