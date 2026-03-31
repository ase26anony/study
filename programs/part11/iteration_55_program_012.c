/* test_doloop.c - Program to exercise GCC's loop doloop optimization validation */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
volatile int global_array[100] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[n % 100];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += global_array[n % 100];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += global_array[n % 100];
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += global_array[n % 100];
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_decrement(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += global_array[n % 100];
    }
    
    return sum;
}

/* Pattern F: Post-decrement with greater-than-zero comparison */
unsigned int pattern_f_postdec_gt_zero(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* Should match: n-- > 0 becomes PLUS(-1), COMPARE with 0 */
    do {
        sum += global_array[n % 100];
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement (won't match because PLUS comes before compare) */
unsigned int pattern_g_predecrement(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This likely won't match the exact pattern */
    while (--n) {
        sum += global_array[n % 100];
    }
    
    return sum;
}

/* Initialize array with some values */
void init_array(void) {
    for (int i = 0; i < 100; i++) {
        global_array[i] = i % 7;
    }
}

int main(void) {
    init_array();
    
    /* Call all patterns with different iteration counts */
    unsigned int results[7];
    
    results[0] = pattern_a_exact_match(1000);
    results[1] = pattern_b_compare_nonzero(1000);
    results[2] = pattern_c_decrement_by_two(1000);
    results[3] = pattern_d_complex_compare(1000);
    results[4] = pattern_e_while_decrement(1000);
    results[5] = pattern_f_postdec_gt_zero(1000);
    results[6] = pattern_g_predecrement(1000);
    
    /* Use results to prevent dead code elimination */
    global_sum = results[0] + results[1] + results[2] + 
                 results[3] + results[4] + results[5] + results[6];
    
    printf("Total sum: %d\n", global_sum);
    
    return 0;
}
