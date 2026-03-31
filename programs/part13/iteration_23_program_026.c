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
    // Use __int128 operations that require double-int representation
    __int128 large_signed = (__int128)LLONG_MAX * 4;
    unsigned __int128 large_unsigned = (unsigned __int128)ULLONG_MAX * 3ULL;
    
    // These operations should trigger double-int range analysis
    __int128 product1 = large_signed * 2;
    unsigned __int128 product2 = large_unsigned * 2ULL;
    
    // 2. Loop bounds with complex exit conditions
    // Loop that depends on 128-bit comparisons
    long long outer_limit = 100;
    for (long long i = 0; i < outer_limit; i++) {
        // Create a loop bound that depends on wide integer comparison
        __int128 dynamic_bound = (__int128)LLONG_MAX - (__int128)i * 1000000LL;
        
        // Inner loop with condition that requires range analysis
        for (long long j = 0; (__int128)j < dynamic_bound / 1000; j++) {
            // Perform arithmetic that might overflow
            __int128 temp = (__int128)i * (__int128)j;
            
            // 3. Conditional branches based on wide comparisons
            // This mimics the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) logic
            if (temp > (__int128)LLONG_MAX) {
                accumulate((unsigned long long)(temp >> 64));
            } else if (temp == (__int128)LLONG_MAX) {
                // Check low part comparison
                if ((unsigned __int128)temp > ULLONG_MAX / 2) {
                    accumulate((unsigned long long)temp);
                }
            }
        }
    }
    
    // 4. Bit-field operations and masking with wide integers
    // Create values that require high/low part comparisons
    unsigned __int128 mask_test = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 shift_test = mask_test >> 32;
    
    // Compare high parts and low parts separately
    if ((shift_test >> 64) > 0) {
        accumulate((unsigned long long)(shift_test >> 64));
    }
    if ((shift_test & 0xFFFFFFFFFFFFFFFFULL) > ULLONG_MAX / 2) {
        accumulate((unsigned long long)(shift_test & 0xFFFFFFFFFFFFFFFFULL));
    }
    
    // 5. Compiler built-ins for overflow checking
    long long a = LLONG_MAX / 2;
    long long b = 3;
    long long result;
    
    // These built-ins may trigger the fixed-value range analysis
    if (__builtin_mul_overflow(a, b, &result)) {
        accumulate(0xDEADBEEF);
    }
    
    // More complex 128-bit operations
    __int128 x = (__int128)0x7FFFFFFFFFFFFFFFLL * 4;
    __int128 y = (__int128)0x7FFFFFFFFFFFFFFFLL * 2;
    
    // Chain of comparisons that stress the double-int logic
    if (x > y) {
        if (x > 0) {
            if ((unsigned __int128)x > (unsigned __int128)y) {
                accumulate((unsigned long long)(x >> 64));
                accumulate((unsigned long long)(x & 0xFFFFFFFFFFFFFFFFULL));
            }
        }
    }
    
    // Boundary case testing - values near max/min
    __int128 near_max = (__int128)LLONG_MAX * 2 - 1;
    __int128 at_max = (__int128)LLONG_MAX * 2;
    
    // This comparison should trigger the exact uncovered code path
    // where high part equals max_r and low part needs comparison
    if (near_max > at_max - 100) {
        accumulate(0xCAFEBABE);
    }
    
    // Test with unsigned 128-bit values
    unsigned __int128 u_max = ~(unsigned __int128)0;
    unsigned __int128 u_near_max = u_max - 1000;
    
    // Force unsigned high/low part comparisons
    if (u_near_max > u_max / 2) {
        if ((u_near_max >> 64) == (u_max >> 64)) {
            // This should trigger the low part comparison
            if ((u_near_max & 0xFFFFFFFFFFFFFFFFULL) > (u_max & 0xFFFFFFFFFFFFFFFFULL) / 2) {
                accumulate(0xF00DF00D);
            }
        }
    }
    
    // Mixed signed/unsigned comparisons
    signed long long sval = -LLONG_MAX / 2;
    unsigned long long uval = ULLONG_MAX / 2;
    
    // Promote to 128-bit and compare
    if ((__int128)sval > (__int128)uval) {
        accumulate(0xBAADF00D);
    }
    
    // Final output to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
