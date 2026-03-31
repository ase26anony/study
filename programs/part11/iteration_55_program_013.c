/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop elimination */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_match(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* Compare against 5, not zero */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement in condition */
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Pre-decrement compare to zero */
unsigned int test_predecrement_zero(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This might match depending on RTL generation */
    while (--n != 0) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern G: Signed counter (should still work) */
int test_signed_counter(void) {
    int sum = 0;
    int n = 100;
    int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern H: Nested loops to increase complexity */
unsigned int test_nested_loops(void) {
    unsigned int sum = 0;
    unsigned int outer = 10;
    unsigned int inner = 5;
    
    while (outer--) {
        unsigned int temp = inner;
        while (temp--) {  /* Inner loop might match */
            sum += arr[temp & 255];
        }
    }
    
    return sum;
}

/* Main function to call all tests */
int main(void) {
    volatile unsigned int result = 0;
    
    /* Call all test functions to ensure compilation */
    result += test_exact_match();
    result += test_compare_nonzero();
    result += test_decrement_by_two();
    result += test_complex_compare();
    result += test_while_postdecrement();
    result += test_predecrement_zero();
    result += test_signed_counter();
    result += test_nested_loops();
    
    /* Use result to prevent dead code elimination */
    return (int)(result % 256);
}
