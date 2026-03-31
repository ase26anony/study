/* test_doloop.c - Generate RTL patterns for GCC's loop doloop pass validation */

#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent loop elimination */
volatile int arr[256] = {0};
volatile int result = 0;

/* Pattern A: Exact match for decrement-and-compare-to-zero */
unsigned int test_exact_pattern(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Initialize array with some values */
    for (int j = 0; j < 256; j++) {
        arr[j] = j % 64;
    }
    
    /* This should generate: PLUS with -1, COMPARE with const0_rtx */
    do {
        sum += arr[i++ % 256];
    } while (n-- != 0);  /* Post-decrement compare to zero */
    
    return sum;
}

/* Pattern B: Decrement but compare against non-zero constant */
unsigned int test_compare_nonzero(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail cmp_arg2 != const0_rtx check */
    while (--n > 5) {  /* Compare against 5, not zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern C: Decrement by value other than 1 */
unsigned int test_decrement_by_two(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should fail XEXP(cmp_arg1, 1) != GEN_INT(-1) check */
    while (n > 0) {
        sum += arr[i++ % 256];
        n -= 2;  /* Decrement by 2, not 1 */
    }
    
    return sum;
}

/* Pattern D: Complex compare source (separate decrement) */
unsigned int test_complex_compare(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This may fail GET_CODE(cmp_arg1) != PLUS check */
    /* Separate decrement and compare operations */
    while (n) {
        sum += arr[i++ % 256];
        n--;  /* Decrement in body, not in compare */
    }
    
    return sum;
}

/* Pattern E: Another exact pattern with while loop */
unsigned int test_exact_while(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* This should also match the pattern */
    while (n--) {  /* Post-decrement to zero */
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern F: Counter in register with pre-decrement */
unsigned int test_predecrement(unsigned int count) {
    unsigned int n = count;
    unsigned int sum = 0;
    unsigned int i = 0;
    
    /* Pre-decrement might still generate PLUS with -1 */
    while (--n != 0) {
        sum += arr[i++ % 256];
    }
    
    return sum;
}

/* Pattern G: Signed counter to test different RTL */
int test_signed_counter(int count) {
    int n = count;
    int sum = 0;
    int i = 0;
    
    /* Signed counter might generate different compare */
    do {
        sum += arr[i++ % 256];
    } while (n-- > 0);
    
    return sum;
}

/* Main function to ensure all tests are called */
int main() {
    unsigned int total = 0;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        arr[i] = (i * 3) % 127;
    }
    
    /* Call all test functions with moderate loop counts */
    total += test_exact_pattern(1000);
    total += test_compare_nonzero(1000);
    total += test_decrement_by_two(1000);
    total += test_complex_compare(1000);
    total += test_exact_while(1000);
    total += test_predecrement(1000);
    total += test_signed_counter(1000);
    
    /* Store result to prevent elimination */
    result = total;
    
    printf("Total: %u\n", total);
    return 0;
}
