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

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_amount bounded but non-constant for analysis
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
    // Signed comparison path - may trigger sgt comparison
    wide_int_t max_signed = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
    
    if (value > max_signed) {
        return 1;  // Overflow in signed domain
    }
    
    // Additional signed comparison with negative bound
    wide_int_t min_signed = -((wide_int_t)1 << (WIDE_SHIFT_MAX - 1));
    if (value < min_signed) {
        return -1;  // Underflow in signed domain
    }
    
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison path - may trigger ugt comparison
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
    
    if (value > max_unsigned / 2) {
        return 1;  // In upper half of unsigned range
    }
    
    // Mixed signed/unsigned in conditional
    if ((wide_int_t)value < 0 && value > 0) {
        // This condition is impossible but creates analysis complexity
        return 2;
    }
    
    return 0;
}

__attribute__((noinline))
int complex_conditional_check(wide_int_t a, wide_int_t b) {
    // Create complex conditional with mixed comparisons
    uwide_int_t ua = (uwide_int_t)a;
    uwide_int_t ub = (uwide_int_t)b;
    
    // This structure mimics the uncovered code's logic
    if (a > 0 || (a == 0 && ua > ub)) {
        return 1;
    }
    
    // Additional path with different comparison types
    if ((a < 0 && ua > 0x7FFFFFFFFFFFFFFFULL) || 
        (a >= 0 && ua <= 0xFFFFFFFFFFFFFFFFULL)) {
        return 2;
    }
    
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Vary inputs across iterations
    for (int i = 0; i < 10000; i++) {
        // Create bounded but non-constant shift amounts
        int shift1 = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed * i + 123) & (WIDE_SHIFT_MAX - 1);
        
        // Generate values requiring range analysis
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i + 1);
        
        // Trigger signed range analysis
        checksum += check_signed_range(val1);
        
        // Trigger unsigned range analysis
        checksum += check_unsigned_range((uwide_int_t)val2);
        
        // Trigger complex conditional analysis
        checksum += complex_conditional_check(val1, val2);
        
        // Additional mixed-width comparison
        if ((val1 > 0 && (uwide_int_t)val1 > 0xFFFFFFFFULL) ||
            (val1 < 0 && (uwide_int_t)(-val1) > 0x8000000000000000ULL)) {
            checksum += 3;
        }
        
        // Update seed to vary next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
