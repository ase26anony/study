/* loop-doloop-test.c
 * Test program to exercise GCC's doloop optimization validation logic.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -c loop-doloop-test.c
 */

#include <stdint.h>

/* Global array to prevent loop removal */
static int arr[100] = {0};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: (plus reg -1) compared to 0 */
    do {
        sum += arr[i % 100];
        i++;
    } while (count-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--count > 5) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (count > 0) {
        sum += arr[i % 100];
        i++;
        count -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (count) {
        sum += arr[i % 100];
        i++;
        count = count - 1;  /* Separate decrement operation */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_decrement(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (count--) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern F: Post-decrement in condition with > 0 */
unsigned int test_postdec_gt_zero(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should match: while (count-- > 0) */
    while (count-- > 0) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern G: Pre-decrement compare to zero */
unsigned int test_predec_to_zero(unsigned int count) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This might match depending on RTL generation */
    while (--count != 0) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Main function to ensure all tests are called */
int main(void) {
    volatile unsigned int results = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Call all test functions to ensure compilation */
    results += test_exact_pattern(1000);
    results += test_compare_nonzero(1000);
    results += test_decrement_by_two(1000);
    results += test_complex_compare(1000);
    results += test_while_decrement(1000);
    results += test_postdec_gt_zero(1000);
    results += test_predec_to_zero(1000);
    
    return (int)results;
}
