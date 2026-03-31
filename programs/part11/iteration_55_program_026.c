/* Test program for GCC loop-doloop pass coverage
 * Targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
volatile int arr[100] = {[0 ... 99] = 1};
volatile int result = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_match(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i % 100];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i % 100];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check
     * because decrement and compare might be separate */
    while (n) {
        sum += arr[i % 100];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(void) {
    unsigned int sum = 0;
    unsigned int n = 100;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register with simple decrement */
unsigned int test_simple_counter(void) {
    unsigned int sum = 0;
    register unsigned int counter asm("r12") = 50;  /* Hint to use register */
    
    /* Force counter into register */
    asm volatile("" : "+r"(counter));
    
    do {
        sum += arr[counter % 100];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase optimization opportunities */
unsigned int test_nested_loops(void) {
    unsigned int sum = 0;
    unsigned int outer = 10;
    unsigned int inner = 5;
    
    while (outer--) {
        unsigned int temp_inner = inner;
        while (temp_inner--) {
            sum += arr[(outer + temp_inner) % 100];
        }
    }
    
    return sum;
}

/* Main function to ensure all tests are called */
int main(void) {
    unsigned int results[7];
    
    /* Call all test functions */
    results[0] = test_exact_match();
    results[1] = test_compare_nonzero();
    results[2] = test_decrement_by_two();
    results[3] = test_complex_compare();
    results[4] = test_while_postdecrement();
    results[5] = test_simple_counter();
    results[6] = test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    unsigned int total = 0;
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    printf("Total: %u\n", total);
    
    /* Also use volatile store */
    result = total;
    
    return (total > 0) ? 0 : 1;
}
