/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_arr(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i + 1;
    }
}

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern
   Should trigger the uncovered lines 136-150 in loop-doloop.cc */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - most likely to generate
       the PLUS with -1 compared to const0_rtx pattern */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant
   Should fail at cmp_arg2 != const0_rtx check */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1
   Should fail at XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
unsigned int pattern_c_decrement_by_two(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration */
    while (n > 0) {
        sum += arr[i & 0xFF];
        i++;
        n -= 2;  /* Not -1, so should fail validation */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement)
   May fail at GET_CODE(cmp_arg1) != PLUS check */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare operations */
    while (1) {
        sum += arr[i & 0xFF];
        i++;
        if (n == 0) break;
        n--;
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - also likely to match */
    while (n--) {
        sum += arr[i & 0xFF];
        i++;
    }
    
    return sum;
}

/* Pattern F: Exact match with different counter type */
unsigned int pattern_f_int_counter(void) {
    int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Using int instead of unsigned int */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to increase chances of pattern matching */
unsigned int pattern_g_nested_loops(void) {
    unsigned int outer = 10;
    unsigned int inner = 10;
    unsigned int sum = 0;
    
    while (outer--) {
        unsigned int temp_inner = inner;
        /* Inner loop with exact pattern */
        while (temp_inner--) {
            sum += arr[(outer + temp_inner) & 0xFF];
        }
    }
    
    return sum;
}

/* Pattern H: Loop with pointer but still decrement pattern */
unsigned int pattern_h_pointer_counter(void) {
    unsigned int n = 100;
    unsigned int *ptr = &n;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Using pointer dereference - might not match but worth trying */
    do {
        sum += arr[i & 0xFF];
        i++;
    } while ((*ptr)-- != 0);
    
    return sum;
}

int main(void) {
    volatile unsigned int results[8];
    
    /* Call all patterns to ensure they're compiled */
    results[0] = pattern_a_exact_match();
    results[1] = pattern_b_compare_nonzero();
    results[2] = pattern_c_decrement_by_two();
    results[3] = pattern_d_complex_compare();
    results[4] = pattern_e_while_exact_match();
    results[5] = pattern_f_int_counter();
    results[6] = pattern_g_nested_loops();
    results[7] = pattern_h_pointer_counter();
    
    /* Print results to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        printf("Result %d: %u\n", i, results[i]);
    }
    
    return 0;
}
