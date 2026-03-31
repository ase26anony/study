#include <stdio.h>
#include <stdint.h>

// Prevent inlining to ensure the fixed-value analysis runs
__attribute__((noinline)) 
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (__int128)0x8000000000000000LL;
    } else {
        shifted -= (__int128)0x4000000000000000LL;
    }
    
    return shifted;
}

__attribute__((noinline))
unsigned __int128 unsigned_wide_compute(int shift_amount, int mask) {
    // Start with unsigned value
    unsigned __int128 base = (unsigned __int128)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    unsigned __int128 shifted = base << shift_amount;
    
    // Apply mask to create bounded but non-constant range
    shifted &= (unsigned __int128)mask;
    shifted |= (unsigned __int128)0x1000000000000000ULL;
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis contexts
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts (0-63 for 64-bit shifts)
        int shift1 = (seed + i) & 63;
        int shift2 = (seed * i) & 63;
        int modifier = (seed + i * 3) & 0xFF;
        
        // Compute signed wide integer
        __int128 signed_result = wide_int_compute(shift1, modifier);
        
        // Compute unsigned wide integer  
        unsigned __int128 unsigned_result = unsigned_wide_compute(shift2, modifier);
        
        // PATH 1: Signed comparison that should trigger max_r/max_s initialization
        // Compare against large positive signed value (0x7F... is max positive for 128-bit signed)
        if (signed_result > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            // This comparison needs to analyze the range of signed_result
            checksum += 1;
        }
        
        // PATH 2: Mixed signed/unsigned comparison with complex conditions
        // The uncovered code has both sgt and ugt comparisons
        if (signed_result > 0 || (signed_result == 0 && 
            (unsigned __int128)signed_result > (unsigned __int128)0xFFFFFFFFULL)) {
            checksum += 2;
        }
        
        // PATH 3: Unsigned comparison that should trigger min_r/min_s initialization
        // Compare against max unsigned 64-bit value
        if (unsigned_result > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 4;
        }
        
        // PATH 4: Complex condition with both signed and unsigned parts
        // This mimics the structure in the uncovered code: (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
        {
            // Simulate high/low parts of a wide integer
            unsigned long long high_part = (unsigned long long)(signed_result >> 64);
            unsigned long long low_part = (unsigned long long)signed_result;
            
            // Create comparison that might trigger the exact uncovered block
            if ((__int128)signed_result > (__int128)0x7FFFFFFFFFFFFFFFLL || 
                (signed_result == (__int128)0x7FFFFFFFFFFFFFFFLL && 
                 low_part > 0xFFFFFFFFULL)) {
                checksum += 8;
            }
        }
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional arithmetic to create more analysis complexity
        if (i % 3 == 0) {
            signed_result = signed_result << (modifier & 7);
        } else if (i % 3 == 1) {
            signed_result = signed_result >> (modifier & 7);
        }
        
        // Final comparison with result of additional transformation
        if (signed_result > 0 && signed_result < (__int128)0x10000000000000000LL) {
            checksum += 16;
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
