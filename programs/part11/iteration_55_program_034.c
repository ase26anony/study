/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int global_array[256] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
int test_exact_pattern(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[n % 256];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
int test_compare_nonzero(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
int test_decrement_by_two(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += global_array[n % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
int test_complex_compare(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += global_array[n % 256];
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
int test_exact_pattern_while(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* This should also match the pattern */
    while (n--) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern F: Post-decrement in while condition */
int test_postdecrement_while(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* Should match: while (counter-- > 0) pattern */
    while (n-- > 0) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern G: Pre-decrement (should still match in some cases) */
int test_predecrement(unsigned int iterations) {
    volatile int sum = 0;
    unsigned int n = iterations;
    
    /* Pre-decrement might still generate PLUS with -1 */
    while (--n) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Initialize array with some values */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i % 64;
    }
}

int main(void) {
    init_array();
    
    /* Call all test functions to ensure compilation */
    int results[7];
    
    results[0] = test_exact_pattern(1000);
    results[1] = test_compare_nonzero(1000);
    results[2] = test_decrement_by_two(1000);
    results[3] = test_complex_compare(1000);
    results[4] = test_exact_pattern_while(1000);
    results[5] = test_postdecrement_while(1000);
    results[6] = test_predecrement(1000);
    
    /* Use results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    printf("Total: %d\n", total);
    
    return 0;
}
