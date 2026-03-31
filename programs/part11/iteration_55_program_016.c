/* test_doloop.c - Exercise GCC's doloop optimization validation logic */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - should match the pattern */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 - should fail cmp_arg2 != const0_rtx */
    while (--n > 5) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration - should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    while (n) {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Not decrement by 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare - may fail GET_CODE(cmp_arg1) != PLUS */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Decrement in body, not in compare */
    }
    
    return sum;
}

/* Pattern E: Another exact match with while loop */
unsigned int pattern_e_while_exact(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register with simple decrement */
unsigned int pattern_f_simple_decrement(void) {
    register unsigned int n = 100;  /* Hint to keep in register */
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to test multiple contexts */
unsigned int pattern_g_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        /* Inner loop with exact pattern */
        unsigned int n = inner;
        do {
            sum += arr[(i + j) & 255];
            j++;
        } while (n-- != 0);
    }
    
    return sum;
}

/* Pattern H: Loop with if condition inside */
unsigned int pattern_h_conditional_inside(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    while (n--) {
        sum += arr[i & 255];
        i++;
        /* Conditional to prevent over-optimization */
        if (sum > 1000) {
            sum -= 500;
        }
    }
    
    return sum;
}

/* Main function to call all patterns */
int main(void) {
    unsigned int results[8];
    unsigned int total = 0;
    
    /* Call each pattern function */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_exact();
    results[5] = pattern_f_simple_decrement();
    results[6] = pattern_g_nested_loops();
    results[7] = pattern_h_conditional_inside();
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    printf("Total: %u\n", total);
    
    return total > 0 ? 0 : 1;
}
