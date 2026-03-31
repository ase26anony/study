/* Test program for GCC loop doloop pass validation logic */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Initialize array with some values */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern */
/* Should trigger the validation logic and potentially pass all checks */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);  /* n-- compared to 0 */
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
/* Should fail at cmp_arg2 != const0_rtx check */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {  /* n-- compared to 5, not 0 */
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
/* Should fail at XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration */
    while (n) {
        sum += arr[i & 0xFF];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
/* May fail at GET_CODE(cmp_arg1) != PLUS or other checks */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare */
    while (1) {
        sum += arr[i & 0xFF];
        i++;
        n--;  /* Separate decrement instruction */
        if (n == 0) break;  /* Compare in separate statement */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
/* Should also trigger the validation logic */
unsigned int pattern_e_while_decrement(void) {
    unsigned int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement */
    while (n--) {  /* n-- implicitly compared to 0 */
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register but with different type */
/* Should still generate the pattern */
unsigned int pattern_f_signed_counter(void) {
    int n = 1000;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Signed counter, still should decrement by 1 */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern G: Nested loops to ensure multiple passes */
unsigned int pattern_g_nested_loops(void) {
    unsigned int outer = 100;
    unsigned int inner = 10;
    unsigned int sum = 0;
    
    while (outer--) {
        unsigned int temp = inner;
        while (temp--) {  /* Inner loop with decrement-and-compare */
            sum += arr[(outer + temp) & 0xFF];
        }
    }
    
    return sum;
}

/* Main function to call all patterns and prevent optimization */
int main(void) {
    unsigned int results[8];
    volatile unsigned int dummy; /* Prevent dead code elimination */
    
    init_array();
    
    /* Call all patterns and store results */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_decrement();
    results[5] = pattern_f_signed_counter();
    results[6] = pattern_g_nested_loops();
    
    /* Use results to prevent optimization */
    dummy = 0;
    for (int i = 0; i < 7; i++) {
        dummy += results[i];
    }
    
    printf("Test completed. Dummy sum: %u\n", dummy);
    
    return 0;
}
