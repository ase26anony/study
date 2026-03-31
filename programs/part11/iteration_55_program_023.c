/* Test program for GCC loop doloop optimization pass coverage */
/* Compile with: gcc -O2 -fdump-rtl-doloop -fno-unroll-loops -c test.c */

#include <stdint.h>

/* Global array to prevent loop elimination */
static int arr[256] = {[0 ... 255] = 1};

/* Volatile sink to prevent optimization */
volatile int sink;

/* Pattern A: Exact match - decrement by 1, compare to zero */
unsigned int test_exact_match(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    sink = sum;
    return sum;
}

/* Pattern B: Compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This should fail: cmp_arg2 != const0_rtx */
    while (--n > 5) {
        sum += arr[i & 255];
        i++;
    }
    
    sink = sum;
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This should fail: XEXP(cmp_arg1, 1) != GEN_INT(-1) */
    while (n) {
        sum += arr[i & 255];
        i++;
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    sink = sum;
    return sum;
}

/* Pattern D: Complex compare source */
unsigned int test_complex_compare(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This may fail: GET_CODE(cmp_arg1) != PLUS
       due to separate decrement and compare */
    while (n) {
        sum += arr[i & 255];
        i++;
        n--;  /* Separate decrement */
        if (n == 0) break;  /* Complex control flow */
    }
    
    sink = sum;
    return sum;
}

/* Pattern E: Another exact match variant */
unsigned int test_while_postdecrement(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This should also match: while (n--) pattern */
    while (n--) {
        sum += arr[i & 255];
        i++;
    }
    
    sink = sum;
    return sum;
}

/* Pattern F: Pre-decrement compare to zero */
unsigned int test_predecrement_zero(unsigned int count) {
    unsigned int i = 0;
    unsigned int n = count;
    unsigned int sum = 0;
    
    /* This might match depending on RTL generation */
    while (--n) {
        sum += arr[i & 255];
        i++;
    }
    
    sink = sum;
    return sum;
}

/* Pattern G: Signed counter (should still work) */
int test_signed_counter(int count) {
    int i = 0;
    int n = count;
    int sum = 0;
    
    /* Signed version of exact match */
    do {
        sum += arr[i & 255];
        i++;
    } while (n-- != 0);
    
    sink = sum;
    return sum;
}

/* Main function to call all test patterns */
int main(void) {
    unsigned int count = 1000;
    unsigned int total = 0;
    
    total += test_exact_match(count);
    total += test_compare_nonzero(count);
    total += test_decrement_by_two(count);
    total += test_complex_compare(count);
    total += test_while_postdecrement(count);
    total += test_predecrement_zero(count);
    total += test_signed_counter((int)count);
    
    sink = total;
    return total > 0 ? 0 : 1;
}
