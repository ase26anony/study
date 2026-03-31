#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MAX 127
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MAX 63
#endif

// Prevent inlining to keep arithmetic opaque to early optimization passes
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)(modifier * 0x1000);
    } else {
        shifted -= (wide_int_t)(modifier * 0x800);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i_f_bits) {
    // This comparison should trigger signed range analysis
    // The constant here is designed to be near the boundary
    // that requires double-int comparison
    if (value > (wide_int_t)((1LL << (i_f_bits - 1)) - 1)) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i_f_bits) {
    // This comparison should trigger unsigned range analysis
    // The mask creates a boundary that requires double-int handling
    uwide_int_t mask = ((uwide_int_t)1 << i_f_bits) - 1;
    if (value > mask) {
        return 1;
    }
    return 0;
}

int main() {
    volatile int seed = 42;  // volatile to prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis contexts
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts
        int shift1 = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed * 3 + i * 5) & (WIDE_SHIFT_MAX - 1);
        
        // Get wide integer values with complex transformations
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i + 1);
        
        // Mixed signed/unsigned comparisons in conditional expressions
        // This should trigger the specific uncovered block
        
        // Path 1: Signed comparison that may trigger max_r/max_s initialization
        if (val1 > (wide_int_t)0x7FFFFFFFFFFFFFFFLL || 
            (val1 == (wide_int_t)0x7FFFFFFFFFFFFFFFLL && 
             (uwide_int_t)val2 > 0xFFFFFFFFFFFFFFFFULL)) {
            checksum += 1;
        }
        
        // Path 2: More complex condition with different bit widths
        // Vary i_f_bits to trigger different boundary calculations
        int i_f_bits = 32 + (i % 32);
        if (check_signed_range(val1, i_f_bits) || 
            check_unsigned_range((uwide_int_t)val2, i_f_bits)) {
            checksum += 2;
        }
        
        // Path 3: Direct comparison that mimics the uncovered code pattern
        // Create a situation where a_high and a_low need comparison
        // against initialized max_r/min_s structures
        wide_int_t combined = val1 + val2;
        uwide_int_t ucombined = (uwide_int_t)combined;
        
        // This structure should trigger the double-int range initialization
        if (combined > (wide_int_t)((1LL << 60) - 1) ||
            (combined == (wide_int_t)((1LL << 60) - 1) &&
             ucombined > (uwide_int_t)((1ULL << 61) - 1))) {
            checksum += 3;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
