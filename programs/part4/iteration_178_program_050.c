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

// Prevent inlining to ensure arithmetic isn't optimized away before analysis
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create complex value ranges
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
    }
    
    // Multiply to further complicate range analysis
    shifted *= (modifier & 3) + 1;
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i_f_bits) {
    // This comparison should trigger signed range analysis
    // The constant 0x7F... is INT64_MAX for 64-bit, similar for 128-bit
#ifdef __SIZEOF_INT128__
    wide_int_t max_signed = ((__int128)1 << 127) - 1;
#else
    wide_int_t max_signed = 0x7FFFFFFFFFFFFFFFLL;
#endif
    
    if (value > max_signed) {
        return 1;
    }
    
    // Additional signed comparison with negative value
    if (value < -max_signed / 2) {
        return 2;
    }
    
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i_f_bits) {
    // This comparison should trigger unsigned range analysis
#ifdef __SIZEOF_INT128__
    uwide_int_t max_unsigned = ~((unsigned __int128)0);
#else
    uwide_int_t max_unsigned = 0xFFFFFFFFFFFFFFFFULL;
#endif
    
    if (value > max_unsigned >> (i_f_bits & 31)) {
        return 1;
    }
    
    // Complex unsigned comparison with mixed operations
    uwide_int_t threshold = max_unsigned / ((i_f_bits & 7) + 2);
    if (value > threshold && value < max_unsigned - threshold) {
        return 2;
    }
    
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate many different cases
    for (int i = 0; i < 10000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i * 3) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed + i * 7) & 31;
        int modifier = (seed + i * 11) & 255;
        
        // Compute wide integer with complex transformations
        wide_int_t result = wide_int_compute(shift1, modifier);
        
        // Perform both signed and unsigned comparisons
        // to trigger both sgt and ugt comparisons in the analysis
        int signed_result = check_signed_range(result, shift2);
        int unsigned_result = check_unsigned_range((uwide_int_t)result, shift2);
        
        // Mix signed and unsigned comparisons in conditional expressions
        // This encourages the compiler to evaluate multiple range constraints
        if ((signed_result > 0) || (unsigned_result > 0 && signed_result == 0)) {
            checksum += (unsigned_result << 16) | signed_result;
        }
        
        // Additional complex conditional with mixed comparisons
        wide_int_t temp = result >> (shift2 & 15);
        if (temp > 0 && (uwide_int_t)temp < 0x1000000000000000ULL) {
            checksum += temp & 0xFFFF;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Force another computation with different parameters
        // to create multiple analysis paths
        if (i & 1) {
            int alt_shift = (seed + i) & (WIDE_SHIFT_MAX / 2);
            wide_int_t alt_result = wide_int_compute(alt_shift, modifier ^ 0xFF);
            
            // Nested comparisons to increase analysis complexity
            if (alt_result > result) {
                if ((uwide_int_t)alt_result < (uwide_int_t)result * 2) {
                    checksum += i;
                }
            }
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
