/* loop-doloop-test.c
 * Test program to exercise GCC's loop doloop optimization validation logic.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops loop-doloop-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256];

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[n % 256];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should fail: cmp_arg2 != const0_rtx */
    while (--n > 5) {
        sum += arr[n % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    while (n > 0) {
        sum += arr[n % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This may fail: GET_CODE(cmp_arg1) != PLUS
       or other checks depending on RTL generation */
    while (n) {
        sum += arr[n % 256];
        n--;  /* Separate decrement from compare */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int test_while_decrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This might also match the pattern */
    while (n--) {
        sum += arr[n % 256];
    }
    
    return sum;
}

/* Pattern F: Post-decrement with greater-than-zero comparison */
unsigned int test_postdec_gt_zero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This should match: while (n-- > 0) pattern */
    while (n-- > 0) {
        sum += arr[n % 256];
    }
    
    return sum;
}

/* Pattern G: Pre-decrement compare (should NOT match) */
unsigned int test_predec_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    
    /* This uses pre-decrement, different pattern */
    while (--n != 0) {
        sum += arr[n % 256];
    }
    
    return sum;
}

/* Main function to ensure all tests are called */
int main(void) {
    unsigned int result;
    volatile unsigned int sink; /* Prevent dead code elimination */
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
    
    /* Call each test function */
    result = test_exact_match(1000);
    sink = result;
    
    result = test_compare_nonzero(1000);
    sink = result;
    
    result = test_decrement_by_two(1000);
    sink = result;
    
    result = test_complex_compare(1000);
    sink = result;
    
    result = test_while_decrement(1000);
    sink = result;
    
    result = test_postdec_gt_zero(1000);
    sink = result;
    
    result = test_predec_compare(1000);
    sink = result;
    
    printf("All tests executed. Results: %u\n", sink);
    
    return 0;
}
