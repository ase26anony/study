/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop elimination */
volatile int arr[256] = {0};
volatile int sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        local_sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);
    
    return local_sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* This should fail: cmp_arg2 != const0_rtx */
    while (--n > 5) {
        local_sum += arr[i & 0xFF];
        i++;
    }
    
    return local_sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* This should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    do {
        local_sum += arr[i & 0xFF];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return local_sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* This may fail: GET_CODE(cmp_arg1) != PLUS */
    while (n) {
        local_sum += arr[i & 0xFF];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return local_sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_decrement(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        local_sum += arr[i & 0xFF];
        i++;
    }
    
    return local_sum;
}

/* Pattern F: Post-decrement with comparison */
unsigned int test_postdecrement_compare(unsigned int n) {
    unsigned int local_sum = 0;
    unsigned int i = 0;
    
    /* Alternative exact match pattern */
    do {
        local_sum += arr[i & 0xFF];
        i++;
    } while (n-- > 0);
    
    return local_sum;
}

/* Main function to ensure all tests are called */
int main() {
    unsigned int result = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Call all test functions with different loop counts */
    result += test_exact_pattern(1000);
    result += test_compare_nonzero(1000);
    result += test_decrement_by_two(1000);
    result += test_complex_compare(1000);
    result += test_while_decrement(1000);
    result += test_postdecrement_compare(1000);
    
    /* Store result to prevent dead code elimination */
    sum = result;
    
    return sum > 0 ? 0 : 1;
}
