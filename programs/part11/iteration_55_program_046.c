/* loop-doloop-test.c
 * Test program to cover GCC's loop doloop validation logic
 * Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops loop-doloop-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop removal */
static int arr[256] = {0};

/* Initialize array with some values */
__attribute__((constructor))
static void init_array(void) {
    for (int i = 0; i < 256; i++) {
        arr[i] = i % 64;
    }
}

/* Pattern A: Exact match for decrement-and-compare-to-zero pattern
 * Should trigger the validation logic and potentially pass all checks
 */
unsigned int pattern_a_exact_match(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* do-while with post-decrement to zero - most likely to generate
     * the exact RTL pattern: PLUS with -1, COMPARE with const0_rtx
     */
    do {
        sum += arr[i++ % 256];
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant
 * Should fail the cmp_arg2 != const0_rtx check
 */
unsigned int pattern_b_compare_nonzero(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Compare against 5 instead of 0 */
    while (--n > 5) {  /* Pre-decrement compare to non-zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1
 * Should fail the XEXP(cmp_arg1, 1) != GEN_INT(-1) check
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

/* Pattern D: Complex compare source (separate decrement)
 * May fail the GET_CODE(cmp_arg1) != PLUS check
 */
unsigned int pattern_d_complex_compare(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Separate decrement and compare operations */
    while (1) {
        sum += arr[i++ % 256];
        n--;
        if (n == 0) break;  /* Separate compare operation */
    }
    
    return sum;
}

/* Pattern E: Another exact match variant with while loop
 * Different syntax but same underlying pattern
 */
unsigned int pattern_e_while_exact(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* while with post-decrement - should also generate the pattern */
    while (n--) {  /* Post-decrement to zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern F: Signed counter with decrement
 * Tests if signedness affects the pattern generation
 */
int pattern_f_signed_counter(void) {
    int n = 100;
    int sum = 0;
    unsigned int i = 0;
    
    do {
        sum += arr[i++ % 256];
    } while (n-- > 0);  /* Signed post-decrement compare to zero */
    
    return sum;
}

/* Pattern G: Loop with multiple exit conditions
 * Less likely to match the simple pattern
 */
unsigned int pattern_g_complex_exit(void) {
    unsigned int n = 100;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Complex condition that might not optimize to simple decrement-and-compare */
    while (n && i < 50) {
        sum += arr[i++ % 256];
        n--;
    }
    
    return sum;
}

/* Main function to call all patterns and ensure they're not optimized away */
int main(void) {
    volatile unsigned int result;  /* volatile to prevent optimization */
    
    printf("Testing loop doloop patterns...\n");
    
    /* Call each pattern function */
    result = pattern_a_exact_match();
    printf("Pattern A result: %u\n", result);
    
    result = pattern_b_compare_nonzero();
    printf("Pattern B result: %u\n", result);
    
    result = pattern_c_decrement_by_two();
    printf("Pattern C result: %u\n", result);
    
    result = pattern_d_complex_compare();
    printf("Pattern D result: %u\n", result);
    
    result = pattern_e_while_exact();
    printf("Pattern E result: %u\n", result);
    
    result = pattern_f_signed_counter();
    printf("Pattern F result: %d\n", result);
    
    result = pattern_g_complex_exit();
    printf("Pattern G result: %u\n", result);
    
    return 0;
}
