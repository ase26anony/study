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

// Prevent inlining to keep analysis complex
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift amount bounded but non-constant
    shift_amount = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation: shift then add/subtract
    wide_int_t shifted = base << shift_amount;
    
    // Create both positive and negative possibilities
    if (modifier & 1) {
        shifted = shifted + (modifier * 1000);
    } else {
        shifted = shifted - (modifier * 500);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value) {
    // Signed comparison path - will trigger sgt comparison
    wide_int_t max_signed = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
    
    if (value > max_signed) {
        return 1;
    }
    
    // Also check negative range
    wide_int_t min_signed = -((wide_int_t)1 << (WIDE_SHIFT_MAX - 1));
    if (value < min_signed) {
        return -1;
    }
    
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison path - will trigger ugt comparison
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
    
    if (value > max_unsigned / 2) {
        return 1;
    }
    
    // Mixed signed/unsigned in conditional
    if ((wide_int_t)value < 0 && value > 1000) {
        return 2;
    }
    
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis paths
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant shift amounts
        int shift1 = (seed + i) & 31;
        int shift2 = (seed * i) & 63;
        
        // Get a wide integer result
        wide_int_t result1 = wide_int_compute(shift1, i);
        wide_int_t result2 = wide_int_compute(shift2, seed);
        
        // Path 1: Signed comparison (triggers sgt)
        int signed_check = check_signed_range(result1);
        
        // Path 2: Unsigned comparison (triggers ugt)
        // Convert to unsigned for unsigned comparison
        uwide_int_t uresult = (uwide_int_t)result2;
        int unsigned_check = check_unsigned_range(uresult);
        
        // Complex conditional mixing both comparisons
        // This encourages the compiler to analyze multiple range constraints
        if ((signed_check != 0) || (unsigned_check != 0)) {
            checksum += (unsigned long long)result1;
            checksum += (unsigned long long)result2;
        }
        
        // Another path with direct comparisons
        wide_int_t temp = result1 + result2;
        wide_int_t max_r_val = 0x7FFFFFFFFFFFFFFFLL;
        
        // Direct comparison that should trigger the target block
        if (temp > max_r_val) {
            checksum ^= (unsigned long long)temp;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional bounded shift to create range analysis
        int bounded_shift = (seed >> 16) & (WIDE_SHIFT_MAX / 2);
        wide_int_t shifted_again = result1 << bounded_shift;
        
        // Mixed signed/unsigned comparison in single expression
        // This is key for triggering the specific uncovered block
        if ((shifted_again > 0) && ((uwide_int_t)shifted_again > 0xFFFFFFFFULL)) {
            checksum += bounded_shift;
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
