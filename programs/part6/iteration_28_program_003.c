/* test_fixed_value.c - Test program to trigger fixed-value range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static __int128 large_values[4] = {0};

/* Function with loop that may trigger a_high.sgt(max_r) */
__attribute__((noinline))
int test_high_part_comparison(int n) {
    /* Create a value that might have non-zero high part in fixed-point analysis */
    int result = 0;
    
    /* Loop with induction variable that could overflow analysis */
    for (int i = 0; i < n; i++) {
        /* Multiplication that could create large values */
        int val = i * 1073741824;  /* 2^30 - could overflow 32-bit */
        
        /* Complex condition that requires range analysis */
        if (val > 0 && __builtin_expect(val < 1000000000, 1)) {
            result += val;
        }
        
        /* Bit manipulation that might affect fixed-point analysis */
        unsigned int bits = (unsigned int)val;
        bits = (bits >> 16) | (bits << 16);  /* Byte swap */
        
        if (bits > 0x7FFFFFFF) {
            result -= 1;
        }
    }
    
    return result;
}

/* Function targeting a_high == 0 && a_low > max_s case */
__attribute__((noinline, const))
unsigned long long test_low_part_overflow(unsigned bits) {
    /* Create values where high part is 0 but low part has specific patterns */
    unsigned long long value = 0;
    
    /* Start with a value that has specific bit pattern */
    value = (1ULL << (bits - 1)) | ((1ULL << (bits - 1)) - 1);
    
    /* Operations that keep high part 0 but manipulate low part */
    value = value & ((1ULL << bits) - 1);
    
    /* This should create a value where high part is 0 in fixed-point analysis
       but low part exceeds the max_s threshold after zero-extension */
    value = value + (bits > 32 ? 0xFFFFFFFF : 0);
    
    return value;
}

/* Function using 128-bit integers which likely use fixed-value internally */
__attribute__((noinline))
__int128 test_128bit_range(int iterations) {
    __int128 accumulator = 0;
    __int128 multiplier = 0x123456789ABCDEF;
    
    for (int i = 0; i < iterations; i++) {
        /* Create values that might trigger the specific comparison */
        accumulator = accumulator * multiplier + i;
        
        /* Mask to keep within certain bounds but with complex pattern */
        accumulator = accumulator & (((__int128)0xFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF);
        
        /* Store for inter-procedural analysis */
        if (i < 4) {
            large_values[i] = accumulator;
        }
    }
    
    return accumulator;
}

/* Function with shifting operations that affect fixed-point analysis */
__attribute__((noinline))
unsigned long test_shift_patterns(unsigned shift) {
    unsigned long value = 0x80000000UL;
    
    /* Create patterns that might be analyzed as fixed-point */
    for (unsigned i = 0; i < shift; i++) {
        value = (value << 1) | (value >> 31);
        
        /* Conditional that depends on bit patterns */
        if ((value & 0x80000000) && (value & 0x7FFFFFFF) > 0x3FFFFFFF) {
            global_counter++;
        }
    }
    
    /* Return value that might have specific range properties */
    return value & ((1UL << shift) - 1);
}

/* Function that creates boundary conditions */
__attribute__((noinline))
int test_boundary_conditions(int limit) {
    int sum = 0;
    
    /* Loop that approaches but doesn't exceed a boundary */
    for (int i = 0; i < limit; i++) {
        /* Create value near 2^31 boundary */
        int val = i + 0x7FFFFFF0;
        
        /* This comparison might trigger the uncovered logic when
           analyzing the range of 'val' */
        if (val > 0x7FFFFFFF) {
            sum += 1;
        } else if (val == 0x7FFFFFFF) {
            sum += 2;
        }
        
        /* Additional arithmetic to create complex value ranges */
        val = (val * 3) / 2;
        
        if (val < 0) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Test 1: High part comparison */
    result += test_high_part_comparison(argc > 1 ? 100 : 50);
    
    /* Test 2: Low part overflow case - targeting a_high == 0 && a_low > max_s */
    for (unsigned bits = 32; bits <= 48; bits += 8) {
        unsigned long long val = test_low_part_overflow(bits);
        result += (int)(val & 0xFFFFFFFF);
    }
    
    /* Test 3: 128-bit operations */
    __int128 big_val = test_128bit_range(argc > 2 ? 10 : 5);
    result += (int)(big_val & 0x7FFFFFFF);
    
    /* Test 4: Shift patterns */
    for (unsigned shift = 16; shift <= 28; shift += 4) {
        unsigned long shift_val = test_shift_patterns(shift);
        result += (int)(shift_val & 0xFFFF);
    }
    
    /* Test 5: Boundary conditions */
    result += test_boundary_conditions(argc > 3 ? 40 : 20);
    
    /* Use the global counter to prevent dead code elimination */
    result += (int)global_counter;
    
    /* Use large_values to prevent optimization */
    for (int i = 0; i < 4; i++) {
        result += (int)(large_values[i] & 0xFF);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
