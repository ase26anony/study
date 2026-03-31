/* Test program for GCC loop doloop optimization pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Global array to prevent loop removal */
volatile int arr[100] = {[0 ... 99] = 1};

/* Pattern A: Exact match for decrement-and-compare to zero */
unsigned int test_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
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
    unsigned int n = 100;
    unsigned int sum = 0;
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
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    do {
        sum += arr[i % 100];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    } while (n > 0);
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += arr[i % 100];
        i++;
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += arr[i % 100];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register with simple decrement */
unsigned int test_simple_decrement(void) {
    register unsigned int n asm("r12") = 100;  /* Hint to use register */
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i % 100];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase chances */
unsigned int test_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    
    while (outer--) {
        unsigned int temp = inner;
        while (temp--) {
            sum += arr[(outer + temp) % 100];
        }
    }
    
    return sum;
}

int main(void) {
    unsigned int results[7];
    
    /* Call all test functions to ensure compilation */
    results[0] = test_exact_match();
    results[1] = test_compare_nonzero();
    results[2] = test_decrement_by_two();
    results[3] = test_complex_compare();
    results[4] = test_while_postdecrement();
    results[5] = test_simple_decrement();
    results[6] = test_nested_loops();
    
    /* Print results to prevent dead code elimination */
    for (int i = 0; i < 7; i++) {
        printf("Result %d: %u\n", i, results[i]);
    }
    
    return 0;
}
