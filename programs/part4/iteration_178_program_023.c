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
    
    // Add a modifier that could make it positive or negative
    long long result = shifted + (modifier * 0x1000LL);
    
    return result;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amount, int modifier) {
    // Start with a positive value
    unsigned long long base = 0xFEDCBA9876543210ULL;
    
    // Non-constant shift - forces range analysis
    unsigned long long shifted = base << shift_amount;
    
    // Add modifier (keeps it positive)
    unsigned long long result = shifted + (modifier * 0x1000ULL);
    
    return result;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_compute_128bit(int shift_amount, int modifier) {
    // Use 128-bit type for more complex double-int analysis
    __int128 base = ((__int128)0x123456789ABCDEF0LL << 64) | 0xFEDCBA9876543210LL;
    
    // Non-constant shift on 128-bit value
    __int128 shifted = base << shift_amount;
    
    // Complex arithmetic that could overflow
    __int128 result = shifted + ((__int128)modifier << 32);
    
    return result;
}

__attribute__((noinline))
unsigned __int128 wide_int_compute_unsigned_128bit(int shift_amount, int modifier) {
    unsigned __int128 base = ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL;
    
    // Non-constant shift
    unsigned __int128 shifted = base << shift_amount;
    
    // Arithmetic that stays in positive range
    unsigned __int128 result = shifted + ((unsigned __int128)modifier << 16);
    
    return result;
}
#endif

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & 63;      // 0-63 bits for 64-bit types
        int shift2 = (seed * i) & 31;      // 0-31 bits
        int mod1 = (seed ^ i) & 0xFF;      // 0-255
        int mod2 = (i * 1103515245 + 12345) & 0xFFFF; // 0-65535
        
        // 64-bit signed computations
        long long signed_result = wide_int_compute_signed(shift1, mod1);
        
        // Trigger signed comparison (sgt in the target code)
        if (signed_result > 0x7FFFFFFFFFFFFFFFLL) {
            // This should trigger max_r/max_s initialization for signed comparison
            checksum += 1;
        }
        
        // Complex condition with && and || like in the target
        if (signed_result > 0x3FFFFFFFFFFFFFFFLL || 
            (signed_result < 0 && signed_result > -0x4000000000000000LL)) {
            checksum += 2;
        }
        
        // 64-bit unsigned computations
        unsigned long long unsigned_result = wide_int_compute_unsigned(shift2, mod2);
        
        // Trigger unsigned comparison (ugt in the target code)
        if (unsigned_result > 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 4;
        }
        
        // Mixed signed/unsigned comparison
        if ((long long)unsigned_result > signed_result && 
            unsigned_result < 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 8;
        }
        
#ifdef __SIZEOF_INT128__
        // 128-bit computations - more likely to trigger double-int logic
        __int128 result_128 = wide_int_compute_128bit(shift1, mod1);
        
        // Complex 128-bit signed comparison
        __int128 max_signed_128 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFLL;
        if (result_128 > max_signed_128) {
            checksum += 16;
        }
        
        // Another condition that might trigger min_r/min_s initialization
        __int128 min_threshold = ((__int128)0x8000000000000000LL << 64);
        if (result_128 < min_threshold || result_128 > 0) {
            checksum += 32;
        }
        
        // 128-bit unsigned
        unsigned __int128 unsigned_128 = wide_int_compute_unsigned_128bit(shift2, mod2);
        
        // Unsigned 128-bit comparison
        unsigned __int128 max_unsigned_128 = ~((unsigned __int128)0);
        if (unsigned_128 > (max_unsigned_128 >> 1)) {
            checksum += 64;
        }
        
        // Mixed 128-bit comparison
        if ((__int128)unsigned_128 > result_128 && 
            unsigned_128 < max_unsigned_128) {
            checksum += 128;
        }
#endif
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
