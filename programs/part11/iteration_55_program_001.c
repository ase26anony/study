/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern */
unsigned int test_exact_pattern(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_compare_nonzero(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
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
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_decrement(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Loop with pre-decrement (might still match) */
unsigned int test_predecrement(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
    } while (--n);
    
    return sum;
}

/* Main function to call all test patterns */
int main(void) {
    volatile unsigned int results[6];
    
    /* Call each test function to ensure loops are compiled */
    results[0] = test_exact_pattern();
    results[1] = test_compare_nonzero();
    results[2] = test_decrement_by_two();
    results[3] = test_complex_compare();
    results[4] = test_while_decrement();
    results[5] = test_predecrement();
    
    /* Print results to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        printf("Result %d: %u\n", i, results[i]);
    }
    
    return 0;
}
