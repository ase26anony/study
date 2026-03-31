/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function that creates values with high part = 0, low part with specific patterns */
__attribute__((noinline))
unsigned long long create_large_low_part(int shift_bits) {
    /* Create value where high part is 0, low part has specific bit pattern */
    unsigned long long mask = (1ULL << shift_bits) - 1;
    return mask; /* All low bits set, high bits 0 */
}

/* Function that might trigger a_high.sgt(max_r) */
__attribute__((noinline))
int test_high_part_greater(int iterations) {
    int sum = 0;
    
    /* Loop with induction variable that compiler can analyze */
    for (int i = 0; i < iterations; i++) {
        /* Create value that might have non-zero high part in fixed-point representation */
        long long val = (long long)i * 0x100000000LL;
        
        /* Comparison that forces range analysis */
        if (val > 0x7FFFFFFFFFFFFFFFLL) {
            sum += 1;
        }
        
        /* Complex arithmetic to create value ranges */
        val = val + (i * 0x1000);
        if (val < 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function targeting a_high == max_r && a_low.ugt(max_s) */
__attribute__((noinline))
int test_low_part_exceeds_mask(int f_bits) {
    int result = 0;
    
    /* Create a value where high part is 0, low part exceeds specific mask */
    unsigned long long max_mask = (1ULL << f_bits) - 1;
    
    /* Value just above the mask - when analyzed, high part = 0, low part > max_s */
    unsigned long long test_value = max_mask + 1;
    
    /* Multiple comparisons to force analysis */
    if (test_value > max_mask) {
        result = 1;
    }
    
    /* Use in loop to create more complex data flow */
    for (unsigned i = 0; i < 10; i++) {
        unsigned long long val = test_value + i;
        
        /* This comparison should trigger the specific condition:
           a_high == 0 && a_low > (all low bits set to f_bits width) */
        if (val > max_mask) {
            result += (int)val;
        }
    }
    
    return result;
}

/* Function with bitwise operations that create specific patterns */
__attribute__((noinline))
int test_bitwise_patterns(void) {
    int result = 0;
    
    /* Create values with specific bit patterns */
    for (unsigned i = 0; i < 256; i++) {
        /* Pattern: high bits 0, low bits with specific count of 1s */
        unsigned long long val = i;
        
        /* Shift to create different alignments */
        val = (val << 32) | (val << 16) | val;
        
        /* Mask to clear high bits */
        val &= 0x0000FFFFFFFFFFFFULL;
        
        /* Comparison that might trigger the condition */
        if (val > 0x0000FFFFFFFFFFFULL) {
            result++;
        }
    }
    
    return result;
}

/* Function using 128-bit integers (might use fixed-value internally) */
#ifdef __SIZEOF_INT128__
__attribute__((noinline))
int test_128bit_operations(void) {
    unsigned __int128 large_val = 0;
    int result = 0;
    
    /* Create a 128-bit value with specific pattern */
    large_val = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* Shift right to create pattern with high part = 0 */
    large_val >>= 96;  /* Now high 32 bits are 0, low 96 bits are 1s */
    
    /* Compare against mask - might trigger the condition */
    unsigned __int128 mask = ((unsigned __int128)1 << 95) - 1;
    
    if (large_val > mask) {
        result = 1;
    }
    
    return result;
}
#endif

/* Function with multiplication that can overflow */
__attribute__((noinline))
int test_multiplication_overflow(int n) {
    int result = 0;
    
    /* Multiplication that might create values needing range analysis */
    for (int i = 0; i < n; i++) {
        long long prod = (long long)i * 0x100000001LL;
        
        /* Comparisons at boundaries */
        if (prod > 0x7FFFFFFFFFFFFFFFLL) {
            result++;
        } else if (prod < -0x7FFFFFFFFFFFFFFFLL) {
            result--;
        }
    }
    
    return result;
}

/* Main function that exercises all test cases */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Prevent constant propagation from compiler knowing all values */
    int iter_count = volatile_var + 100;
    int f_bits = volatile_var + 31;  /* Common fixed-point fractional bits */
    
    /* Test various patterns */
    total += test_high_part_greater(iter_count);
    total += test_low_part_exceeds_mask(f_bits);
    total += test_bitwise_patterns();
    total += test_multiplication_overflow(50);
    
#ifdef __SIZEOF_INT128__
    total += test_128bit_operations();
#endif
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000) {
        printf("Result: %d\n", total);
    }
    
    return 0;
}
