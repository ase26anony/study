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
__attribute__((noinline, noipa))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_amount bounded but non-constant for analysis
    shift_amount = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation: shift then add/subtract
    wide_int_t shifted = base << shift_amount;
    
    // Create both positive and negative possibilities
    if (modifier & 1) {
        shifted = shifted + (wide_int_t)(modifier * 1000);
    } else {
        shifted = shifted - (wide_int_t)(modifier * 500);
    }
    
    return shifted;
}

__attribute__((noinline, noipa))
int check_signed_range(wide_int_t value) {
    // Signed comparison path - will trigger sgt comparison
    wide_int_t max_signed = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
    if (value > max_signed) {
        return 1;
    }
    
    // Additional signed comparison with negative bound
    wide_int_t min_signed = -((wide_int_t)1 << (WIDE_SHIFT_MAX - 1));
    if (value < min_signed) {
        return -1;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison path - will trigger ugt comparison
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
    
    // Complex conditional with mixed comparisons
    if (value > max_unsigned / 2) {
        if (value == max_unsigned) {
            return 2;
        }
        return 1;
    }
    
    return 0;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    int checksum = 0;
    
    // Loop to create multiple analysis paths
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant inputs
        int shift = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int mod = (seed * i) & 0xFF;
        
        // Get wide integer result
        wide_int_t result = wide_int_compute(shift, mod);
        
        // Path 1: Signed comparison (triggers sgt)
        int signed_check = check_signed_range(result);
        checksum += signed_check;
        
        // Path 2: Unsigned comparison (triggers ugt)
        // Convert to unsigned for unsigned comparison
        uwide_int_t uresult = (uwide_int_t)result;
        
        // Additional transformation to create analysis complexity
        if (mod & 0x1) {
            uresult = uresult >> (shift & 0x3F);
        } else {
            uresult = uresult << ((mod & 0x3) + 1);
        }
        
        int unsigned_check = check_unsigned_range(uresult);
        checksum += unsigned_check;
        
        // Additional mixed comparison in main to encourage the specific pattern
        // This mimics the structure in the uncovered code
        wide_int_t temp = result;
        if (mod > 128) {
            temp = temp << (mod & 0x1F);
        }
        
        // Complex conditional with both signed and unsigned comparisons
        // Similar to: if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
        uwide_int_t utemp = (uwide_int_t)temp;
        wide_int_t stemp = temp;
        
        // Create conditions that require range analysis
        wide_int_t bound1 = (wide_int_t)1 << 60;
        uwide_int_t bound2 = (uwide_int_t)1 << 62;
        
        if (stemp > bound1 || ((uwide_int_t)stemp == bound1 && utemp > bound2)) {
            checksum += 3;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
