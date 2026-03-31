/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

/* Global array to prevent loop removal */
static int arr[256] = {[0 ... 255] = 1};

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int pattern_a_exact_match(unsigned int iterations) {
    unsigned int n = iterations;
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
unsigned int pattern_b_compare_nonzero(unsigned int iterations) {
    unsigned int n = iterations;
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
unsigned int pattern_c_decrement_by_two(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Decrement by 2 each iteration - should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    while (n > 0) {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Not decrement by 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int pattern_d_complex_compare(unsigned int iterations) {
    unsigned int n = iterations;
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

/* Pattern E: Another exact match variant with while loop */
unsigned int pattern_e_while_postdec(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also match */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Counter in register but with different type */
unsigned int pattern_f_int_counter(unsigned int iterations) {
    int n = (int)iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* int counter instead of unsigned - should still work */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Pattern G: Nested loops to test multiple contexts */
unsigned int pattern_g_nested_loops(unsigned int outer_iter, unsigned int inner_iter) {
    unsigned int outer = outer_iter;
    unsigned int inner;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        inner = inner_iter;
        /* Inner loop should also be considered */
        do {
            sum += arr[i & 255];
            i++;
        } while (inner-- != 0);
        
    } while (outer-- != 0);
    
    return sum;
}

/* Pattern H: Loop with early exit - might affect pattern recognition */
unsigned int pattern_h_early_exit(unsigned int iterations) {
    unsigned int n = iterations;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i & 255];
        i++;
        if (sum > 1000) break;  /* Early exit */
    } while (n-- != 0);
    
    return sum;
}

/* Main function to call all patterns */
int main(void) {
    volatile unsigned int result;  /* volatile to prevent optimization */
    
    /* Call each pattern with different iteration counts */
    result = pattern_a_exact_match(1000);
    result += pattern_b_compare_nonzero(1000);
    result += pattern_c_decrement_by_two(1000);
    result += pattern_d_complex_compare(1000);
    result += pattern_e_while_postdec(1000);
    result += pattern_f_int_counter(1000);
    result += pattern_g_nested_loops(10, 100);
    result += pattern_h_early_exit(1000);
    
    return (int)(result > 0);
}
