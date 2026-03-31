/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256];

/* Initialize array to prevent optimization */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
}

/* PATTERN A: Exact match for decrement-and-compare-to-zero pattern
   Should trigger the validation logic at lines 136-150 */
unsigned int pattern_a_exact_match(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - most likely to generate
       the PLUS with -1 compared to const0_rtx pattern */
    do {
        sum += arr[i++ % 256];
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return sum;
}

/* PATTERN B: Decrement but compare against non-zero constant
   Should fail at cmp_arg2 != const0_rtx check */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {  /* Pre-decrement compare to non-zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* PATTERN C: Decrement by value other than 1
   Should fail at XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration */
    while (n > 0) {
        sum += arr[i++ % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* PATTERN D: Complex compare source (separate decrement)
   May fail at GET_CODE(cmp_arg1) != PLUS check */
unsigned int pattern_d_complex_compare(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* Separate decrement and compare operations */
    while (n) {
        sum += arr[i++ % 256];
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Additional variant: while loop with post-decrement */
unsigned int pattern_e_while_postdecrement(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* while with post-decrement - also likely to match */
    while (n--) {  /* Post-decrement in condition */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Variant with signed counter to test different RTL patterns */
int pattern_f_signed_counter(void) {
    int sum = 0;
    int n = 100;
    int i = 0;
    
    do {
        sum += arr[i++ % 256];
    } while (n-- > 0);  /* Signed post-decrement compare to zero */
    
    return sum;
}

/* Main function to ensure all patterns are compiled and executed */
int main(void) {
    unsigned int results[6];
    
    init_array();
    
    /* Call all pattern functions to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_postdecrement();
    results[5] = pattern_f_signed_counter();
    
    /* Print results to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        printf("Pattern %d result: %u\n", i, results[i]);
    }
    
    return 0;
}
