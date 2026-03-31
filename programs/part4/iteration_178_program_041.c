#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
    wide_int_t base = (wide_int_t)modifier * 1000000007LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add another value that depends on shift_amount
    // This creates complex value ranges
    wide_int_t result = shifted + (shift_amount * 123456789LL);
    
    // Conditional transformation based on shift_amount
    if (shift_amount > (WIDE_SHIFT_MASK / 2)) {
        // For large shifts, perform additional operations
        result = result - ((wide_int_t)modifier << (shift_amount / 2));
    } else {
        // For small shifts, different transformation
        result = result + ((wide_int_t)(shift_amount * 987654321LL));
    }
    
    return result;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with positive value
    uwide_int_t base = (uwide_int_t)modifier * 0x123456789ABCDEFULL;
    
    // Logical shift right - keeps value non-negative
    uwide_int_t shifted = base >> (shift_amount & (WIDE_SHIFT_MASK - 1));
    
    // Mask operation ensures result stays in certain range
    uwide_int_t masked = shifted & 0xFFFFFFFFFFFFFFFFULL;
    
    // Add value that could cause overflow analysis
    uwide_int_t result = masked + (uwide_int_t)(shift_amount * 0x100000001ULL);
    
    return result;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate many different analysis scenarios
    for (int i = 0; i < 10000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i * 3) & WIDE_SHIFT_MASK;
        int shift2 = (seed + i * 7) & (WIDE_SHIFT_MASK - 1);
        int mod1 = (seed + i * 11) & 0xFF;
        int mod2 = (seed + i * 13) & 0xFF;
        
        // Compute wide integer values
        wide_int_t signed_result = wide_int_compute(shift1, mod1);
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, mod2);
        
        // MIXED SIGNED/UNSIGNED COMPARISONS
        // This should trigger the range analysis for double-int types
        
        // 1. Signed comparison against large positive constant
        // Forces analysis of signed range bounds
        if (signed_result > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // 2. Complex condition with equality check (matches uncovered code pattern)
        if (signed_result < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL) ||
            (signed_result > 0 && signed_result < 1000)) {
            checksum += 2;
        }
        
        // 3. Unsigned comparison against max value
        // Forces unsigned range analysis
        if ((uwide_int_t)signed_result > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 4;
        }
        
        // 4. Mixed signed/unsigned in same expression
        // Complex condition that requires both analyses
        wide_int_t temp = signed_result + (wide_int_t)unsigned_result;
        if (temp > 0 && (uwide_int_t)temp < 0x8000000000000000ULL) {
            checksum += 8;
        }
        
        // 5. Additional comparison to trigger min_s initialization
        // The negative constant should trigger the min_s = -1 initialization
        if (signed_result < -0x1000000000000000LL) {
            checksum += 16;
        }
        
        // 6. Comparison that might trigger both max_r and min_r analysis
        // Using a value that could be very large positive or negative
        wide_int_t complex_val = signed_result << (shift1 & 3);
        if (complex_val > 0x3FFFFFFFFFFFFFFFLL || 
            complex_val < -0x3FFFFFFFFFFFFFFFLL) {
            checksum += 32;
        }
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    // Print result to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    // Additional test with specific values to target the uncovered lines
    // These values are chosen to exercise boundary conditions
    {
        int test_shifts[] = {0, 1, WIDE_SHIFT_MASK/2, WIDE_SHIFT_MASK-1};
        int test_mods[] = {0, 1, 127, 255};
        
        for (int s = 0; s < 4; s++) {
            for (int m = 0; m < 4; m++) {
                wide_int_t val = wide_int_compute(test_shifts[s], test_mods[m]);
                
                // Comparisons designed to trigger the specific uncovered block
                // The constants here are chosen to match patterns in the uncovered code
                if (val > 0x7FFFFFFFFFFFFFFFLL) {
                    checksum += 64;
                }
                
                if ((uwide_int_t)val > 0xFFFFFFFFFFFFFFFFULL) {
                    checksum += 128;
                }
                
                // Test with negative bounds
                if (val < -0x7FFFFFFFFFFFFFFFLL) {
                    checksum += 256;
                }
            }
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
