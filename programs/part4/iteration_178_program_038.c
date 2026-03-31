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

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create complex range
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x123456789ABCDEFLL;
    }
    
    // Multiply to expand range further
    shifted *= (modifier & 3) + 1;
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i) {
    int result = 0;
    
    // This comparison should trigger signed range analysis
    // The constant 0x7FFFFFFFFFFFFFFFLL is max positive for signed 64-bit
    // For 128-bit, it's still a large positive value
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        result |= 1;
    }
    
    // Another signed comparison with negative threshold
    if (value < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL)) {
        result |= 2;
    }
    
    // Mixed signed/unsigned comparison scenario
    uwide_int_t uval = (uwide_int_t)value;
    
    // This unsigned comparison should trigger unsigned range analysis
    if (uval > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        result |= 4;
    }
    
    // Complex condition with both signed and unsigned aspects
    // This mimics the structure in the uncovered code
    if (value > (wide_int_t)0x3FFFFFFFFFFFFFFFLL ||
        ((uwide_int_t)value > (uwide_int_t)0x7FFFFFFFFFFFFFFFULL && 
         value < (wide_int_t)0)) {
        result |= 8;
    }
    
    return result;
}

__attribute__((noinline))
wide_int_t create_min_max_scenario(int shift, int mod) {
    wide_int_t val = wide_int_compute(shift, mod);
    
    // Create conditional paths that affect the value
    if (shift > (WIDE_SHIFT_MASK / 2)) {
        // Right shift for unsigned range analysis
        uwide_int_t uval = (uwide_int_t)val;
        uval >>= (shift / 2);
        val = (wide_int_t)uval;
    } else {
        // Left shift for signed range analysis
        val <<= (shift + 1);
    }
    
    // Add more arithmetic complexity
    val = val + (wide_int_t)(mod * 0x10001);
    val = val * (wide_int_t)((shift & 7) + 1);
    
    return val;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    int checksum = 0;
    
    // Loop to generate many different scenarios
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & WIDE_SHIFT_MASK;
        int mod1 = (seed + i * 3) & 0xFF;
        int mod2 = (seed * i + 123) & 0xFF;
        
        // Create values that require range analysis
        wide_int_t val1 = create_min_max_scenario(shift1, mod1);
        wide_int_t val2 = create_min_max_scenario(shift2, mod2);
        
        // Perform comparisons that should trigger the target code
        int result1 = check_signed_range(val1, i);
        int result2 = check_signed_range(val2, i);
        
        // Additional complex comparison
        if (val1 > val2) {
            result1 |= 16;
        }
        
        // Mixed signed/unsigned comparison in conditional
        uwide_int_t uval1 = (uwide_int_t)val1;
        uwide_int_t uval2 = (uwide_int_t)val2;
        
        if (uval1 > uval2 && val1 < val2) {
            // This condition is designed to exercise both
            // signed and unsigned comparison logic
            result1 |= 32;
        }
        
        checksum += result1 + result2;
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
