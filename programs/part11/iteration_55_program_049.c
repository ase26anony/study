/* Test program for GCC loop doloop optimization pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop elimination */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare to zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* Compare against 5, not zero */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n) {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (1) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
        if (n == 0) break;  /* Compare in separate statement */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int pattern_e_while_postdecrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement in while condition */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Pre-decrement compare to zero */
unsigned int pattern_f_predecrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This might match depending on RTL generation */
    while (--n) {  /* Pre-decrement */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern G: Signed counter (should still work) */
int pattern_g_signed_counter(void) {
    int n = 100;
    int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);  /* Signed post-decrement */
    
    return sum;
}

/* Pattern H: Loop with early exit (complex control flow) */
unsigned int pattern_h_early_exit(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
        if (sum > 1000) break;  /* Additional exit condition */
    } while (n-- != 0);
    
    return sum;
}

int main(void) {
    unsigned int results[8];
    
    /* Call all patterns to ensure compilation */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_postdecrement();
    results[5] = pattern_f_predecrement();
    results[6] = pattern_g_signed_counter();
    results[7] = pattern_h_early_exit();
    
    /* Use results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    printf("Total: %u\n", total);
    return 0;
}
