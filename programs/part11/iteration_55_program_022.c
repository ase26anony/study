/* Test program for GCC loop doloop pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>

/* Global array to prevent loop elimination */
volatile int global_array[100] = {0};
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare to zero */
unsigned int test_exact_pattern(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += global_array[n % 100];
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {
        sum += global_array[n % 100];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += global_array[n % 100];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    while (n) {
        sum += global_array[n % 100];
        n--;  /* Separate decrement instruction */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    /* This should also match the pattern */
    while (n--) {
        sum += global_array[n % 100];
    }
    
    return sum;
}

/* Pattern F: Counter in register with simple decrement */
unsigned int test_simple_counter(unsigned int iterations) {
    register unsigned int counter asm("r12") = iterations;
    int sum = 0;
    
    /* Force counter into register, exact pattern */
    do {
        sum += global_array[counter % 100];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase chances */
unsigned int test_nested_loops(unsigned int iterations) {
    unsigned int outer = iterations / 10;
    unsigned int inner;
    int sum = 0;
    
    while (outer--) {
        inner = 10;
        do {
            sum += global_array[(outer + inner) % 100];
        } while (inner-- != 0);
    }
    
    return sum;
}

/* Pattern H: Loop with if condition inside */
unsigned int test_conditional_inside(unsigned int iterations) {
    unsigned int n = iterations;
    int sum = 0;
    
    do {
        if (n % 2) {
            sum += global_array[n % 100];
        } else {
            sum -= global_array[n % 100];
        }
    } while (n-- != 0);
    
    return sum;
}

int main() {
    unsigned int iterations = 1000;
    int total = 0;
    
    /* Initialize array with some values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Call all test functions to generate various loop patterns */
    total += test_exact_pattern(iterations);
    total += test_compare_nonzero(iterations);
    total += test_decrement_by_two(iterations);
    total += test_complex_compare(iterations);
    total += test_while_postdecrement(iterations);
    total += test_simple_counter(iterations);
    total += test_nested_loops(iterations);
    total += test_conditional_inside(iterations);
    
    /* Store result to prevent elimination */
    global_sum = total;
    
    printf("Total: %d\n", total);
    return 0;
}
