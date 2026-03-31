/* test-doloop-coverage.c
 * 
 * This program contains various loop patterns designed to exercise
 * GCC's loop doloop optimization pass validation logic.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops test-doloop-coverage.c -o test-doloop
 */

#include <stdio.h>

/* Global array to prevent loop removal */
volatile int global_array[100] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern
 * Should generate: PLUS with -1, compared to const0_rtx
 */
int test_pattern_a(void) {
    unsigned int counter = 1000;
    int sum = 0;
    
    /* do-while with post-decrement to zero - most likely to match */
    do {
        sum += global_array[counter % 100];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant
 * Should fail: cmp_arg2 != const0_rtx check
 */
int test_pattern_b(void) {
    unsigned int counter = 1000;
    int sum = 0;
    
    /* Compare against 5 instead of 0 */
    while (counter-- > 5) {
        sum += global_array[counter % 100];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1
 * Should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) check
 */
int test_pattern_c(void) {
    unsigned int counter = 1000;
    int sum = 0;
    
    /* Decrement by 2 each iteration */
    while (counter > 0) {
        sum += global_array[counter % 100];
        counter -= 2;  /* Not -1, should fail validation */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement)
 * May fail: GET_CODE(cmp_arg1) != PLUS check
 */
int test_pattern_d(void) {
    unsigned int counter = 1000;
    int sum = 0;
    
    /* Separate decrement and compare */
    while (counter) {
        sum += global_array[counter % 100];
        counter--;  /* Decrement in body, not in compare */
    }
    
    return sum;
}

/* Pattern E: Another variant of exact match with while loop */
int test_pattern_e(void) {
    unsigned int counter = 1000;
    int sum = 0;
    
    /* while with post-decrement - should also match */
    while (counter--) {
        sum += global_array[counter % 100];
    }
    
    return sum;
}

/* Pattern F: Exact match with different counter type */
int test_pattern_f(void) {
    int counter = 1000;
    int sum = 0;
    
    /* Signed int counter, still should match */
    do {
        sum += global_array[counter % 100];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern G: Counter used in complex way (should still try to match) */
int test_pattern_g(void) {
    unsigned int counter = 1000;
    unsigned int i = 0;
    int sum = 0;
    
    /* Counter used in index calculation */
    do {
        sum += global_array[i++ % 100];
    } while (counter-- != 0);
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main(void) {
    int results[7];
    
    /* Initialize global array with some values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i % 10;
    }
    
    /* Call all test patterns to ensure they're compiled */
    results[0] = test_pattern_a();
    results[1] = test_pattern_b();
    results[2] = test_pattern_c();
    results[3] = test_pattern_d();
    results[4] = test_pattern_e();
    results[5] = test_pattern_f();
    results[6] = test_pattern_g();
    
    /* Use results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    printf("Total: %d\n", total);
    return 0;
}
