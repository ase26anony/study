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

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)0x8000000000000000LL;
    } else {
        shifted -= (wide_int_t)0x4000000000000000LL;
    }
    
    // Additional arithmetic to create analysis complexity
    if (shift_amount > 16) {
        shifted *= (modifier & 0xFF) + 1;
    }
    
    return shifted;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with unsigned value
    uwide_int_t base = (uwide_int_t)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    uwide_int_t shifted = base << shift_amount;
    
    // Mask to ensure non-negative for unsigned comparisons
    uwide_int_t mask = ((uwide_int_t)1 << (shift_amount & (WIDE_SHIFT_MAX-1))) - 1;
    shifted &= ~mask;
    
    // Add based on modifier
    shifted += (modifier * 0x10001ULL) & 0xFFFFFFFFULL;
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    int checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i * 3) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed + i * 7) & 31;  // Smaller range for some operations
        
        // Get modifier values
        int mod1 = (seed ^ i) & 0xFF;
        int mod2 = (i * 1103515245 + 12345) & 0xFF;
        
        // Compute wide integer values
        wide_int_t signed_val = wide_int_compute(shift1, mod1);
        uwide_int_t unsigned_val = unsigned_wide_compute(shift2, mod2);
        
        // PATH 1: Signed comparison that should trigger max_r/min_s initialization
        // Compare against large positive signed value
        if (signed_val > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative bound
        if (signed_val < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
            checksum += 2;
        }
        
        // Complex condition mixing signed and unsigned aspects
        // This should trigger the exact uncovered block with sgt/ugt comparisons
        wide_int_t temp = signed_val;
        if (shift1 > 16) {
            temp = temp >> (shift1 & 15);
        }
        
        // Simulate the a_high/a_low comparison structure
        // by comparing different parts of the value
        if (signed_val > (wide_int_t)0x3FFFFFFFFFFFFFFFLL || 
            (signed_val == (wide_int_t)0x3FFFFFFFFFFFFFFFLL && 
             (uwide_int_t)temp > 0xFFFFFFFFFFFFFFFFULL)) {
            checksum += 4;
        }
        
        // PATH 2: Unsigned comparison
        if (unsigned_val > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 8;
        }
        
        // Mixed signed/unsigned comparison
        if ((wide_int_t)unsigned_val > (wide_int_t)0x7FFFFFFFFFFFFFFFLL &&
            (uwide_int_t)signed_val < 0x8000000000000000ULL) {
            checksum += 16;
        }
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
