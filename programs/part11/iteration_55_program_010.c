/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
volatile int global_array[256];
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[counter % 256];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--counter > 5) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += global_array[counter % 256];
        counter -= 2;  /* Decrement by 2, not 1 */
    } while (counter > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (counter) {
        sum += global_array[counter % 256];
        counter--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_exact_pattern_while(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This should also match the pattern */
    while (counter--) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with > 0 */
unsigned int test_postdec_gt_zero(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This should match: counter-- > 0 */
    do {
        sum += global_array[counter % 256];
    } while (counter-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement compare to zero */
unsigned int test_predec_to_zero(unsigned int iterations) {
    unsigned int counter = iterations;
    unsigned int sum = 0;
    
    /* This might match depending on RTL generation */
    while (--counter != 0) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Initialize global array */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i % 64;
    }
}

int main(void) {
    init_array();
    
    unsigned int result = 0;
    unsigned int iterations = 1000;
    
    /* Call all test functions to ensure compilation */
    result += test_exact_pattern(iterations);
    result += test_compare_nonzero(iterations);
    result += test_decrement_by_two(iterations);
    result += test_complex_compare(iterations);
    result += test_exact_pattern_while(iterations);
    result += test_postdec_gt_zero(iterations);
    result += test_predec_to_zero(iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", result);
    
    return 0;
}
