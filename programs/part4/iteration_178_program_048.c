#include <stdio.h>
#include <stdint.h>

// Prevent inlining to ensure the analysis sees complex operations
__attribute__((noinline))
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add a modifier that could make it negative
    shifted += (__int128)modifier * 0x100000001LL;
    
    // Complex transformation mixing signed and unsigned
    if (modifier & 1) {
        // For odd modifiers, perform arithmetic right shift
        shifted = shifted >> (shift_amount & 0x3F);
    } else {
        // For even modifiers, perform logical right shift
        shifted = (unsigned __int128)shifted >> (shift_amount & 0x3F);
    }
    
    return shifted;
}

__attribute__((noinline))
long long wide_ll_compute(int shift_amount, int modifier) {
    // Similar logic with 64-bit types
    long long base = 0x123456789ABCDEF0LL;
    
    // Non-constant shift that can overflow into negative
    long long shifted = base << (shift_amount & 0x3F);
    
    // Add/subtract based on modifier
    if (modifier > 0) {
        shifted += (long long)modifier * 0x100000001LL;
    } else {
        shifted -= (long long)(-modifier) * 0x100000001LL;
    }
    
    // Conditional transformation
    if (shift_amount & 1) {
        // Mask to ensure non-negative for unsigned comparison
        shifted &= 0x7FFFFFFFFFFFFFFFLL;
    }
    
    return shifted;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant values
        int shift1 = (seed + i) & 0x3F;      // 0-63 bits
        int shift2 = (seed * i) & 0x3F;
        int mod1 = (seed + i * 3) & 0xFF;    // 0-255
        int mod2 = (seed - i * 5) & 0xFF;
        
        // Compute with 128-bit types
        __int128 result128 = wide_int_compute(shift1, mod1);
        
        // PATH 1: Signed comparison (sgt in the target code)
        // Compare against large positive signed value
        if (result128 > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative bound
        if (result128 < (__int128)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
            checksum += 2;
        }
        
        // PATH 2: Unsigned comparison (ugt in the target code)
        // Compare against max unsigned value
        if ((unsigned __int128)result128 > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 4;
        }
        
        // Mixed signed/unsigned comparison in conditional expression
        // This mimics the (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))) pattern
        if (result128 > 0 || (result128 == 0 && (unsigned __int128)result128 > 0)) {
            checksum += 8;
        }
        
        // Compute with 64-bit types (treated as double-int on 32-bit)
        long long result64 = wide_ll_compute(shift2, mod2);
        
        // More comparisons to trigger range analysis
        if (result64 > 0x3FFFFFFFFFFFFFFFLL) {
            checksum += 16;
        }
        
        if ((unsigned long long)result64 > 0x7FFFFFFFFFFFFFFFULL) {
            checksum += 32;
        }
        
        // Complex conditional with both signed and unsigned parts
        if (result64 > 0x100000000LL || (result64 == 0x100000000LL && 
            (unsigned long long)result64 > 0xFFFFFFFFULL)) {
            checksum += 64;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
