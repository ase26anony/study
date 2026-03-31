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

// Prevent inlining to keep analysis complex
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Bounded shift amount (0 to WIDE_SHIFT_MAX-1)
    int bounded_shift = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation: shift then add/subtract
    wide_int_t shifted = base << bounded_shift;
    
    // Conditional arithmetic based on modifier
    if (modifier & 1) {
        shifted = shifted + (modifier * 0x10001);
    } else {
        shifted = shifted - (modifier * 0xFFFF);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value) {
    // Signed comparison against large positive value
    // This should trigger signed greater-than (sgt) analysis
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison against max value
    // This should trigger unsigned greater-than (ugt) analysis
    if (value > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int mixed_comparison(wide_int_t a_high, uwide_int_t a_low) {
    // Simulate the comparison structure from the uncovered code
    // This creates analysis complexity with mixed signed/unsigned
    int result = 0;
    
    // Create conditions similar to the uncovered block
    wide_int_t max_r = 0;
    uwide_int_t max_s = (uwide_int_t)-1;
    
    // Force range analysis to compute bounds
    if (a_high > max_r) {
        result |= 1;
    }
    
    if ((wide_int_t)a_high == max_r && a_low > max_s) {
        result |= 2;
    }
    
    return result;
}

int main() {
    volatile int seed = 0x12345678;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Vary inputs across iterations
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant values
        int shift = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int modifier = (seed * i) & 0xFF;
        
        // Get a wide integer result
        wide_int_t result = wide_int_compute(shift, modifier);
        
        // Path 1: Signed comparison
        int signed_check = check_signed_range(result);
        checksum += signed_check;
        
        // Path 2: Unsigned comparison
        int unsigned_check = check_unsigned_range((uwide_int_t)result);
        checksum += unsigned_check * 2;
        
        // Path 3: Mixed signed/unsigned comparison
        // Split the wide int into high and low parts for analysis
        wide_int_t a_high = result >> (WIDE_SHIFT_MAX / 2);
        uwide_int_t a_low = (uwide_int_t)result & ((uwide_int_t)-1 >> 1);
        
        int mixed_check = mixed_comparison(a_high, a_low);
        checksum += mixed_check * 4;
        
        // Modify seed to vary next iteration
        seed = seed * 1103515245 + 12345;
        
        // Additional complexity: conditional shift based on result
        if (result > 0) {
            wide_int_t shifted_again = result << ((seed >> 3) & 7);
            checksum += (shifted_again > 0) ? 1 : 0;
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
