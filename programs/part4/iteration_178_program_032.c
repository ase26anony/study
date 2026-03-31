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
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
    }
    
    // Multiply to increase range complexity
    shifted *= (modifier & 3) + 1;
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i) {
    // This comparison should trigger signed range analysis
    // with max_r/max_s initialization
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i) {
    // This comparison should trigger unsigned range analysis
    // with min_r/min_s initialization
    if (value > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int mixed_comparisons(wide_int_t a_high, uwide_int_t a_low, int i_f_bits) {
    // Complex conditional with both signed and unsigned comparisons
    // This structure mimics the uncovered code's logic
    if (a_high > (wide_int_t)0 || 
        (a_high == (wide_int_t)0 && a_low > (uwide_int_t)0xFFFFFFFF)) {
        return 1;
    }
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & WIDE_SHIFT_MASK;
        
        // Compute wide integer values
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i + 1);
        
        // Trigger signed comparison analysis
        if (check_signed_range(val1, i)) {
            checksum += 1;
        }
        
        // Trigger unsigned comparison analysis
        if (check_unsigned_range((uwide_int_t)val2, i)) {
            checksum += 2;
        }
        
        // Create values that might trigger the specific comparison pattern
        // from the uncovered lines
        wide_int_t a_high = val1 >> (shift1 & 15);
        uwide_int_t a_low = (uwide_int_t)val2 & 0xFFFFFFFFULL;
        
        // This should trigger the exact comparison structure
        // that initializes max_r, max_s, min_r, min_s
        if (mixed_comparisons(a_high, a_low, shift1 & 31)) {
            checksum += 3;
        }
        
        // Additional complex comparison to increase analysis pressure
        wide_int_t temp = val1 + val2;
        if (temp > 0 && (uwide_int_t)temp < 0x1000000000000000ULL) {
            checksum += 5;
        }
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
