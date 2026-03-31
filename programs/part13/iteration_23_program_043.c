#include <stdio.h>
#include <limits.h>
#include <stdint.h>

// Volatile to prevent optimization
static volatile unsigned long long checksum = 0;

// Helper to mix bits and prevent dead code elimination
static void mix(unsigned long long val) {
    checksum ^= val;
}

int main() {
    // 1. Large Integer Arithmetic with Overflow/Underflow
    // Use __int128 operations that require double-int representation
    __int128 large_signed = (__int128)LLONG_MAX * 4;
    unsigned __int128 large_unsigned = (unsigned __int128)ULLONG_MAX * 3ULL;
    
    mix((unsigned long long)large_signed);
    mix((unsigned long long)(large_signed >> 64));
    mix((unsigned long long)large_unsigned);
    mix((unsigned long long)(large_unsigned >> 64));
    
    // 2. Loop Bounds with Complex Exit Conditions
    // Loop with bounds near 64-bit limits
    long long start = LLONG_MAX - 1000;
    long long limit = LLONG_MAX - 500;
    long long factor = 2;
    
    for (long long i = start; i < (limit * factor) / 3; i += 1) {
        // Complex exit condition requiring range analysis
        if (i > LLONG_MAX / 2) {
            mix((unsigned long long)i);
            break;
        }
    }
    
    // 3. Conditional Branches Based on Wide Comparisons
    // Direct comparisons mimicking the uncovered logic
    __int128 a = (__int128)0x7FFFFFFFFFFFFFFFLL * 4;
    __int128 b = (__int128)0x8000000000000000LL * 2;
    
    // Chain of comparisons that should trigger double-int logic
    if (a > b) {
        mix(0x1);
    }
    
    if (a == b) {
        mix(0x2);
    }
    
    // More complex condition similar to the uncovered code
    unsigned __int128 max_val = (unsigned __int128)ULLONG_MAX << 32;
    unsigned __int128 test_val = (unsigned __int128)0xFFFFFFFFFFFFFFFFULL * 5;
    
    if (test_val > max_val) {
        mix(0x4);
    }
    
    // 4. Bit-Field Operations and Masking
    // Create values that require high/low part comparisons
    unsigned __int128 mask = ((unsigned __int128)0xFFFF0000FFFF0000ULL << 64) | 0x0000FFFF0000FFFFULL;
    unsigned __int128 val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    unsigned __int128 masked = val & mask;
    unsigned __int128 shifted = val >> 96;
    
    // Comparisons that might trigger the specific uncovered condition
    if ((masked >> 64) > 0x7FFFFFFFFFFFFFFFULL) {
        mix(0x8);
    }
    
    if (((masked >> 64) == 0x7FFFFFFFFFFFFFFFULL) && 
        ((masked & 0xFFFFFFFFFFFFFFFFULL) > 0xFFFFFFFFFFFFFFFEULL)) {
        mix(0x10);
    }
    
    // 5. Compiler Built-ins for Wide Arithmetic
    // Use overflow checking built-ins
    long long x = LLONG_MAX;
    long long y = 2;
    long long result;
    
    if (__builtin_mul_overflow(x, y, &result)) {
        mix(0x20);  // Overflow occurred
    }
    
    // Test with __int128
    __int128 x128 = (__int128)LLONG_MAX * 2;
    __int128 y128 = (__int128)LLONG_MAX * 2;
    __int128 sum128 = x128 + y128;
    
    if (sum128 > (__int128)LLONG_MAX * 4) {
        mix(0x40);
    }
    
    // Additional boundary tests
    // Test near maximum signed 128-bit value
    __int128 near_max = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 increment = 1;
    
    for (int j = 0; j < 10; j++) {
        near_max += increment;
        // This comparison should exercise the high/low part logic
        if (near_max > (((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFEULL)) {
            mix(0x80 + j);
        }
    }
    
    // Test with shifting operations
    unsigned long long shift_amount = 60;
    unsigned __int128 shifted_val = (unsigned __int128)1 << shift_amount;
    shifted_val = shifted_val << 4;  // Push to 128-bit boundary
    
    if (shifted_val > ((unsigned __int128)1 << 63)) {
        mix(0x100);
    }
    
    // Final output to prevent optimization
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
