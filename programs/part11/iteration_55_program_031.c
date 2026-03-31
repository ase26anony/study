/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdint.h>

/* Global array to prevent loop elimination */
volatile int arr[256] = {0};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - should generate PLUS with -1 */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);  /* n-- > 0 would also work */
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement and compare against 5, not zero */
    while (--n > 5) {  /* Should fail cmp_arg2 != const0_rtx check */
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum + n;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration */
    while (n) {  /* Separate decrement instruction */
        sum += arr[i & 0xFF];
        i++;
        n -= 2;  /* Should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (not simple PLUS) */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare operations */
    while (1) {
        sum += arr[i & 0xFF];
        i++;
        n--;  /* Decrement as separate instruction */
        if (n == 0) break;  /* Compare as separate operation */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_decrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {  /* Should generate the desired pattern */
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register but with different type */
unsigned int pattern_f_int_counter(void) {
    int n = 100;
    int sum = 0;
    unsigned int i = 0;
    
    /* Signed int counter, still should keep in register */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase chances */
unsigned int pattern_g_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        /* Inner loop with exact pattern */
        unsigned int n = inner;
        do {
            sum += arr[(i + j) & 0xFF];
            j++;
        } while (n-- != 0);
    }
    
    return sum;
}

/* Pattern H: Loop with early exit (might affect pattern) */
unsigned int pattern_h_early_exit(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 0xFF];
        i++;
        if (sum > 1000) break;  /* Early exit */
    } while (n-- != 0);
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main(void) {
    volatile unsigned int results[8];
    
    /* Call all patterns to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_decrement();
    results[5] = pattern_f_int_counter();
    results[6] = pattern_g_nested_loops();
    results[7] = pattern_h_early_exit();
    
    /* Use results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    return total > 0 ? 0 : 1;
}
