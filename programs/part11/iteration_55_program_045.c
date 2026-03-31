/* test-doloop-coverage.c
 * 
 * This program contains various loop patterns designed to trigger
 * GCC's doloop optimization pass validation logic, specifically
 * covering lines 136-150 in loop-doloop.cc.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int global_array[256];
volatile int global_sum = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern
 * Should generate: PLUS with -1, compared to const0_rtx
 */
int pattern_a_exact_match(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* do-while with post-decrement to zero - most likely to match */
    do {
        sum += global_array[counter % 256];
    } while (counter-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant
 * Should fail: cmp_arg2 != const0_rtx check
 */
int pattern_b_compare_nonzero(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Compare against 5 instead of 0 */
    while (counter-- > 5) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1
 * Should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) check
 */
int pattern_c_decrement_by_two(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Decrement by 2 each iteration */
    while (counter > 0) {
        sum += global_array[counter % 256];
        counter -= 2;  /* Not -1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement)
 * Should potentially fail: GET_CODE(cmp_arg1) != PLUS check
 * or other checks depending on RTL generation
 */
int pattern_d_complex_compare(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Separate decrement and compare */
    while (1) {
        sum += global_array[counter % 256];
        if (counter == 0) break;
        counter--;
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop
 * Should also match the decrement-and-compare pattern
 */
int pattern_e_while_decrement(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* while with post-decrement */
    while (counter--) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Pattern F: Pre-decrement instead of post-decrement
 * Might generate different RTL pattern
 */
int pattern_f_pre_decrement(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Pre-decrement version */
    while (--counter) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Pattern G: Signed counter with > 0 comparison
 * Should still match if it generates PLUS -1 pattern
 */
int pattern_g_signed_counter(void) {
    int counter = 100;
    int sum = 0;
    
    do {
        sum += global_array[counter % 256];
    } while (counter-- > 0);
    
    return sum;
}

/* Pattern H: Loop with multiple exit conditions
 * Should fail validation due to complex control flow
 */
int pattern_h_complex_exit(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    while (counter) {
        sum += global_array[counter % 256];
        if (sum > 1000) break;  /* Additional exit condition */
        counter--;
    }
    
    return sum;
}

/* Main function to ensure all patterns are compiled */
int main(void) {
    int results[8];
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i % 64;
    }
    
    /* Call all pattern functions to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_decrement();
    results[5] = pattern_f_pre_decrement();
    results[6] = pattern_g_signed_counter();
    results[7] = pattern_h_complex_exit();
    
    /* Use results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    printf("Total: %d\n", total);
    return total > 0 ? 0 : 1;
}
