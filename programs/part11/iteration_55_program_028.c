/* Test program for GCC loop doloop optimization pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop elimination */
static int arr[1000] = {[0 ... 999] = 1};

/* Pattern A: Exact match - decrement by 1 and compare to zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - should generate PLUS with -1 */
    do {
        sum += arr[i % 1000];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement and compare against 5, not zero */
    while (--n > 5) {
        sum += arr[i % 1000];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Subtract 2 each iteration - not decrement by 1 */
    while (n > 0) {
        sum += arr[i % 1000];
        i++;
        n -= 2;  /* Not -1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source - separate decrement */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare - may not produce PLUS in COMPARE */
    while (n) {
        sum += arr[i % 1000];
        i++;
        n--;  /* Decrement as separate statement */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_decrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {
        sum += arr[i % 1000];
        i++;
    }
    
    return sum;
}

/* Pattern F: Pre-decrement compare to zero */
unsigned int pattern_f_predecrement_zero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement to zero - may still match */
    while (--n != 0) {
        sum += arr[i % 1000];
        i++;
    }
    
    return sum;
}

/* Pattern G: Signed counter to test different RTL */
unsigned int pattern_g_signed_counter(void) {
    int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Signed counter decrement */
    do {
        sum += arr[i % 1000];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern H: Nested loops - outer loop might match */
unsigned int pattern_h_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    while (outer--) {
        unsigned int temp = inner;
        while (temp--) {
            sum += arr[i % 1000];
            i++;
        }
    }
    
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
    results[5] = pattern_f_predecrement_zero();
    results[6] = pattern_g_signed_counter();
    results[7] = pattern_h_nested_loops();
    
    /* Print something to prevent dead code elimination */
    printf("Results: ");
    for (int i = 0; i < 8; i++) {
        printf("%u ", results[i]);
    }
    printf("\n");
    
    return 0;
}
