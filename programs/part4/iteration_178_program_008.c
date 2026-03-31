#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MASK 63
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MASK 31
#endif

// Prevent inlining to ensure fixed-value analysis runs
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
    
    // Mask to ensure non-negative for unsigned comparisons
    uwide_int_t mask = (uwide_int_t)0xFFFFFFFFFFFFFFFFULL;
    if (modifier & 2) {
        shifted &= mask;
    } else {
        shifted |= (uwide_int_t)1ULL << (shift_amount & WIDE_SHIFT_MASK);
    }
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis paths
    for (int i = 0; i < 1000; i++) {
        // Bounded shift amounts (0-63 for __int128, 0-31 for long long)
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * 3 + i * 7) & WIDE_SHIFT_MASK;
        
        // Get signed wide integer result
        wide_int_t signed_result = wide_int_compute(shift1, i);
        
        // Get unsigned wide integer result  
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, i);
        
        // PATH 1: Signed comparison that may trigger max_r/max_s initialization
        // Compare against large positive signed value
        wide_int_t signed_threshold = (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
        if (signed_result > signed_threshold) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative threshold
        if (signed_result < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL)) {
            checksum += 2;
        }
        
        // Complex condition with && and || to encourage multiple range checks
        if (signed_result > 0 || (signed_result == 0 && i > 500)) {
            checksum += 4;
        }
        
        // PATH 2: Unsigned comparison that may trigger ugt logic
        uwide_int_t unsigned_threshold = (uwide_int_t)0xFFFFFFFFFFFFFFFFULL;
        if (unsigned_result > unsigned_threshold) {
            checksum += 8;
        }
        
        // Mixed signed/unsigned comparison after type conversion
        if ((wide_int_t)unsigned_result > signed_threshold) {
            checksum += 16;
        }
        
        // Nested conditions to create control flow complexity
        if (shift1 > 16) {
            wide_int_t temp = signed_result >> (shift1 & 15);
            if (temp > 0x7FFF) {
                checksum += 32;
            }
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
