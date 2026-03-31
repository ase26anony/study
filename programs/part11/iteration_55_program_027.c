/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int global_array[256];
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* do-while with post-decrement to zero - should generate PLUS with -1 */
    do {
        sum += global_array[n % 256];
    } while (n-- != 0);  /* Should produce: (plus n -1) compared to 0 */
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_nonzero_compare(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* Compare against 5 instead of 0 - should fail cmp_arg2 != const0_rtx */
    while (--n > 5) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* Subtract 2 each iteration - should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    while (n > 0) {
        sum += global_array[n % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* Separate decrement and compare - may fail GET_CODE(cmp_arg1) != PLUS */
    while (n) {
        sum += global_array[n % 256];
        n--;  /* Decrement in body, not in compare */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int test_while_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {
        sum += global_array[n % 256];
    }
    
    return sum;
}

/* Pattern F: Exact match with different counter type */
int test_signed_counter(int iterations) {
    int n = iterations;
    int sum = 0;
    
    /* Signed counter, but same pattern */
    do {
        sum += global_array[abs(n) % 256];
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase optimization opportunities */
unsigned int test_nested_loops(unsigned int outer, unsigned int inner) {
    unsigned int i = outer;
    unsigned int total = 0;
    
    while (i--) {
        unsigned int j = inner;
        /* Inner loop with exact pattern */
        do {
            total += global_array[(i + j) % 256];
        } while (j-- != 0);
    }
    
    return total;
}

/* Initialize array with non-zero values */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i + 1;
    }
}

int main(void) {
    init_array();
    
    /* Call all test functions to ensure compilation */
    unsigned int results[8];
    
    results[0] = test_exact_pattern(100);
    results[1] = test_nonzero_compare(100);
    results[2] = test_decrement_by_two(100);
    results[3] = test_complex_compare(100);
    results[4] = test_while_pattern(100);
    results[5] = test_signed_counter(100);
    results[6] = test_nested_loops(10, 10);
    
    /* Use results to prevent dead code elimination */
    unsigned int final_sum = 0;
    for (int i = 0; i < 7; i++) {
        final_sum += results[i];
    }
    
    printf("Final sum: %u\n", final_sum);
    
    return 0;
}
