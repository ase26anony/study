/* test_doloop.c - Test program for GCC loop doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_arr(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern
   Should trigger the validation logic and pass all checks */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - most likely to generate
       the exact RTL pattern: PLUS with -1, compared to const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    return sum;
}

/* Pattern B: Decrement and compare against non-zero constant
   Should fail at cmp_arg2 != const0_rtx check */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {
        sum += arr[i & 255];
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
        sum += arr[i & 255];
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
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Decrement as separate statement */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop
   Should also trigger the validation logic */
unsigned int pattern_e_while_decrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - may also generate the pattern */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern F: Loop with pre-decrement (may still match in some cases) */
unsigned int pattern_f_pre_decrement(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement version */
    while (--n) {
        sum += arr[i & 255];
        i++;
    }
    
    return sum;
}

/* Pattern G: Signed counter to test different RTL patterns */
int pattern_g_signed_counter(void) {
    int n = 100;
    int sum = 0;
    unsigned int i = 0;
    
    /* Signed counter with post-decrement */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- > 0);
    
    return sum;
}

/* Main function to call all patterns and ensure they're not optimized away */
int main(void) {
    volatile unsigned int result;  /* volatile to prevent optimization */
    
    printf("Testing doloop pattern coverage...\n");
    
    /* Call each pattern function */
    result = pattern_a_exact_match();
    printf("Pattern A result: %u\n", result);
    
    result = pattern_b_compare_nonzero();
    printf("Pattern B result: %u\n", result);
    
    result = pattern_c_decrement_by_two();
    printf("Pattern C result: %u\n", result);
    
    result = pattern_d_complex_compare();
    printf("Pattern D result: %u\n", result);
    
    result = pattern_e_while_decrement();
    printf("Pattern E result: %u\n", result);
    
    result = pattern_f_pre_decrement();
    printf("Pattern F result: %u\n", result);
    
    result = pattern_g_signed_counter();
    printf("Pattern G result: %d\n", result);
    
    return 0;
}
