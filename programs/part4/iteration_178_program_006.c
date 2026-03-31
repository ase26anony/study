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
    
    // Add a modifier that could make it positive or negative
    // Use signed arithmetic to create range uncertainty
    wide_int_t result = shifted + (modifier * 0x100000001LL);
    
    return result;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value) {
    // Signed comparison against large positive value
    // This should trigger signed greater-than (sgt) comparison
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison - triggers ugt comparison
    if (value > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int complex_range_check(wide_int_t a, wide_int_t b) {
    // Mixed signed/unsigned comparisons in conditional expression
    // Similar to the uncovered code's structure
    uwide_int_t ua = (uwide_int_t)a;
    uwide_int_t ub = (uwide_int_t)b;
    
    // Complex condition with both signed and unsigned comparisons
    if (a > (wide_int_t)0x3FFFFFFFFFFFFFFFLL || 
        (a == (wide_int_t)0x3FFFFFFFFFFFFFFFLL && ua > ub)) {
        return 1;
    }
    
    // Another path with different bounds
    if (a < (wide_int_t)(-0x4000000000000000LL) ||
        (a == (wide_int_t)(-0x4000000000000000LL) && ua < ub)) {
        return -1;
    }
    
    return 0;
}

int main() {
    volatile int seed = 0xDEADBEEF;  // Prevent constant propagation
    int checksum = 0;
    
    // Loop to generate multiple analysis scenarios
    for (int i = 0; i < 1000; i++) {
        // Bounded, non-constant shift amounts
        int shift = (seed + i) & WIDE_SHIFT_MASK;
        int modifier = (seed * i) & 0xFF;
        
        // Get a wide integer result with uncertain range
        wide_int_t result = wide_int_compute(shift, modifier);
        
        // Path 1: Signed comparison
        checksum += check_signed_range(result);
        
        // Path 2: Unsigned comparison
        checksum += check_unsigned_range((uwide_int_t)result);
        
        // Path 3: Complex mixed comparison
        // Create another value with different transformation
        wide_int_t result2 = result >> ((shift + 1) & WIDE_SHIFT_MASK);
        checksum += complex_range_check(result, result2);
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    // Print to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
