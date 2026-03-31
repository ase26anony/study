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
    // The shift amount is bounded but not constant
    wide_int_t shifted = base << shift_amount;
    
    // Add a modifier that can make the value negative
    // This creates signed range complexity
    shifted += (wide_int_t)modifier * 0x100000001LL;
    
    // Additional arithmetic to create analysis complexity
    if (modifier & 1) {
        // For odd modifiers, perform additional transformation
        shifted = shifted * 3 / 2;
    } else {
        // For even modifiers, different transformation
        shifted = shifted - (base >> (shift_amount & 7));
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i) {
    // This comparison should trigger signed range analysis
    // The constant 0x7FFFFFFFFFFFFFFF is INT64_MAX
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        return 1;
    }
    
    // Additional signed comparison with negative value
    if (value < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
        return 2;
    }
    
    // Mixed signed/unsigned comparison scenario
    uwide_int_t uval = (uwide_int_t)value;
    if (i & 1) {
        // Sometimes compare as unsigned
        if (uval > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
            return 3;
        }
    }
    
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int shift) {
    // This should trigger unsigned range analysis
    // The comparison against max unsigned value
    if (value > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        return 1;
    }
    
    // Complex conditional with both comparisons
    if (value > (uwide_int_t)0x8000000000000000ULL && 
        value < (uwide_int_t)0xFFFFFFFF00000000ULL) {
        return 2;
    }
    
    // Additional transformation that might trigger min_s initialization
    uwide_int_t shifted = value << (shift & 15);
    if (shifted == 0) {
        return 3;
    }
    
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    int checksum = 0;
    
    // Loop to generate many different cases
    for (int i = 0; i < 10000; i++) {
        // Generate bounded but non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & 31;
        
        // Call the wide integer computation
        wide_int_t result = wide_int_compute(shift1, shift2);
        
        // Perform signed range check - this should trigger the target code
        checksum += check_signed_range(result, i);
        
        // Also perform unsigned range check
        uwide_int_t uresult = (uwide_int_t)result;
        checksum += check_unsigned_range(uresult, shift1);
        
        // Additional complex comparison that mixes signed and unsigned
        // This mimics the structure in the uncovered lines
        if ((result > (wide_int_t)0) || 
            ((uwide_int_t)result > (uwide_int_t)0xFFFFFFFFULL)) {
            checksum++;
        }
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional case: shift in the opposite direction
        if (i & 3) {
            wide_int_t right_shifted = result >> (shift2 & 15);
            if (right_shifted < 0) {
                checksum += 5;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
