#include <stdio.h>
#include <limits.h>
#include <stdint.h>

// Force compiler to perform range analysis on wide integers
volatile unsigned long long checksum = 0;

// Helper to prevent optimization
static void accumulate(unsigned long long val) {
    checksum ^= val;
}

int main() {
    // 1. Large integer arithmetic with overflow/underflow
    __int128 signed_128_max = ((__int128)LLONG_MAX << 32) | 0xFFFFFFFF;
    unsigned __int128 unsigned_128_max = ((unsigned __int128)ULLONG_MAX << 64) | ULLONG_MAX;
    
    // Multiplication that exceeds 64-bit ranges
    __int128 x = (__int128)0x7FFFFFFFFFFFFFFFLL * 4;
    accumulate((unsigned long long)(x >> 64));
    accumulate((unsigned long long)x);
    
    // Shift operations on maximum values
    unsigned long long y = ~0ULL >> 3;
    unsigned long long z = ~0ULL << 2;
    accumulate(y);
    accumulate(z);
    
    // 2. Loop bounds with complex exit conditions
    long long limit = LLONG_MAX / 4;
    long long factor = 5;
    long long step = LLONG_MAX / 8;
    
    // Loop with exit condition requiring double-int comparison
    for (long long i = 0; i < limit * factor; i += step) {
        // Nested loop with different type
        for (int j = 0; j < 10; ++j) {
            __int128 product = (__int128)i * j;
            accumulate((unsigned long long)product);
            
            // Conditional based on wide comparison
            if (product > signed_128_max / 2) {
                accumulate(j);
            }
        }
        
        // Break early to avoid infinite loop in test
        if (i > limit) break;
    }
    
    // 3. Conditional branches based on wide comparisons
    unsigned __int128 a = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 32);
    unsigned __int128 b = 0x123456789ABCDEF0ULL;
    
    // Chain comparisons mimicking a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (a > unsigned_128_max / 2) {
        accumulate(1);
    }
    
    if ((unsigned __int128)a * b > unsigned_128_max) {
        accumulate(2);
    }
    
    // Explicit high/low part comparisons
    unsigned long long a_high = (unsigned long long)(a >> 64);
    unsigned long long a_low = (unsigned long long)a;
    unsigned long long max_r = ULLONG_MAX / 2;
    unsigned long long max_s = ULLONG_MAX;
    
    if (a_high > max_r || (a_high == max_r && a_low > max_s)) {
        accumulate(3);
    }
    
    // 4. Bit-field operations and masking
    unsigned __int128 val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int shift = 60;
    unsigned long long mask = 0xFFFFFFFFULL;
    
    for (int i = 0; i < 4; ++i) {
        unsigned long long field = (unsigned long long)((val >> (shift + i * 8)) & mask);
        accumulate(field);
        
        // Comparison that may trigger range analysis
        if (field > mask / 2) {
            accumulate(i + 100);
        }
    }
    
    // 5. Compiler built-ins for wide arithmetic
    long long op1 = LLONG_MAX;
    long long op2 = 2;
    long long result;
    
    // Overflow checking built-ins
    if (__builtin_mul_overflow(op1, op2, &result)) {
        accumulate(4);  // Should trigger on overflow
    }
    
    // Count leading zeros on computed values
    unsigned long long computed = (unsigned long long)(val >> 64) | (unsigned long long)val;
    int clz = __builtin_clzll(computed);
    accumulate(clz);
    
    // 128-bit division forcing range analysis
    __int128 dividend = (__int128)LLONG_MIN * 2;
    __int128 divisor = LLONG_MAX / 2;
    if (divisor != 0) {
        __int128 quotient = dividend / divisor;
        accumulate((unsigned long long)quotient);
        
        // Conditional with complex comparison
        if (quotient > 0 && quotient < LLONG_MAX) {
            accumulate(5);
        }
    }
    
    // Additional test: mixed signed/unsigned comparisons
    unsigned __int128 uval = ~(unsigned __int128)0;
    __int128 sval = (__int128)LLONG_MAX << 10;
    
    if ((__int128)uval > sval) {
        accumulate(6);
    }
    
    // Test with compile-time constants
    static const __int128 const_large = ((__int128)0x7FFFFFFFFFFFFFFFLL << 32) | 0xFFFFFFFF;
    static const __int128 const_small = -((__int128)0x7FFFFFFFFFFFFFFFLL << 32);
    
    if (const_large > const_small) {
        accumulate(7);
    }
    
    // Final output to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
