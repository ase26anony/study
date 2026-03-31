/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function with loop where index may reach maximum value */
__attribute__((noinline))
void test_high_part_greater_than_max(int n) {
    /* This should trigger a_high.sgt(max_r) when n is positive */
    for (int i = 0; i < n; i++) {
        /* Complex arithmetic that might create fixed-point ranges */
        int val = i * 2 + 1;
        if (val > 1000) {
            global_counter += val;
        }
    }
}

/* Function targeting a_high == 0 && a_low > max_s */
__attribute__((noinline))
void test_zero_high_large_low(unsigned int bits) {
    /* Create value with high part 0 but low part potentially large */
    unsigned long long mask = (1ULL << bits) - 1;
    
    /* This creates a value where high part is 0, low part is mask */
    unsigned long long value = mask;
    
    /* Operations that might trigger the specific comparison */
    if (value > (mask >> 1)) {
        global_counter += value;
    }
    
    /* Try to create value just above the mask boundary */
    if (bits < 63) {
        unsigned long long overflow_val = mask + 1;
        if (overflow_val > mask) {
            global_counter += 1;
        }
    }
}

/* Function with bitwise operations and shifts */
__attribute__((noinline))
void test_bitwise_range(unsigned int x) {
    /* Create patterns that might trigger fixed-value analysis */
    unsigned int shifted = x << 16;
    unsigned int masked = shifted & 0xFFFF0000;
    
    /* Comparison near boundary */
    if (masked == 0xFFFF0000) {
        global_counter += 1;
    }
    
    /* Create value with specific bit pattern */
    unsigned int pattern = 0x80000000;
    if (x > 0) {
        pattern = pattern >> (x % 32);
    }
    
    if (pattern > 0x7FFFFFFF) {
        global_counter += pattern;
    }
}

/* Function using 128-bit integers (might use different fixed-value logic) */
#ifdef __SIZEOF_INT128__
__attribute__((noinline))
void test_128bit_range(unsigned long long n) {
    __int128 large_val = (__int128)n * n;
    
    /* Create comparisons that might trigger range analysis */
    if (large_val > (__int128)1 << 60) {
        global_counter += (unsigned long long)large_val;
    }
    
    /* Test with negative values */
    __int128 neg_val = -large_val;
    if (neg_val < -1000) {
        global_counter += 1;
    }
}
#endif

/* Function with complex loop bounds */
__attribute__((noinline))
void test_complex_loop_bounds(int start, int end, int step) {
    /* Loop with variable bounds - compiler needs to analyze range */
    for (int i = start; i < end; i += step) {
        /* Multiplication that could overflow */
        int prod = i * step;
        
        /* Comparison that might trigger the uncovered logic */
        if (prod > 1000000) {
            global_counter += prod;
        }
        
        /* Bit manipulation */
        unsigned int bits = (unsigned int)prod;
        if ((bits & 0x80000000) != 0) {
            global_counter += 1;
        }
    }
}

/* Function that creates values near type boundaries */
__attribute__((noinline))
void test_boundary_values(unsigned int type_bits) {
    unsigned long long max_for_bits = (1ULL << type_bits) - 1;
    unsigned long long near_max = max_for_bits - 1;
    
    /* These comparisons should trigger range analysis */
    if (near_max > max_for_bits / 2) {
        global_counter += near_max;
    }
    
    /* Create value that might have high part 0, low part > max_s */
    unsigned long long high_zero_val = max_for_bits;
    if (high_zero_val == max_for_bits) {
        global_counter += 1;
    }
    
    /* Test with exactly the boundary value */
    if (max_for_bits > 0) {
        global_counter += max_for_bits;
    }
}

/* Main function that calls all test cases with various inputs */
int main(int argc, char *argv[]) {
    /* Test different bit widths to trigger different i_f_bits values */
    unsigned int test_bits[] = {8, 16, 32, 48, 63};
    
    /* Test case 1: Trigger a_high.sgt(max_r) */
    printf("Testing high part greater than max...\n");
    for (int i = 0; i < 5; i++) {
        test_high_part_greater_than_max(1000 + i * 500);
    }
    
    /* Test case 2: Trigger a_high == 0 && a_low > max_s */
    printf("Testing zero high with large low part...\n");
    for (int i = 0; i < sizeof(test_bits)/sizeof(test_bits[0]); i++) {
        test_zero_high_large_low(test_bits[i]);
    }
    
    /* Test case 3: Bitwise operations */
    printf("Testing bitwise range analysis...\n");
    for (unsigned int i = 0; i < 100; i += 7) {
        test_bitwise_range(i);
    }
    
    /* Test case 4: Complex loop bounds */
    printf("Testing complex loop bounds...\n");
    test_complex_loop_bounds(-100, 100, 3);
    test_complex_loop_bounds(0, 1000, 7);
    test_complex_loop_bounds(100, 10000, 13);
    
    /* Test case 5: Boundary values */
    printf("Testing boundary values...\n");
    for (int i = 0; i < sizeof(test_bits)/sizeof(test_bits[0]); i++) {
        test_boundary_values(test_bits[i]);
    }
    
#ifdef __SIZEOF_INT128__
    /* Test case 6: 128-bit integers if supported */
    printf("Testing 128-bit ranges...\n");
    for (unsigned long long i = 1; i < 100; i *= 2) {
        test_128bit_range(i);
    }
#endif
    
    /* Use volatile to prevent dead code elimination */
    volatile_var = global_counter > 0;
    
    printf("Test completed. Global counter: %lu\n", global_counter);
    return 0;
}
