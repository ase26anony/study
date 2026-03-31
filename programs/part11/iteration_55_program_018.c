/* loop-doloop-test.c
 * Test program to cover GCC's loop doloop optimization validation logic.
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -c loop-doloop-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop elimination */
static int arr[1000] = {0};

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern */
unsigned int test_pattern_a(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i % 1000];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_pattern_b(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i % 1000];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_pattern_c(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += arr[i % 1000];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_pattern_d(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check
     * because decrement and compare might be separate operations */
    while (n) {
        sum += arr[i % 1000];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int test_pattern_e(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i % 1000];
        i++;
    }
    
    return sum;
}

/* Pattern F: Exact match with signed counter */
int test_pattern_f(int iterations) {
    int n = iterations;
    int sum = 0;
    int i = 0;
    
    /* Signed counter should still generate the pattern */
    do {
        sum += arr[i % 1000];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern G: Counter in register but complex exit condition */
unsigned int test_pattern_g(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Complex condition that might not match simple pattern */
    do {
        sum += arr[i % 1000];
        i++;
    } while (n-- && i < 500);  /* Multiple conditions */
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main() {
    volatile unsigned int result = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 1000; i++) {
        arr[i] = i % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += test_pattern_a(1000);
    result += test_pattern_b(1000);
    result += test_pattern_c(1000);
    result += test_pattern_d(1000);
    result += test_pattern_e(1000);
    result += test_pattern_f(1000);
    result += test_pattern_g(1000);
    
    printf("Result: %u\n", result);
    return 0;
}
