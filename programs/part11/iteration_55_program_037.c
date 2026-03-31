/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop elimination */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int limit) {
    unsigned int n = limit;
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
unsigned int test_compare_nonzero(unsigned int limit) {
    unsigned int n = limit;
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
unsigned int test_decrement_by_two(unsigned int limit) {
    unsigned int n = limit;
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
unsigned int test_complex_compare(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
        /* Compare happens implicitly in while condition */
    }
    
    return sum;
}

/* Pattern E: Another exact pattern with while loop */
unsigned int test_while_pattern(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Post-decrement in complex expression */
unsigned int test_complex_expression(unsigned int limit) {
    unsigned int n = limit;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* More complex expression that might still generate PLUS(-1) */
    do {
        sum += arr[i & 255];
        i++;
    } while ((n-- > 0) && (i < 1000));
    
    return sum;
}

/* Main function to ensure all tests are called */
int main() {
    volatile unsigned int results[6];
    
    /* Call each test function with different limits */
    results[0] = test_exact_pattern(100);
    results[1] = test_compare_nonzero(100);
    results[2] = test_decrement_by_two(100);
    results[3] = test_complex_compare(100);
    results[4] = test_while_pattern(100);
    results[5] = test_complex_expression(100);
    
    /* Return sum of all results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 6; i++) {
        total += results[i];
    }
    
    return total > 0 ? 0 : 1;
}
