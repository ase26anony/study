/* test_doloop_patterns.c
 * 
 * This program contains various loop patterns designed to exercise
 * GCC's loop doloop optimization pass validation logic.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops test_doloop_patterns.c
 */

#include <stdio.h>

/* Global array to prevent loop removal */
volatile int global_array[100] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-against-zero */
unsigned int pattern_a_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[i++ % 100];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Compare against 5 instead of 0 - should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += global_array[i++ % 100];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Decrement by 2 - should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n) {
        sum += global_array[i++ % 100];
        n -= 2;  /* Not -1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Separate decrement and compare - may not produce simple PLUS pattern */
    while (n) {
        sum += global_array[i++ % 100];
        n--;
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* This while pattern might also generate the desired RTL */
    while (n--) {
        sum += global_array[i++ % 100];
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with > 0 comparison */
unsigned int pattern_f_postdec_gt_zero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Post-decrement with > 0 comparison */
    do {
        sum += global_array[i++ % 100];
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement compare against zero */
unsigned int pattern_g_predec_compare_zero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    int i = 0;
    
    /* Pre-decrement might generate different pattern */
    while (--n != 0) {
        sum += global_array[i++ % 100];
    }
    
    return sum;
}

/* Helper to initialize array */
void init_array(void) {
    for (int i = 0; i < 100; i++) {
        global_array[i] = i % 7;
    }
}

int main(void) {
    init_array();
    
    unsigned int result = 0;
    
    /* Call each pattern to ensure compilation generates the RTL */
    result += pattern_a_exact_match(1000);
    result += pattern_b_compare_nonzero(1000);
    result += pattern_c_decrement_by_two(1000);
    result += pattern_d_complex_compare(1000);
    result += pattern_e_while_exact_match(1000);
    result += pattern_f_postdec_gt_zero(1000);
    result += pattern_g_predec_compare_zero(1000);
    
    /* Store in volatile to prevent optimization */
    global_sum = result;
    
    printf("Result: %u\n", result);
    printf("Global sum: %d\n", global_sum);
    
    return 0;
}
