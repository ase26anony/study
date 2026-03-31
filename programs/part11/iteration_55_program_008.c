/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-and-compare-against-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i % 256];
        i = (i + 1) % 256;
    } while (n-- != 0);  /* Post-decrement compare against zero */
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* Compare against 5, not zero */
        sum += arr[i % 256];
        i = (i + 1) % 256;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i % 256];
        i = (i + 1) % 256;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i % 256];
        i = (i + 1) % 256;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement in condition */
        sum += arr[i % 256];
        i = (i + 1) % 256;
    }
    
    return sum;
}

/* Pattern F: For loop that might match */
unsigned int test_for_pattern(unsigned int iterations) {
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* For loop with decrement */
    for (unsigned int n = iterations; n-- > 0; ) {
        sum += arr[i % 256];
        i = (i + 1) % 256;
    }
    
    return sum;
}

/* Pattern G: Do-while with pre-decrement (won't match) */
unsigned int test_predecrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement won't match PLUS with -1 pattern */
    do {
        sum += arr[i % 256];
        i = (i + 1) % 256;
    } while (--n != 0);
    
    return sum;
}

int main(void) {
    volatile unsigned int result;  /* volatile to prevent optimization */
    unsigned int iterations = 1000;
    
    printf("Testing doloop optimization patterns...\n");
    
    /* Call all test functions to ensure compilation */
    result = test_exact_pattern(iterations);
    printf("Pattern A result: %u\n", result);
    
    result = test_compare_nonzero(iterations);
    printf("Pattern B result: %u\n", result);
    
    result = test_decrement_by_two(iterations);
    printf("Pattern C result: %u\n", result);
    
    result = test_complex_compare(iterations);
    printf("Pattern D result: %u\n", result);
    
    result = test_while_pattern(iterations);
    printf("Pattern E result: %u\n", result);
    
    result = test_for_pattern(iterations);
    printf("Pattern F result: %u\n", result);
    
    result = test_predecrement(iterations);
    printf("Pattern G result: %u\n", result);
    
    return 0;
}
