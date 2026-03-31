/* loop-doloop-coverage.c
 * Test program to cover GCC's loop doloop validation logic
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -c loop-doloop-coverage.c
 */

#include <stdint.h>

/* Global array to prevent loop elimination */
volatile int arr[100] = {[0 ... 99] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
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
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    do {
        sum += arr[i % 100];
        i++;
    } while (--n > 5);  /* Compare against 5, not zero */
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += arr[i % 100];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check
     * because decrement and compare might be separate */
    while (n) {
        sum += arr[i % 100];
        i++;
        n--;  /* Separate decrement from compare */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_exact(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This might also generate the pattern */
    while (n--) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with do-while */
unsigned int pattern_f_postdec_do_while(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Classic post-decrement pattern */
    do {
        sum += arr[i % 100];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement with zero comparison */
unsigned int pattern_g_predec_zero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement might also work */
    while (--n != 0) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main(void) {
    volatile unsigned int results[7];
    
    /* Call all patterns to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_exact();
    results[5] = pattern_f_postdec_do_while();
    results[6] = pattern_g_predec_zero();
    
    /* Use results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    return total > 0 ? 0 : 1;
}
