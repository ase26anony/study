#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 wide_uint_t;
#define WIDE_SHIFT_MASK 63
#else
typedef long long wide_int_t;
typedef unsigned long long wide_uint_t;
#define WIDE_SHIFT_MASK 31
#endif

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add a modifier that could make the value negative
    shifted += (wide_int_t)modifier * 0x100000001LL;
    
    return shifted;
}

__attribute__((noinline))
wide_uint_t wide_uint_compute(int shift_amount, int modifier) {
    // Start with a positive value
    wide_uint_t base = 0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    wide_uint_t shifted = base >> shift_amount;
    
    // Mask to ensure non-negative for unsigned comparison
    shifted &= (wide_uint_t)0x7FFFFFFFFFFFFFFFULL;
    
    // Add modifier
    shifted += (wide_uint_t)modifier * 0x80000000ULL;
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts
        int shift1 = (seed + i * 3) & WIDE_SHIFT_MASK;
        int shift2 = (seed + i * 5) & (WIDE_SHIFT_MASK / 2);
        int mod1 = (seed + i * 7) & 0xFF;
        int mod2 = (seed + i * 11) & 0x7F;
        
        // Compute wide integer values
        wide_int_t signed_val = wide_int_compute(shift1, mod1);
        wide_uint_t unsigned_val = wide_uint_compute(shift2, mod2);
        
        // PATH 1: Signed comparison that should trigger max_r/max_s initialization
        // This compares against a large positive value that requires double-int analysis
        if (signed_val > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // PATH 2: Additional signed comparison with negative bound
        // This may trigger min_r/min_s initialization
        if (signed_val < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL)) {
            checksum += 2;
        }
        
        // PATH 3: Mixed signed/unsigned comparison in complex condition
        // This mimics the "a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))" pattern
        wide_int_t temp = signed_val + (wide_int_t)i;
        if (temp > (wide_int_t)0x3FFFFFFFFFFFFFFFLL || 
            (temp == (wide_int_t)0x3FFFFFFFFFFFFFFFLL && 
             (wide_uint_t)temp > 0xFFFFFFFFULL)) {
            checksum += 4;
        }
        
        // PATH 4: Unsigned comparison
        if (unsigned_val > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 8;
        }
        
        // PATH 5: Another complex condition with arithmetic
        wide_int_t shifted_again = signed_val << ((shift1 + 1) & WIDE_SHIFT_MASK);
        if (shifted_again > 0 || (shifted_again == 0 && unsigned_val > 0)) {
            checksum += 16;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
