/* Test program for GCC loop doloop optimization pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop elimination */
static int arr[256] = {0};

/* Pattern A: Exact match for decrement-and-compare to zero */
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

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
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
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i & 0xFF];
        i++;
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
        sum += arr[i & 0xFF];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern F: Signed counter (should still work) */
int test_signed_counter(int iterations) {
    int n = iterations;
    int sum = 0;
    int i = 0;
    
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase optimization opportunities */
unsigned int test_nested_loops(unsigned int outer, unsigned int inner) {
    unsigned int o = outer;
    unsigned int sum = 0;
    
    do {
        unsigned int i = inner;
        unsigned int j = 0;
        
        /* Inner loop with exact pattern */
        while (i--) {
            sum += arr[j & 0xFF];
            j++;
        }
        
    } while (o-- != 0);
    
    return sum;
}

/* Initialize array with some values */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
}

int main(void) {
    init_array();
    
    /* Call all test functions with different iteration counts */
    volatile unsigned int result;  /* volatile to prevent optimization */
    
    result = test_exact_match(1000);
    printf("Test A result: %u\n", result);
    
    result = test_compare_nonzero(1000);
    printf("Test B result: %u\n", result);
    
    result = test_decrement_by_two(1000);
    printf("Test C result: %u\n", result);
    
    result = test_complex_compare(1000);
    printf("Test D result: %u\n", result);
    
    result = test_while_postdecrement(1000);
    printf("Test E result: %u\n", result);
    
    result = test_signed_counter(1000);
    printf("Test F result: %u\n", result);
    
    result = test_nested_loops(10, 100);
    printf("Test G result: %u\n", result);
    
    return 0;
}
