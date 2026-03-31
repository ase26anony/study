/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create inter-procedural data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function with loop where index may reach maximum value */
/* This targets a_high.sgt(max_r) path */
void test_high_part_gt_zero(int n) {
    /* Create a value that might have high part > 0 */
    for (long long i = 0; i < n; i++) {
        /* Complex expression that could overflow to high part > 0 */
        long long val = i * 0x7FFFFFFFFFFFFFFFLL;
        
        /* Use the value to prevent dead code elimination */
        if (val > 0x7FFFFFFFFFFFFFFFLL) {
            global_counter++;
        }
    }
}

/* Function targeting a_high == 0 && a_low.ugt(max_s) path */
/* Need a_low to be > max_s where max_s is large unsigned after zext */
void test_low_part_large_unsigned(void) {
    /* Create values with high part 0 but low part with specific bit patterns */
    unsigned long long mask = ~0ULL;
    
    /* Shift operations to create specific bit patterns */
    for (unsigned i = 0; i < 64; i++) {
        /* Create value where high part is 0 but low part has high bits set */
        unsigned long long val = mask >> i;
        
        /* Complex comparison that might trigger the specific condition */
        if (val > (mask >> 1)) {
            global_counter += val;
        }
    }
}

/* Function using __int128 to potentially trigger fixed-value analysis */
void test_int128_range(void) {
    __int128 large_val = ((__int128)1 << 120);
    __int128 result = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Operations that might create values with interesting ranges */
        result = large_val + i;
        
        /* Comparison near boundary */
        if (result > (((__int128)1 << 120) - 1)) {
            global_counter++;
        }
    }
}

/* Function with bitwise operations and shifts */
/* This creates values with specific bit patterns for range analysis */
void test_bit_patterns(unsigned width) {
    unsigned long long max_for_width = (1ULL << width) - 1;
    
    for (unsigned long long i = 0; i <= max_for_width; i++) {
        /* Create pattern where high bits might be zero but low bits large */
        unsigned long long pattern = (i << (64 - width)) | (max_for_width >> 1);
        
        /* Complex condition that might trigger the uncovered comparison */
        if ((pattern & max_for_width) > (max_for_width >> 1)) {
            global_counter += pattern;
        }
    }
}

/* Function with multiplication that can overflow */
void test_overflow_mul(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that could create high part > 0 */
        long long product = (long long)i * INT_MAX;
        
        /* Comparison that might be analyzed by range propagation */
        if (__builtin_expect(product > INT_MAX * 100LL, 0)) {
            global_counter += product;
        }
    }
}

/* Pure function to allow aggressive value propagation */
__attribute__((pure))
static unsigned long long generate_pattern(unsigned bits) {
    /* Create value with specific bit pattern based on bits */
    if (bits >= 64) return ~0ULL;
    
    unsigned long long mask = (1ULL << bits) - 1;
    /* Return value where high part is 0 but low part has high bit set */
    return mask & (1ULL << (bits - 1));
}

/* Function using the pure function with different bit widths */
void test_pure_function_range(void) {
    for (unsigned bits = 32; bits <= 64; bits++) {
        unsigned long long val = generate_pattern(bits);
        
        /* This comparison might trigger the specific condition when
           bits corresponds to i_f_bits in the uncovered code */
        if (val > ((1ULL << (bits - 1)) - 1)) {
            global_counter += val;
        }
    }
}

/* Main function that calls all test cases with various parameters */
int main(void) {
    /* Call test functions with different parameters to explore various ranges */
    
    /* Test with values that might create high part > 0 */
    test_high_part_gt_zero(100);
    test_high_part_gt_zero(1000);
    
    /* Test low part patterns */
    test_low_part_large_unsigned();
    
    /* Test with 128-bit integers */
    test_int128_range();
    
    /* Test with different bit widths */
    test_bit_patterns(32);
    test_bit_patterns(48);
    test_bit_patterns(56);
    test_bit_patterns(60);
    
    /* Test overflow scenarios */
    test_overflow_mul(100);
    test_overflow_mul(1000);
    
    /* Test with pure function */
    test_pure_function_range();
    
    /* Use the global counter to prevent dead code elimination */
    printf("Result: %lu\n", global_counter);
    
    return 0;
}
