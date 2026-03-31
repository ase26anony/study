/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop removal */
volatile int global_array[256];
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should generate: PLUS with -1, compare with const0_rtx */
    do {
        sum += global_array[n % 256];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += global_array[n % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += global_array[n % 256];
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern F: Counter in register but with different type */
int test_int_counter(int iterations) {
    int n = iterations;
    int sum = 0;
    
    /* Using int instead of unsigned int */
    do {
        sum += global_array[(n & 0xFF)];
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Loop with pre-decrement (should still match in some cases) */
unsigned int test_predecrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    while (--n) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Initialize global array */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
}

int main(void) {
    init_array();
    
    unsigned int result = 0;
    
    /* Call all test functions to ensure compilation */
    result += test_exact_pattern(1000);
    result += test_compare_nonzero(1000);
    result += test_decrement_by_two(1000);
    result += test_complex_compare(1000);
    result += test_while_postdecrement(1000);
    result += test_int_counter(1000);
    result += test_predecrement(1000);
    
    /* Store result to prevent dead code elimination */
    global_sum = result;
    
    return global_sum > 0 ? 0 : 1;
}
