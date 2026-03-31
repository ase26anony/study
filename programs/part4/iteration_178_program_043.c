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

// Prevent inlining to ensure analysis happens
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_count, int modifier) {
    // Start with a value that can become negative
    wide_int_t base = (wide_int_t)modifier - 50;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_count;
    
    // Additional arithmetic to create complex value ranges
    wide_int_t result = shifted + (modifier * 1000LL);
    
    return result;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i_f_bits) {
    // This comparison should trigger signed range analysis
    // The constant here approximates max_r/max_s from the uncovered code
    wide_int_t max_signed = ((wide_int_t)1 << (i_f_bits - 1)) - 1;
    
    if (value > max_signed) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i_f_bits) {
    // This comparison should trigger unsigned range analysis
    uwide_int_t max_unsigned = ((uwide_int_t)1 << i_f_bits) - 1;
    
    if (value > max_unsigned) {
        return 1;
    }
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate multiple analysis scenarios
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift counts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & WIDE_SHIFT_MASK;
        
        // Generate values with different ranges
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, 1000 - i);
        
        // Mixed signed/unsigned comparisons
        int result1 = check_signed_range(val1, 60);
        int result2 = check_signed_range(val2, 62);
        
        // Convert to unsigned for unsigned comparisons
        uwide_int_t uval1 = (uwide_int_t)val1;
        uwide_int_t uval2 = (uwide_int_t)val2;
        
        int result3 = check_unsigned_range(uval1, 61);
        int result4 = check_unsigned_range(uval2, 63);
        
        // Complex conditional with mixed comparisons
        if ((result1 && result3) || (result2 && result4)) {
            checksum += (uval1 & 0xFFFF) + (uval2 & 0xFFFF);
        } else if (val1 > val2) {
            // Another signed comparison
            checksum += 1;
        } else if (uval1 > uval2) {
            // Another unsigned comparison
            checksum += 2;
        }
        
        // Additional path with arithmetic that affects range analysis
        wide_int_t combined = val1 + val2;
        if (combined < 0) {
            // Negative range check
            checksum += 3;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
