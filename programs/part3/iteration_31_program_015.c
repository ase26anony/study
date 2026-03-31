/* test-doloop-pattern.c
 * Target: PowerPC or other architectures with condition code registers
 * Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-doloop-pattern.c
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        local_sum += (counter & 3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        do {
            total += (i * j);
            global_sum += 3;
        } while (--j != 0);
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int sum = 0;
    unsigned int counter = (unsigned int)n;
    
    /* Force the pattern with explicit comparison */
    while (counter != 0) {
        sum += (int)counter;
        global_sum += 4;
        counter--;  /* Decrement after use */
    }
    
    return sum;
}

/* Variant 5: Another do-while variant with different operation */
int test_another_dowhile(unsigned int n) {
    int result = 0;
    unsigned int count = n;
    
    if (count == 0) return 0;
    
    do {
        result ^= count;  /* XOR operation */
        global_sum += 5;
    } while (--count != 0);
    
    return result;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    /* Use command line argument for variability, but ensure minimum iterations */
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations < 10) base_iterations = 10;
        if (base_iterations > 10000) base_iterations = 10000;
    }
    
    int total_result = 0;
    
    /* Execute all variants with different iteration counts */
    total_result += test_do_while_predec(base_iterations);
    total_result += test_while_postdec(base_iterations / 2);
    total_result += test_nested_loops(5, base_iterations / 5);
    total_result += test_mixed_types(base_iterations / 3);
    total_result += test_another_dowhile(base_iterations / 4);
    
    /* Print results to ensure execution */
    printf("Total result: %d\n", total_result);
    printf("Global sum: %d\n", global_sum);
    
    /* Simple validation */
    if (global_sum > 0 && total_result != 0) {
        printf("Test completed successfully\n");
        return 0;
    } else {
        printf("Test failed - loops may have been optimized away\n");
        return 1;
    }
}
