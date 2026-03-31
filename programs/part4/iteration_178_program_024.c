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
__attribute__((noinline, noipa))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a non-constant base value
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Use volatile to prevent constant propagation
    volatile int vol_shift = shift_amount;
    volatile int vol_mod = modifier;
    
    // Complex transformation with non-constant shift
    wide_int_t shifted = base << vol_shift;
    
    // Add/subtract based on modifier
    if (vol_mod & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x1111111111111111LL;
    }
    
    // Additional conditional transformation
    if (vol_mod & 2) {
        shifted = ~shifted;
    }
    
    return shifted;
}

__attribute__((noinline, noipa))
int check_signed_range(wide_int_t value) {
    // Signed comparison - triggers sgt comparison
    wide_int_t max_signed = (wide_int_t)1 << (sizeof(wide_int_t) * 8 - 2);
    if (value > max_signed) {
        return 1;
    }
    
    // Another signed comparison with negative bound
    wide_int_t min_signed = -(max_signed >> 1);
    if (value < min_signed) {
        return -1;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison - triggers ugt comparison
    uwide_int_t threshold = (uwide_int_t)0xFFFFFFFFFFFFFFFFULL;
    
    // Complex conditional with mixed comparisons
    if (value > threshold || 
        (value > (threshold >> 1) && value < (threshold >> 2))) {
        return 1;
    }
    
    return 0;
}

int main() {
    volatile int seed = 0xDEADBEEF;
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis paths
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i * 3) & WIDE_SHIFT_MASK;
        int shift2 = (seed ^ i * 5) & (WIDE_SHIFT_MASK - 1);
        
        // Vary the modifier to create different paths
        int modifier = (seed + i * 7) & 0xF;
        
        // Compute with wide integers
        wide_int_t result1 = wide_int_compute(shift1, modifier);
        wide_int_t result2 = wide_int_compute(shift2, modifier ^ 0xA);
        
        // Perform signed range checks
        int signed_check1 = check_signed_range(result1);
        int signed_check2 = check_signed_range(result2 + (i & 0xFF));
        
        // Perform unsigned range checks
        uwide_int_t uresult1 = (uwide_int_t)result1;
        uwide_int_t uresult2 = (uwide_int_t)result2;
        
        // Conditional that mixes signed and unsigned comparisons
        if ((signed_check1 > 0 && (uresult1 > 0x7FFFFFFFFFFFFFFFULL)) ||
            (signed_check2 < 0 && (uresult2 < 0x8000000000000000ULL))) {
            checksum += 1;
        }
        
        // Additional unsigned comparison path
        if (check_unsigned_range(uresult1) || 
            check_unsigned_range(uresult2 >> (shift1 & 0x1F))) {
            checksum += 2;
        }
        
        // Complex conditional that requires analyzing both bounds
        wide_int_t combined = result1 - result2;
        if (combined > ((wide_int_t)1 << (shift2 & 0x3F)) &&
            (uwide_int_t)combined < (0xFFFFFFFFULL << (shift1 & 0x1F))) {
            checksum += 4;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
