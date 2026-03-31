/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int global_array[100] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (counter-- > 5) {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (counter > 0) {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
        counter -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (counter) {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
        counter--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_pattern(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should also match the pattern */
    while (counter--) {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with > 0 */
unsigned int test_postdecrement_gt_zero(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This might match depending on RTL generation */
    do {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
    } while (counter-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement (should not match) */
unsigned int test_predecrement(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Pre-decrement won't match the PLUS with -1 pattern */
    while (--counter) {
        sum += global_array[i % 100];
        i = (i + 1) % 100;
    }
    
    return sum;
}

/* Main function to call all test patterns */
int main() {
    unsigned int iterations = 1000;
    unsigned int results[7];
    
    /* Initialize global array with some values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i % 10;
    }
    
    /* Execute all test patterns */
    results[0] = test_exact_pattern(iterations);
    results[1] = test_compare_nonzero(iterations);
    results[2] = test_decrement_by_two(iterations);
    results[3] = test_complex_compare(iterations);
    results[4] = test_while_pattern(iterations);
    results[5] = test_postdecrement_gt_zero(iterations);
    results[6] = test_predecrement(iterations);
    
    /* Use results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    printf("Total sum: %u\n", total);
    
    return 0;
}
