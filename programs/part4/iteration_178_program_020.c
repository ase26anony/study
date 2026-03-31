#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MASK 63
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MASK 31
#endif

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed complexity
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
    }
    
    return shifted;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with unsigned value
    uwide_int_t base = (uwide_int_t)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    uwide_int_t shifted = base << shift_amount;
    
    // Apply mask to ensure non-negative for unsigned comparisons
    uwide_int_t mask = (uwide_int_t)0xFFFFFFFFFFFFFFFFULL;
    if (modifier & 2) {
        shifted &= mask;
    } else {
        shifted |= (uwide_int_t)1ULL << (shift_amount & WIDE_SHIFT_MASK);
    }
    
    return shifted;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    uint64_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i + 123) & WIDE_SHIFT_MASK;
        
        // Get signed wide integer result
        wide_int_t signed_result = wide_int_compute(shift1, i);
        
        // Get unsigned wide integer result  
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, i);
        
        // PATH 1: Signed comparison that may trigger max_r/min_s initialization
        // Compare against large positive signed value
        if (signed_result > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative bound
        if (signed_result < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
            checksum += 2;
        }
        
        // Complex condition mixing signed and unsigned comparisons
        // This mimics the structure in the uncovered code
        if (signed_result > 0 || 
            ((uwide_int_t)signed_result) > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 4;
        }
        
        // PATH 2: Unsigned comparison
        // Compare against max unsigned value
        if (unsigned_result > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 8;
        }
        
        // Mixed comparison: signed value compared after extension
        wide_int_t temp = signed_result;
        // Force sign extension simulation
        if (shift1 > 16) {
            temp = temp << (shift1 - 16);
        }
        
        // This comparison structure closely matches the uncovered code pattern
        // It creates conditions where both signed and unsigned comparisons
        // are needed for the same value
        uwide_int_t temp_unsigned = (uwide_int_t)temp;
        if (temp > (wide_int_t)0x7FFFFFFFFFFFFFFFLL ||
            (temp == (wide_int_t)0x7FFFFFFFFFFFFFFFLL &&
             temp_unsigned > 0xFFFFFFFFFFFFFFFFULL)) {
            checksum += 16;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
