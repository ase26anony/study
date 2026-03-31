/* test_doloop_patterns.c
 * 
 * This program contains various loop patterns designed to trigger
 * GCC's loop doloop optimization pass validation logic.
 * The goal is to cover lines 136-150 in loop-doloop.cc which check
 * for specific decrement-and-compare RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256];

/* Initialize array to prevent optimization */
__attribute__((constructor))
static void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-by-1 and compare-to-zero
 * Should produce: PLUS with -1, COMPARE with const0_rtx
 */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero */
    do {
        sum += arr[i++ % 256];
    } while (n-- != 0);  /* Post-decrement and compare to zero */
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant
 * Should fail: cmp_arg2 != const0_rtx check
 */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {  /* Pre-decrement and compare to 5 */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1
 * Should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) check
 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration */
    while (n) {
        sum += arr[i++ % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (not simple PLUS)
 * Should fail: GET_CODE(cmp_arg1) != PLUS check
 */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare operations */
    while (1) {
        sum += arr[i++ % 256];
        n--;  /* Separate decrement instruction */
        if (n == 0) break;  /* Compare in separate statement */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop
 * Should also trigger the validation logic
 */
unsigned int pattern_e_while_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement */
    while (n--) {  /* Post-decrement to zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern F: Counter in register with different type
 * Should still produce the pattern
 */
unsigned int pattern_f_int_counter(void) {
    int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i++ % 256];
    } while (n-- > 0);  /* Post-decrement with > 0 comparison */
    
    return sum;
}

/* Pattern G: Nested loops to ensure multiple validation attempts */
unsigned int pattern_g_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    while (outer--) {
        unsigned int temp = inner;
        while (temp--) {  /* Inner loop should match pattern */
            sum += arr[i++ % 256];
        }
    }
    
    return sum;
}

/* Main function to call all patterns and prevent dead code elimination */
int main(void) {
    volatile unsigned int results[8];
    int idx = 0;
    
    /* Call each pattern and store results */
    results[idx++] = pattern_a_exact_match();
    results[idx++] = pattern_b_compare_nonzero();
    results[idx++] = pattern_c_decrement_by_two();
    results[idx++] = pattern_d_complex_compare();
    results[idx++] = pattern_e_while_exact_match();
    results[idx++] = pattern_f_int_counter();
    results[idx++] = pattern_g_nested_loops();
    
    /* Print a summary to prevent optimization */
    printf("Results: ");
    for (int i = 0; i < idx; i++) {
        printf("%u ", results[i]);
    }
    printf("\n");
    
    return 0;
}
