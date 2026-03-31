/* loop-doloop-coverage.c
 * Test program to cover GCC's loop doloop validation logic
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -c loop-doloop-coverage.c
 */

#include <stdint.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5, not zero - should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 - should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i & 0xFF];
        i++;
        n -= 2;  /* Not -1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare - may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i & 0xFF];
        i++;
        n--;  /* Decrement in body, not in compare */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int test_exact_match_while(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Post-decrement in while condition - should also match */
    while (n--) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register but with different type */
int test_signed_counter(int iterations) {
    int n = iterations;
    int sum = 0;
    int i = 0;
    
    /* Signed counter, but same pattern */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern G: Loop with pre-decrement (won't match exact pattern) */
unsigned int test_predecrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement - different pattern */
    while (--n) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Main function to ensure all tests are called */
int main(void) {
    unsigned int total = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Call all test functions to ensure compilation */
    total += test_exact_match(1000);
    total += test_compare_nonzero(1000);
    total += test_decrement_by_two(1000);
    total += test_complex_compare(1000);
    total += test_exact_match_while(1000);
    total += test_signed_counter(1000);
    total += test_predecrement(1000);
    
    /* Use volatile to prevent dead code elimination */
    volatile unsigned int result = total;
    
    return (result > 0) ? 0 : 1;
}
