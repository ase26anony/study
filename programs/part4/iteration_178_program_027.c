#include <stdio.h>
#include <stdint.h>

// Prevent inlining to ensure the analysis runs on the actual operations
__attribute__((noinline)) 
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)(-1000);
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add a modifier that could make the value positive or negative
    __int128 result = shifted + (__int128)modifier * 1000000LL;
    
    return result;
}

__attribute__((noinline))
unsigned __int128 unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with a positive value for unsigned analysis
    unsigned __int128 base = 0x123456789ABCDEFULL;
    
    // Non-constant shift - forces range analysis
    unsigned __int128 shifted = base << shift_amount;
    
    // Add a modifier
    unsigned __int128 result = shifted + (unsigned __int128)modifier * 1000ULL;
    
    return result;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift amounts
        int shift1 = (seed + i) & 63;      // 0-63 bits for __int128
        int shift2 = (seed * i) & 63;
        int mod1 = (seed + i * 3) & 255;   // 0-255
        int mod2 = (seed * i + 5) & 255;
        
        // Compute with signed wide integer
        __int128 signed_result = wide_int_compute(shift1, mod1);
        
        // Compute with unsigned wide integer  
        unsigned __int128 unsigned_result = unsigned_wide_compute(shift2, mod2);
        
        // PATH 1: Signed comparison that should trigger the target block
        // Compare against large positive and negative bounds
        if (signed_result > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        if (signed_result < (__int128)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
            checksum += 2;
        }
        
        // Mixed signed/unsigned comparison in conditional expression
        // This mimics the structure in the uncovered code
        if (signed_result > 0 || 
            ((unsigned __int128)signed_result > 0xFFFFFFFFFFFFFFFFULL)) {
            checksum += 4;
        }
        
        // PATH 2: Complex conditional similar to the uncovered code
        // Compare high and low parts separately
        __int128 a_high = signed_result >> 64;
        unsigned __int128 a_low = (unsigned __int128)signed_result & 0xFFFFFFFFFFFFFFFFULL;
        
        // This structure should trigger the max_r/min_s initialization
        if (a_high > 0 || 
            (a_high == 0 && a_low > 0x7FFFFFFFFFFFFFFFULL)) {
            checksum += 8;
        }
        
        // PATH 3: Unsigned comparisons
        if (unsigned_result > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 16;
        }
        
        if ((unsigned_result >> shift1) > 0x7FFFFFFFFFFFFFFFULL) {
            checksum += 32;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
