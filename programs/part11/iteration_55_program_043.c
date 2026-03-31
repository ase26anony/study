/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int global_array[256];
volatile int global_sum = 0;

/* Initialize array to prevent constant folding */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i % 64;
    }
}

/* PATTERN A: Exact match for decrement-and-compare-to-zero pattern
   Should trigger the validation logic at lines 136-150 */
int pattern_a_exact_match(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* do-while with post-decrement to zero - most likely to generate
       the desired PLUS(-1) compared to const0_rtx pattern */
    do {
        sum += global_array[counter % 256];
    } while (counter-- != 0);
    
    return sum;
}

/* PATTERN A2: Another exact match variant with while loop */
int pattern_a2_while_decrement(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* while with post-decrement - should also generate the pattern */
    while (counter--) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* PATTERN B: Compare against non-zero constant
   Should fail at cmp_arg2 != const0_rtx check */
int pattern_b_compare_nonzero(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Compare against 5 instead of 0 */
    while (--counter > 5) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* PATTERN C: Decrement by value other than 1
   Should fail at XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
int pattern_c_decrement_by_two(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Decrement by 2 each iteration */
    do {
        sum += global_array[counter % 256];
        counter -= 2;  /* Not a simple decrement by 1 */
    } while (counter > 0);
    
    return sum;
}

/* PATTERN D: Complex compare source
   Should fail at GET_CODE(cmp_arg1) != PLUS check */
int pattern_d_complex_compare(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Separate decrement and compare operations */
    do {
        sum += global_array[counter % 256];
        counter--;
    } while (counter != 0);
    
    return sum;
}

/* PATTERN E: Counter in register but with additional operations
   Tests edge cases in the validation */
int pattern_e_mixed_operations(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Mix of operations that might still produce the pattern */
    while (counter) {
        sum += global_array[counter % 256];
        /* The --counter > 0 might generate different patterns */
        if (--counter == 0) break;
    }
    
    return sum;
}

/* PATTERN F: Nested loops to test multiple instances */
int pattern_f_nested_loops(void) {
    unsigned int outer = 50;
    unsigned int inner = 20;
    int sum = 0;
    
    while (outer--) {
        unsigned int temp_inner = inner;
        /* Inner loop with exact pattern */
        while (temp_inner--) {
            sum += global_array[(outer + temp_inner) % 256];
        }
    }
    
    return sum;
}

/* PATTERN G: Loop with pre-decrement (might generate different RTL) */
int pattern_g_predecrement(void) {
    unsigned int counter = 100;
    int sum = 0;
    
    /* Pre-decrement version */
    while (--counter) {
        sum += global_array[counter % 256];
    }
    
    return sum;
}

/* Main function to call all patterns and ensure they're not optimized away */
int main(void) {
    int total = 0;
    
    init_array();
    
    /* Call all patterns and accumulate results to prevent dead code elimination */
    total += pattern_a_exact_match();
    total += pattern_a2_while_decrement();
    total += pattern_b_compare_nonzero();
    total += pattern_c_decrement_by_two();
    total += pattern_d_complex_compare();
    total += pattern_e_mixed_operations();
    total += pattern_f_nested_loops();
    total += pattern_g_predecrement();
    
    /* Store in global to ensure computation isn't optimized away */
    global_sum = total;
    
    printf("Total sum: %d\n", total);
    
    return 0;
}
