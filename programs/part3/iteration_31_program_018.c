/* Test program for doloop optimization with specific RTL pattern:
 * SET with COMPARE of (PLUS reg -1) against const0_rtx
 * Targeting architectures with condition code registers (PowerPC, SPARC)
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_dowhile_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple operation to prevent dead code elimination */
        global_sum += (counter & 1);
    } while (--counter != 0);  /* Pre-decrement in condition - should generate (PLUS reg -1) */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += counter;
        global_sum += (counter & 3);
    }
    
    return local_sum;
}

/* Variant 3: Simple counted loop with unsigned */
int test_simple_loop(unsigned int n) {
    int local_sum = 0;
    unsigned int i = n;
    
    while (i != 0) {
        local_sum += i;
        global_sum += (i & 7);
        i--;  /* Separate decrement */
    }
    
    return local_sum;
}

/* Variant 4: Nested loops - inner loop should have the pattern */
int test_nested_loops(int outer_iter, int inner_base) {
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        unsigned int inner_counter = inner_base + (o & 3);  /* Vary inner loop size slightly */
        
        /* Inner loop with decrement-and-branch pattern */
        while (inner_counter-- != 0) {
            total += (o * inner_counter);
            global_sum += (o & 1);
        }
    }
    
    return total;
}

/* Variant 5: Function with parameter to force runtime value */
int test_param_loop(int iterations) {
    int sum = 0;
    int count = iterations;
    
    /* Use do-while to ensure at least one iteration */
    if (count > 0) {
        do {
            sum += (count * 3);
            global_sum += (count & 15);
        } while (--count != 0);
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variable loop count, but with a minimum */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    printf("Testing doloop patterns with base iterations = %d\n", base_iterations);
    
    int result1 = test_dowhile_predec(base_iterations);
    int result2 = test_while_postdec(base_iterations);
    int result3 = test_simple_loop(base_iterations);
    int result4 = test_nested_loops(5, base_iterations / 5);
    int result5 = test_param_loop(base_iterations);
    
    int total_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Results: %d, %d, %d, %d, %d\n", result1, result2, result3, result4, result5);
    printf("Total result: %d\n", total_result);
    printf("Global sum: %d\n", global_sum);
    
    /* Return consistent value for test verification */
    return (total_result > 0 && global_sum > 0) ? 0 : 1;
}
