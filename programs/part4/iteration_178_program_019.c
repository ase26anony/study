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

// Prevent inlining to ensure analysis happens in the fixed-value pass
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_amount bounded but non-constant
    shift_amount = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation with potential for overflow
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
    }
    
    // Multiply to further expand possible range
    shifted *= (modifier & 3) + 1;
    
    return shifted;
}

__attribute__((noinline))
int analyze_ranges(wide_int_t value, int iteration) {
    int result = 0;
    
    // Mixed signed/unsigned comparisons like in the target code
    // This should trigger the fixed-value range analysis
    
    // Signed comparison (sgt in the target)
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        result |= 1;
    }
    
    // Unsigned comparison (ugt in the target)
    uwide_int_t uval = (uwide_int_t)value;
    if (uval > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        result |= 2;
    }
    
    // Additional comparisons to create analysis complexity
    wide_int_t masked = value & ((wide_int_t)0x0F0F0F0F0F0F0F0FLL << (iteration & 7));
    if (masked < (wide_int_t)0) {
        result |= 4;
    }
    
    // Compound condition similar to the uncovered code
    if (value > (wide_int_t)0x3FFFFFFFFFFFFFFFLL || 
        (value == (wide_int_t)0x3FFFFFFFFFFFFFFFLL && 
         uval > (uwide_int_t)0x7FFFFFFFFFFFFFFFULL)) {
        result |= 8;
    }
    
    return result;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate multiple analysis contexts
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant inputs
        int shift = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int mod = (seed * i) & 0xFF;
        
        // Get a wide integer value through complex transformations
        wide_int_t computed = wide_int_compute(shift, mod);
        
        // Analyze it with mixed signed/unsigned comparisons
        int analysis_result = analyze_ranges(computed, i);
        
        // Update checksum to prevent dead code elimination
        checksum += (analysis_result * i) ^ (unsigned long long)computed;
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional control flow to create analysis complexity
        if (i % 100 == 0) {
            // Nested comparisons with different bounds
            wide_int_t temp = computed >> (shift & 15);
            if (temp > 0 && temp < (wide_int_t)(0x1LL << (shift & 31))) {
                checksum += temp;
            }
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
