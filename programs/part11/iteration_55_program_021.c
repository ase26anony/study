/* Test program for GCC loop doloop optimization pass coverage */
#include <stdio.h>

/* Global array to prevent loop removal */
volatile int arr[100] = {0};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i % 100];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i % 100];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i % 100];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_decrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with > 0 */
unsigned int pattern_f_postdec_gt_zero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Should match: while (n-- > 0) */
    while (n-- > 0) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern G: Pre-decrement compare to zero */
unsigned int pattern_g_predec_to_zero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Should match: while (--n != 0) */
    while (--n != 0) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

int main() {
    unsigned int result = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Call all patterns to ensure compilation */
    result += pattern_a_exact_match(1000);
    result += pattern_b_compare_nonzero(1000);
    result += pattern_c_decrement_by_two(1000);
    result += pattern_d_complex_compare(1000);
    result += pattern_e_while_decrement(1000);
    result += pattern_f_postdec_gt_zero(1000);
    result += pattern_g_predec_to_zero(1000);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", result);
    
    return 0;
}
