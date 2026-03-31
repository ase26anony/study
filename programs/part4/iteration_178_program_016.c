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

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline, noipa))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_amount bounded but non-constant for analysis
    shift_amount = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation: shift then add/subtract
    wide_int_t shifted = base << shift_amount;
    
    // Create both positive and negative possibilities
    if (modifier & 1) {
        shifted = shifted + (wide_int_t)(modifier * 0x1000);
    } else {
        shifted = shifted - (wide_int_t)(modifier * 0x800);
    }
    
    return shifted;
}

__attribute__((noinline, noipa))
int check_signed_range(wide_int_t value, int i) {
    // This triggers signed comparison analysis
    wide_int_t max_signed = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
    
    // Complex condition similar to the uncovered code
    if (value > max_signed) {
        return 1;
    }
    
    // Additional check with equality case
    if (value == max_signed && (uwide_int_t)value > (uwide_int_t)max_signed) {
        return 2;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int check_unsigned_range(uwide_int_t value, int i) {
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
    
    // Mixed signed/unsigned comparison scenario
    if ((wide_int_t)value > (wide_int_t)(max_unsigned >> 1)) {
        return 1;
    }
    
    // Another condition that might trigger the uncovered block
    if (value > max_unsigned - (i & 0xFFF)) {
        return 2;
    }
    
    return 0;
}

int main() {
    volatile int seed = 0xDEADBEEF;  // Prevent constant propagation
    int checksum = 0;
    
    // Loop to generate multiple analysis scenarios
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant shift amounts
        int shift1 = (seed + i * 3) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed ^ i * 5) & (WIDE_SHIFT_MAX - 1);
        
        // Compute wide integer values
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i + 1);
        
        // Perform signed range checks (triggers sgt comparison)
        checksum += check_signed_range(val1, i);
        checksum += check_signed_range(val2, i);
        
        // Perform unsigned range checks (triggers ugt comparison)
        checksum += check_unsigned_range((uwide_int_t)val1, i);
        checksum += check_unsigned_range((uwide_int_t)val2, i);
        
        // Complex conditional with mixed comparisons
        if ((val1 > 0 && (uwide_int_t)val1 > 0x7FFFFFFFFFFFFFFFULL) ||
            (val2 < 0 && (uwide_int_t)val2 > 0xFFFFFFFF00000000ULL)) {
            checksum += 3;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional bounded shift that depends on previous results
        int dynamic_shift = (checksum + i) & 31;
        wide_int_t dynamic_val = val1 << dynamic_shift;
        
        // Comparison that depends on shift result
        if (dynamic_val > (wide_int_t)0x7FFFFFFFFFFFFFFFLL >> (dynamic_shift & 15)) {
            checksum += 5;
        }
    }
    
    // Prevent dead code elimination
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
