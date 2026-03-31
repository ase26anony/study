#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Prevent inlining to ensure complex analysis happens
__attribute__((noinline))
long long wide_int_compute_signed(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    long long base = 0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    long long shifted = base << shift_amount;
    
    // Add a modifier that could make it negative
    shifted += (modifier - 32) * 0x100000000LL;
    
    return shifted;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amount, int modifier) {
    // Start with a positive value
    unsigned long long base = 0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    unsigned long long shifted = base << shift_amount;
    
    // Add a positive modifier
    shifted += (modifier & 0xFF) * 0x1000ULL;
    
    return shifted;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_compute_128bit(int shift_amount, int modifier) {
    __int128 base = ((__int128)0x123456789ABCDEF0LL << 64) | 0xFEDCBA9876543210LL;
    
    // Complex transformation with non-constant shift
    __int128 shifted = base << (shift_amount & 63);
    
    // Add/subtract based on modifier
    if (modifier & 1) {
        shifted += ((__int128)modifier << 32);
    } else {
        shifted -= ((__int128)modifier << 32);
    }
    
    return shifted;
}

__attribute__((noinline))
unsigned __int128 wide_int_compute_unsigned_128bit(int shift_amount, int modifier) {
    unsigned __int128 base = ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL;
    
    // Non-constant shift
    unsigned __int128 shifted = base << (shift_amount & 63);
    
    // Mask to ensure non-negative
    shifted &= ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    return shifted;
}
#endif

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant values
        int shift1 = (seed + i) & 63;      // 0-63 bits
        int shift2 = (seed * i) & 31;      // 0-31 bits
        int mod1 = (seed + i * 3) & 0xFF;  // 0-255
        int mod2 = (seed - i * 5) & 0x7F;  // 0-127
        
        // Path 1: Signed 64-bit comparison
        long long result_signed = wide_int_compute_signed(shift1, mod1);
        
        // This comparison should trigger signed range analysis
        // The constant 0x7FFFFFFFFFFFFFFFLL is max positive signed 64-bit
        if (result_signed > 0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative bound
        if (result_signed < -0x7FFFFFFFFFFFFFFFLL) {
            checksum += 2;
        }
        
        // Path 2: Unsigned 64-bit comparison
        unsigned long long result_unsigned = wide_int_compute_unsigned(shift2, mod2);
        
        // This comparison should trigger unsigned range analysis
        if (result_unsigned > 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 4;
        }
        
        // Mixed signed/unsigned comparison in same expression
        // This encourages the compiler to evaluate multiple range constraints
        if ((result_signed > 0x3FFFFFFFFFFFFFFFLL) || 
            ((long long)result_unsigned < -0x4000000000000000LL)) {
            checksum += 8;
        }
        
        // Complex conditional that depends on previous comparisons
        int complex_shift = shift1;
        if (result_signed > 0) {
            complex_shift = shift2;
        }
        
        // Use variable shift count based on condition
        long long conditional_result = 0x12345678LL << complex_shift;
        if (conditional_result > 0x7FFFFFFFFFFFFFFFLL && 
            (unsigned long long)conditional_result > 0x8000000000000000ULL) {
            checksum += 16;
        }
        
#ifdef __SIZEOF_INT128__
        // Path 3: 128-bit signed operations
        __int128 result_128 = wide_int_compute_128bit(shift1, mod1);
        
        // Signed comparison for 128-bit
        __int128 max_signed_128 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFLL;
        if (result_128 > max_signed_128) {
            checksum += 32;
        }
        
        // Path 4: 128-bit unsigned operations
        unsigned __int128 result_unsigned_128 = wide_int_compute_unsigned_128bit(shift2, mod2);
        
        // Unsigned comparison for 128-bit
        unsigned __int128 max_unsigned_128 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
        if (result_unsigned_128 > (max_unsigned_128 >> 1)) {
            checksum += 64;
        }
        
        // Mixed 128-bit comparison
        if (result_128 > 0 || (unsigned __int128)result_128 > max_unsigned_128) {
            checksum += 128;
        }
#endif
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
