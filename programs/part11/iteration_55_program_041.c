/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop elimination */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(void) {
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

/* Pattern E: Another exact match variant with while loop */
unsigned int test_while_decrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Post-decrement with greater-than-zero comparison */
unsigned int test_postdec_gt_zero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This might match depending on RTL generation */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Pre-decrement compare to zero */
unsigned int test_predec_to_zero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This might generate different RTL */
    while (--n != 0) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Main function to ensure all tests are called */
int main(void) {
    volatile unsigned int results[7];
    
    results[0] = test_exact_match();
    results[1] = test_compare_nonzero();
    results[2] = test_decrement_by_two();
    results[3] = test_complex_compare();
    results[4] = test_while_decrement();
    results[5] = test_postdec_gt_zero();
    results[6] = test_predec_to_zero();
    
    /* Return non-zero if any test returned zero (unlikely) */
    return !(results[0] && results[1] && results[2] && 
             results[3] && results[4] && results[5] && results[6]);
}
