#include <stdio.h>
#include <stdint.h>

// Prevent inlining to ensure the analysis sees complex operations
__attribute__((noinline)) 
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add a modifier that could make it positive or negative
    // Use signed arithmetic to create potential negative values
    __int128 result = shifted + (__int128)modifier * 0x1000;
    
    return result;
}

__attribute__((noinline))
unsigned __int128 unsigned_wide_compute(int shift_amount, int mask) {
    // Start with unsigned value
    unsigned __int128 base = (unsigned __int128)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    unsigned __int128 shifted = base << shift_amount;
    
    // Apply mask to create bounded but non-constant range
    unsigned __int128 masked = shifted & ((unsigned __int128)mask * 0xFFFFFFFFULL);
    
    return masked;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts (0-63)
        int shift1 = (seed + i * 3) & 63;
        int shift2 = (seed + i * 5) & 63;
        int mod1 = (seed + i * 7) & 0xFF;
        int mod2 = (seed + i * 11) & 0xFF;
        
        // Get signed wide integer result
        __int128 signed_result = wide_int_compute(shift1, mod1);
        
        // Get unsigned wide integer result  
        unsigned __int128 unsigned_result = unsigned_wide_compute(shift2, mod2);
        
        // PATH 1: Signed comparison against large positive value
        // This should trigger signed greater-than (sgt) comparison
        if (signed_result > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // PATH 2: Mixed signed/unsigned comparison
        // First part: signed comparison (a_high.sgt(max_r))
        // Second part: unsigned comparison (a_low.ugt(max_s))
        if ((signed_result > (__int128)0x3FFFFFFFFFFFFFFFLL) ||
            ((signed_result == (__int128)0x3FFFFFFFFFFFFFFFLL) &&
             ((unsigned __int128)signed_result > 0xFFFFFFFFULL))) {
            checksum += 2;
        }
        
        // PATH 3: Complex condition with both signed and unsigned parts
        // This mimics the structure in the uncovered code
        __int128 a_high = signed_result >> 64;
        unsigned __int128 a_low = (unsigned __int128)signed_result & 0xFFFFFFFFFFFFFFFFULL;
        
        // Create conditions that require computing max_r/min_s bounds
        if (a_high > (__int128)0) {
            checksum += 4;
        }
        
        if ((a_high > (__int128)0) || 
            (a_high == (__int128)0 && a_low > 0x8000000000000000ULL)) {
            checksum += 8;
        }
        
        // PATH 4: Additional unsigned comparison path
        if ((unsigned __int128)(signed_result + 0x1000) > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 16;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
