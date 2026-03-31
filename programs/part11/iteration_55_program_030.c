/* test_doloop_patterns.c
 * This program contains various loop patterns designed to trigger
 * GCC's doloop optimization pass validation logic.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
volatile int global_array[256];
volatile int global_sum = 0;

/* Initialize array to prevent optimization */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i % 64;
    }
}

/* PATTERN A: Exact match for decrement-and-compare-against-zero
 * Should produce: PLUS with -1, COMPARE with const0_rtx
 */
unsigned int pattern_a_exact_match(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    /* do-while with post-decrement to zero - most likely to match */
    do {
        sum += global_array[counter % 256];
    } while (counter-- != 0);
    
    return sum;
}

/* PATTERN A2: Another exact match variant
 * while loop with post-decrement in condition
 */
unsigned int pattern_a2_while_postdec(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    while (counter--) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* PATTERN B: Compare against non-zero constant
 * Should fail: cmp_arg2 != const0_rtx check
 */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    /* Compare against 5 instead of 0 */
    while (counter-- > 5) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* PATTERN B2: Another non-zero compare variant
 * Should also fail const0_rtx check
 */
unsigned int pattern_b2_compare_positive(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    do {
        sum += global_array[counter % 256];
    } while (--counter > 100);  /* Compare against 100, not 0 */
    
    return sum;
}

/* PATTERN C: Decrement by value other than 1
 * Should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) check
 */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    /* Decrement by 2 each iteration */
    while (counter) {
        sum += global_array[counter % 256];
        counter -= 2;  /* Not -1 */
    }
    
    return sum;
}

/* PATTERN C2: Decrement by variable amount
 * Should also fail the GEN_INT(-1) check
 */
unsigned int pattern_c2_decrement_variable(void) {
    unsigned int counter = 1000;
    unsigned int step = 3;
    unsigned int sum = 0;
    
    while (counter) {
        sum += global_array[counter % 256];
        counter -= step;  /* Variable decrement, not constant -1 */
    }
    
    return sum;
}

/* PATTERN D: Complex compare source (separate decrement)
 * Should fail: GET_CODE(cmp_arg1) != PLUS check
 * because decrement and compare are separate operations
 */
unsigned int pattern_d_separate_decrement(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    /* Separate decrement and compare operations */
    while (1) {
        sum += global_array[counter % 256];
        counter--;
        if (counter == 0) break;
    }
    
    return sum;
}

/* PATTERN D2: Compare with arithmetic in condition
 * Might produce different RTL pattern
 */
unsigned int pattern_d2_complex_condition(void) {
    unsigned int counter = 1000;
    unsigned int sum = 0;
    
    /* Complex condition that's not a simple PLUS */
    while ((counter - 1) != 0) {
        sum += global_array[counter % 256];
        counter--;
    }
    
    return sum;
}

/* PATTERN E: Signed counter (should still work if pattern matches) */
int pattern_e_signed_counter(void) {
    int counter = 1000;
    int sum = 0;
    
    do {
        sum += global_array[abs(counter) % 256];
    } while (counter-- > 0);
    
    return sum;
}

/* PATTERN F: Nested loops to increase chances of pattern matching */
unsigned int pattern_f_nested_loops(void) {
    unsigned int outer = 100;
    unsigned int inner = 10;
    unsigned int sum = 0;
    
    while (outer--) {
        unsigned int temp_inner = inner;
        while (temp_inner--) {
            sum += global_array[(outer + temp_inner) % 256];
        }
    }
    
    return sum;
}

/* Main function to call all patterns */
int main(void) {
    unsigned int results[12];
    
    init_array();
    
    /* Call each pattern function */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_a2_while_postdec();
    results[2] = pattern_b_compare_nonzero();
    results[3] = pattern_b2_compare_positive();
    results[4] = pattern_c_decrement_by_two();
    results[5] = pattern_c2_decrement_variable();
    results[6] = pattern_d_separate_decrement();
    results[7] = pattern_d2_complex_condition();
    results[8] = pattern_e_signed_counter();
    results[9] = pattern_f_nested_loops();
    
    /* Store to volatile to prevent dead code elimination */
    global_sum = results[0] + results[1] + results[2] + results[3] +
                 results[4] + results[5] + results[6] + results[7] +
                 results[8] + results[9];
    
    printf("Total sum: %u\n", global_sum);
    
    return 0;
}
