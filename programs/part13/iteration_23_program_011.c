/* test_fixed_value.c
 * Designed to trigger double-int range comparison logic in GCC's fixed-value.cc
 * Specifically targets lines 264-277 in fixed-value.cc.gcov
 */

#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* Force compiler to consider various boundary conditions */
#define MAX_64 0x7FFFFFFFFFFFFFFFLL
#define MAX_U64 0xFFFFFFFFFFFFFFFFULL
#define MIN_64 0x8000000000000000LL

/* Volatile to prevent dead code elimination */
static volatile unsigned long long checksum = 0;

/* Helper to accumulate checksum without being optimized away */
static void accumulate(unsigned long long val) {
    checksum ^= val;
}

int main(void) {
    /* 1. Large integer arithmetic with overflow/underflow */
    /* Use __int128 operations that require double-int representation */
    __int128 a = (__int128)MAX_64 * 4;  /* Will overflow 64-bit signed */
    __int128 b = (__int128)MIN_64 * 2;  /* Underflow check */
    unsigned __int128 c = (unsigned __int128)MAX_U64 * 3;  /* Unsigned overflow */
    
    /* Force range analysis by storing intermediate results */
    long long a_high = (long long)(a >> 64);
    long long a_low = (long long)a;
    accumulate((unsigned long long)a_high);
    accumulate((unsigned long long)a_low);
    
    /* 2. Loop bounds with complex exit conditions */
    /* Loop where exit condition depends on wide calculations */
    for (long long i = 0; i < 100; ++i) {
        /* Create a limit that requires double-int analysis */
        __int128 limit = (__int128)i * MAX_64 / 100;
        
        /* Inner loop with comparison against wide integer */
        for (long long j = 0; j < 100; ++j) {
            __int128 product = (__int128)j * MAX_64;
            
            /* Conditional branch mimicking the uncovered logic:
             * if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
             */
            if (product > limit) {
                accumulate(j);
            }
            
            /* Additional comparison with unsigned 128-bit */
            unsigned __int128 uprod = (unsigned __int128)j * MAX_U64;
            if (uprod > (unsigned __int128)limit) {
                accumulate(j + 1000);
            }
        }
    }
    
    /* 3. Conditional branches based on wide comparisons */
    /* Chain of comparisons that should trigger the specific logic */
    __int128 x = (__int128)MAX_64 * MAX_64;  /* Very large value */
    __int128 y = (__int128)MIN_64 * MIN_64;  /* Very large positive (negative * negative) */
    
    /* Complex condition that requires both high and low part comparisons */
    if (x > y) {
        accumulate(1);
    }
    
    if (x == y) {
        accumulate(2);
    }
    
    /* Compare against boundary constants */
    __int128 boundary = ((__int128)0x7FFFFFFFFFFFFFFFLL << 32) | 0xFFFFFFFFLL;
    if (x > boundary) {
        accumulate(3);
    }
    
    if (x == boundary) {
        accumulate(4);
    }
    
    /* 4. Bit-field operations and masking */
    /* Create 128-bit value from separate high/low parts */
    unsigned __int128 combined = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                                 0xFEDCBA9876543210ULL;
    
    /* Extract bit-fields at various positions */
    for (int shift = 0; shift < 128; shift += 16) {
        unsigned __int128 field = (combined >> shift) & 0xFFFF;
        accumulate((unsigned long long)field);
        
        /* Compare extracted field */
        if (field > 0x8000) {
            accumulate(shift);
        }
    }
    
    /* 5. Compiler built-ins for wide arithmetic */
    /* Use overflow checking built-ins */
    long long ovf_a = MAX_64;
    long long ovf_b = 2;
    long long ovf_result;
    
    if (__builtin_mul_overflow(ovf_a, ovf_b, &ovf_result)) {
        accumulate(5);  /* Overflow occurred */
    }
    
    /* 128-bit built-in operations */
    int bits = __builtin_clzll(MAX_U64);
    accumulate(bits);
    
    /* Additional stress test with varying shift amounts */
    for (int shift = 1; shift <= 64; shift *= 2) {
        unsigned long long shifted = MAX_U64 >> shift;
        unsigned long long shifted2 = MAX_U64 << shift;
        
        /* Comparisons that might trigger range analysis */
        if (shifted > 0xFFFFFFFFULL) {
            accumulate(shift);
        }
        
        if (shifted2 < MAX_U64) {
            accumulate(shift + 100);
        }
    }
    
    /* Final output to prevent entire program from being optimized away */
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum & 0xFF);
}
