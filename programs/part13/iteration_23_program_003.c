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
    // 1. Large Integer Arithmetic with Overflow/Underflow
    // Use 128-bit arithmetic that requires double-int representation
    __int128 signed_128_max = ((__int128)LLONG_MAX << 32) | 0xFFFFFFFF;
    __int128 signed_128_min = ((__int128)LLONG_MIN << 32);
    unsigned __int128 unsigned_128_max = ((unsigned __int128)ULLONG_MAX << 64) | ULLONG_MAX;
    
    // Multiplication that exceeds 64-bit ranges
    __int128 x = (__int128)0x7FFFFFFFFFFFFFFFLL * 4;  // Should overflow signed 64-bit
    unsigned __int128 y = (unsigned __int128)0xFFFFFFFFFFFFFFFFULL * 3;  // Should overflow unsigned 64-bit
    
    accumulate((unsigned long long)x);
    accumulate((unsigned long long)(x >> 64));
    accumulate((unsigned long long)y);
    accumulate((unsigned long long)(y >> 64));
    
    // 2. Loop Bounds with Complex Exit Conditions
    // Loop with exit condition based on 128-bit arithmetic
    long long limit_base = 1000000;
    long long factor = 1000000;
    long long step = 7;
    
    // This loop bound calculation requires double-int analysis
    for (long long i = 0; i < (limit_base * factor); i += step) {
        // Inside loop, perform operations that may overflow
        __int128 product = (__int128)i * i;
        unsigned __int128 uproduct = (unsigned __int128)i * (unsigned __int128)i;
        
        // 3. Conditional Branches Based on Wide Comparisons
        // Mimic the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) logic
        if (product > signed_128_max) {
            accumulate(i);
        } else if (product == signed_128_max) {
            // Additional check that might trigger the second part of the condition
            if ((unsigned __int128)product > unsigned_128_max >> 1) {
                accumulate(i + 1);
            }
        }
        
        if (uproduct > unsigned_128_max / 2) {
            accumulate(i * 2);
        }
        
        // Prevent infinite loops in case of overflow
        if (i > 10000000) break;
    }
    
    // 4. Bit-Field Operations and Masking
    // Create values that require high/low part comparisons
    unsigned __int128 wide_val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned long long high_part = (unsigned long long)(wide_val >> 64);
    unsigned long long low_part = (unsigned long long)wide_val;
    
    // Extract bit-fields that span the 64-bit boundary
    for (int shift = 60; shift < 68; shift++) {
        unsigned __int128 extracted = (wide_val >> shift) & 0xFF;
        accumulate((unsigned long long)extracted);
        
        // Comparisons that might trigger the specific uncovered logic
        if (extracted > (unsigned __int128)0xFFFFFFFFFFFFFFFFULL) {
            accumulate(shift);
        }
    }
    
    // 5. Compiler Built-ins for Wide Arithmetic
    // Use overflow checking built-ins with 64-bit types
    long long a = LLONG_MAX / 2;
    long long b = 3;
    long long result;
    int overflow = __builtin_mul_overflow(a, b, &result);
    accumulate(overflow);
    
    // Use __int128 built-in operations
    __int128 div_result = signed_128_max / 2;
    accumulate((unsigned long long)div_result);
    
    // Additional test cases targeting boundary conditions
    // Near boundary values for signed comparisons
    __int128 near_max = signed_128_max - 1;
    __int128 at_max = signed_128_max;
    __int128 over_max = signed_128_max + 1;
    
    // These comparisons should exercise the double-int comparison logic
    if (near_max < at_max) accumulate(1);
    if (at_max == signed_128_max) accumulate(2);
    if (over_max > signed_128_max) accumulate(3);
    
    // Unsigned boundary tests
    unsigned __int128 near_umax = unsigned_128_max - 1;
    unsigned __int128 at_umax = unsigned_128_max;
    
    if (near_umax < at_umax) accumulate(4);
    if (at_umax == unsigned_128_max) accumulate(5);
    
    // Mixed signed/unsigned comparisons
    if ((unsigned __int128)near_max > unsigned_128_max / 2) accumulate(6);
    
    // Complex expression that might trigger the exact uncovered condition
    // a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    __int128 test_val = ((__int128)0x0 << 64) | 0x1;  // a_high = 0, a_low = 1
    __int128 max_r_val = 0;  // max_r = 0
    unsigned __int128 max_s_val = (unsigned __int128)(-1);  // max_s = -1 (all ones)
    
    // This should trigger the second part of the condition:
    // a_high == max_r (both 0) && a_low.ugt(max_s) (1 > all ones? false)
    // But with different values it could be true
    if (test_val > max_r_val) {
        accumulate(100);
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
