#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Prevent inlining to ensure complex operations aren't simplified
__attribute__((noinline))
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add a modifier that can make the value negative
    shifted += (__int128)modifier * 0x100000001LL;
    
    // Complex transformation mixing signed and unsigned aspects
    if (modifier & 1) {
        // For odd modifiers, perform additional arithmetic
        shifted = (shifted * 3) / 2;
    } else {
        // For even modifiers, mask to ensure non-negative
        shifted = shifted & 0x7FFFFFFFFFFFFFFFLL;
    }
    
    return shifted;
}

__attribute__((noinline))
long long wide_ll_compute(int shift_amount, int modifier) {
    // Similar logic with long long to ensure 64-bit double-int handling
    long long base = 0x123456789ABCDEF0LL;
    
    // Variable shift that can exceed 32 bits on 32-bit targets
    long long shifted = base << (shift_amount & 63);
    
    // Create signed/unsigned ambiguity
    if (modifier > 0) {
        shifted += (long long)modifier * 0x80000000LL;
    } else {
        shifted -= (long long)(-modifier) * 0x80000000LL;
    }
    
    // Conditional transformation affecting range
    if (shift_amount > 16) {
        shifted = (shifted >> 4) | (shifted << 60);  // Rotate
    }
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate many different cases
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant shift amounts
        int shift1 = (seed + i) & 63;      // 0-63 bits
        int shift2 = (seed * i) & 31;      // 0-31 bits
        int mod1 = (seed ^ i) & 0xFF;      // 0-255
        int mod2 = (i * 1103515245 + 12345) & 0x7F; // 0-127
        
        // Compute with __int128 (double-int on 64-bit, quad-int on 32-bit)
        __int128 result128 = wide_int_compute(shift1, mod1);
        
        // Compute with long long (double-int on 32-bit targets)
        long long result64 = wide_ll_compute(shift2, mod2);
        
        // SIGNED COMPARISON PATH - triggers sgt comparison
        // Compare against large positive signed value
        if (result128 > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            // This should trigger signed greater-than comparison
            checksum += 1;
        }
        
        // MIXED SIGNED/UNSIGNED COMPARISON - triggers both sgt and ugt
        // First check signed, then unsigned in same expression
        if (result128 > (__int128)0x3FFFFFFFFFFFFFFFLL ||
            (result128 == (__int128)0x3FFFFFFFFFFFFFFFLL &&
             (unsigned __int128)result128 > 0xFFFFFFFFULL)) {
            // This matches the pattern: a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
            checksum += 2;
        }
        
        // UNSIGNED COMPARISON PATH - triggers ugt comparison
        // Ensure value is non-negative for unsigned comparison
        unsigned __int128 unsigned_val = result128 & 0xFFFFFFFFFFFFFFFFULL;
        if (unsigned_val > 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 4;
        }
        
        // Additional complexity: nested conditions affecting range analysis
        if (shift1 > 32) {
            // When shifting many bits, compare against different bounds
            if (result64 > 0) {
                checksum += 8;
            } else if ((unsigned long long)result64 > 0x8000000000000000ULL) {
                checksum += 16;
            }
        } else {
            // Different comparison for small shifts
            if (result64 < -0x100000000LL) {
                checksum += 32;
            }
        }
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
