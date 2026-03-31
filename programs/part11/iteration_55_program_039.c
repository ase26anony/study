/* Test program for GCC loop-doloop pass coverage
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop removal */
volatile int arr[256] = {0};
volatile int result = 0;

/* Pattern A: Exact match for decrement-and-compare to zero */
unsigned int test_exact_match(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i++ % 256];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should fail: cmp_arg2 != const0_rtx */
    while (--n > 5) {
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    do {
        sum += arr[i++ % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    /* This may fail: GET_CODE(cmp_arg1) != PLUS
     * Decrement and compare are separate operations */
    while (n) {
        sum += arr[i++ % 256];
        n--;
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern F: Counter in register with simple decrement */
unsigned int test_simple_decrement(unsigned int count) {
    register unsigned int n asm("r12") = count;  /* Hint to keep in register */
    unsigned int sum = 0;
    int i = 0;
    
    do {
        sum += arr[i++ % 256];
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase optimization opportunities */
unsigned int test_nested_loops(unsigned int count) {
    unsigned int outer = count / 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    int i = 0;
    
    while (outer--) {
        unsigned int n = inner;
        do {
            sum += arr[i++ % 256];
        } while (n-- != 0);
    }
    
    return sum;
}

/* Pattern H: Loop with early exit (still should match pattern) */
unsigned int test_early_exit(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    int i = 0;
    
    do {
        sum += arr[i++ % 256];
        if (sum > 1000) break;
    } while (n-- != 0);
    
    return sum;
}

int main(void) {
    unsigned int count = 1000;
    unsigned int total = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
    
    /* Call all test functions to generate various loop patterns */
    total += test_exact_match(count);
    total += test_compare_nonzero(count);
    total += test_decrement_by_two(count);
    total += test_complex_compare(count);
    total += test_while_postdecrement(count);
    total += test_simple_decrement(count);
    total += test_nested_loops(count);
    total += test_early_exit(count);
    
    /* Store result to prevent dead code elimination */
    result = total;
    
    printf("Result: %u\n", total);
    return 0;
}
