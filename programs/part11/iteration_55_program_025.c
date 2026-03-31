/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop removal */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
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
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int pattern_e_while_exact(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement to zero */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: For loop that might match */
unsigned int pattern_f_for_loop(void) {
    unsigned int sum = 0;
    unsigned int i;
    
    /* Some for loops might generate the pattern */
    for (i = 100; i-- > 0; ) {  /* Post-decrement in condition */
        sum += arr[i & 255];
    }
    
    return sum;
}

/* Pattern G: Different counter type */
unsigned int pattern_g_int_counter(void) {
    int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Using int instead of unsigned int */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern H: Nested loops - outer might match */
unsigned int pattern_h_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 5;
    unsigned int sum = 0;
    unsigned int i, j;
    
    /* Outer loop might match the pattern */
    do {
        for (j = 0; j < inner; j++) {
            sum += arr[(i + j) & 255];
        }
        i++;
    } while (outer-- != 0);
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main(void) {
    unsigned int results[8];
    
    /* Call all patterns to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_exact();
    results[5] = pattern_f_for_loop();
    results[6] = pattern_g_int_counter();
    results[7] = pattern_h_nested_loops();
    
    /* Use results to prevent dead code elimination */
    volatile unsigned int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    return total > 0 ? 0 : 1;
}
