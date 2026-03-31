/* test_doloop_patterns.c
 * This program contains various loop patterns to exercise GCC's
 * doloop optimization validation logic (lines 136-150 in loop-doloop.cc)
 */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop removal */
volatile int arr[100] = {0};
volatile int sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    int local_sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        local_sum += arr[n % 100];
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return local_sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    int local_sum = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* Compare against 5, not zero */
        local_sum += arr[n % 100];
    }
    
    return local_sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    int local_sum = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        local_sum += arr[n % 100];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return local_sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    int local_sum = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        local_sum += arr[n % 100];
        n--;  /* Separate decrement instruction */
    }
    
    return local_sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_exact_pattern_while(unsigned int iterations) {
    unsigned int n = iterations;
    int local_sum = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement to zero */
        local_sum += arr[n % 100];
    }
    
    return local_sum;
}

/* Pattern F: Loop with pre-decrement (should still match) */
unsigned int test_predecrement_pattern(unsigned int iterations) {
    unsigned int n = iterations + 1;  /* Adjust for pre-decrement */
    int local_sum = 0;
    
    /* This might generate different RTL but could still match */
    while (--n) {
        local_sum += arr[n % 100];
    }
    
    return local_sum;
}

/* Pattern G: Signed counter (should still work) */
int test_signed_counter(int iterations) {
    int n = iterations;
    int local_sum = 0;
    
    do {
        local_sum += arr[abs(n) % 100];
    } while (n-- > 0);
    
    return local_sum;
}

/* Pattern H: Nested loops to increase optimization opportunities */
unsigned int test_nested_loops(unsigned int outer, unsigned int inner) {
    unsigned int i = outer;
    unsigned int j;
    int local_sum = 0;
    
    do {
        j = inner;
        do {
            local_sum += arr[(i + j) % 100];
        } while (j-- != 0);
    } while (i-- != 0);
    
    return local_sum;
}

/* Main function to call all test patterns */
int main() {
    unsigned int result = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Call each test pattern with different iteration counts */
    result += test_exact_pattern(1000);
    result += test_compare_nonzero(1000);
    result += test_decrement_by_two(1000);
    result += test_complex_compare(1000);
    result += test_exact_pattern_while(1000);
    result += test_predecrement_pattern(1000);
    result += test_signed_counter(1000);
    result += test_nested_loops(100, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Total result: %u\n", result);
    
    return 0;
}
