/* test_doloop.c - Program to exercise GCC's loop doloop optimization validation */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
}

/* Pattern A: Exact match for decrement-by-1 compare-against-zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - should match the pattern */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);  /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 - should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* This compares against 5, not zero */
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
    
    /* Decrement by 2 each iteration - should fail GEN_INT(-1) check */
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
    
    /* Separate decrement and compare - may not produce the PLUS pattern */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
        /* Compare happens implicitly in while condition */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {  /* This should also generate the pattern */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Negative test - incrementing loop */
unsigned int pattern_f_incrementing_loop(void) {
    unsigned int n = 0;
    unsigned int sum = 0;
    
    /* Incrementing loop - won't match decrement pattern at all */
    for (n = 0; n < 100; n++) {
        sum += arr[n & 255];
    }
    
    return sum;
}

/* Pattern G: Exact match with different data type */
unsigned int pattern_g_int_exact_match(void) {
    int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Using signed int - should still match */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Main function to ensure all patterns are compiled and executed */
int main(void) {
    volatile unsigned int results[7]; /* volatile to prevent optimization */
    
    /* Call all pattern functions to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_exact_match();
    results[5] = pattern_f_incrementing_loop();
    results[6] = pattern_g_int_exact_match();
    
    /* Print results to prevent dead code elimination */
    for (int i = 0; i < 7; i++) {
        printf("Pattern %d result: %u\n", i, results[i]);
    }
    
    return 0;
}
