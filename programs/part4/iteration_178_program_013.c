#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MAX 127
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MAX 63
#endif

// Prevent inlining to ensure analysis happens at optimization level
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = 1LL << (WIDE_SHIFT_MAX / 2);
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed complexity
    if (modifier & 1) {
        shifted += (wide_int_t)modifier * 0x123456789ABCDEFLL;
    } else {
        shifted -= (wide_int_t)modifier * 0xFEDCBA987654321LL;
    }
    
    return shifted;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with non-negative value
    uwide_int_t base = 0xFFFFFFFFULL;
    
    // Logical shift right to ensure non-negative
    uwide_int_t shifted = base >> (shift_amount & 31);
    
    // Mask to keep within bounds but create analysis complexity
    shifted = shifted & ((1ULL << (WIDE_SHIFT_MAX - 10)) - 1);
    
    // Add based on modifier
    shifted += (uwide_int_t)modifier * 0x100000001ULL;
    
    return shifted;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    uint64_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts
        int shift1 = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed * i) & 31;
        int mod1 = (seed ^ i) & 0xFF;
        int mod2 = (seed + i * 3) & 0xFF;
        
        // Get wide integer results
        wide_int_t signed_result = wide_int_compute(shift1, mod1);
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, mod2);
        
        // PATH 1: Signed comparison that may trigger max_r/min_s initialization
        // Compare against large positive signed value
        wide_int_t signed_threshold = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 2)) - 1;
        if (signed_result > signed_threshold) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative threshold
        wide_int_t negative_threshold = -((wide_int_t)1 << (WIDE_SHIFT_MAX - 3));
        if (signed_result < negative_threshold) {
            checksum += 2;
        }
        
        // Complex condition mixing signed and unsigned aspects
        if (signed_result > 0 && (uwide_int_t)signed_result > 0x7FFFFFFFFFFFFFFFULL) {
            checksum += 4;
        }
        
        // PATH 2: Unsigned comparison
        uwide_int_t unsigned_threshold = ((uwide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
        if (unsigned_result > unsigned_threshold) {
            checksum += 8;
        }
        
        // Mixed comparison: signed variable in unsigned context
        if ((uwide_int_t)signed_result > unsigned_threshold >> 1) {
            checksum += 16;
        }
        
        // Complex condition that might trigger the uncovered block
        // This resembles the structure in the uncovered lines
        wide_int_t a_high = signed_result >> (WIDE_SHIFT_MAX / 2);
        uwide_int_t a_low = (uwide_int_t)signed_result & (((uwide_int_t)1 << (WIDE_SHIFT_MAX / 2)) - 1);
        
        // This comparison structure should trigger the target code
        wide_int_t max_r_val = 0;
        uwide_int_t max_s_val = (uwide_int_t)-1;
        max_s_val = max_s_val & (((uwide_int_t)1 << (WIDE_SHIFT_MAX / 2)) - 1);
        
        if (a_high > max_r_val || (a_high == max_r_val && a_low > max_s_val)) {
            checksum += 32;
        }
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
