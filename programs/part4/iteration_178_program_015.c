#include <stdio.h>
#include <stdint.h>

// Prevent constant propagation and inlining
__attribute__((noinline))
long long wide_int_compute_signed(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    long long base = 0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    long long shifted = base << shift_amount;
    
    // Add a modifier that could make it positive or negative
    // Use modulo to keep it bounded but non-constant
    long long result = shifted + (modifier * 0x1000LL);
    
    return result;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amount, int modifier) {
    // Start with a positive value
    unsigned long long base = 0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    unsigned long long shifted = base << shift_amount;
    
    // Add modifier - could overflow
    unsigned long long result = shifted + (modifier * 0x1000ULL);
    
    return result;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_compute_128bit(int shift_amount, int modifier) {
    // Use 128-bit type for double-int analysis
    __int128 base = (__int128)0x123456789ABCDEF0LL << 64;
    base |= 0xFEDCBA9876543210LL;
    
    // Complex transformation with non-constant shift
    __int128 shifted = base << (shift_amount & 63);
    
    // Arithmetic that could change sign
    __int128 result = shifted + ((__int128)modifier << 32);
    
    return result;
}

__attribute__((noinline))
unsigned __int128 wide_int_compute_unsigned_128bit(int shift_amount, int modifier) {
    unsigned __int128 base = (unsigned __int128)0xFEDCBA9876543210ULL << 64;
    base |= 0x123456789ABCDEF0ULL;
    
    unsigned __int128 shifted = base << (shift_amount & 63);
    unsigned __int128 result = shifted + ((unsigned __int128)modifier << 40);
    
    return result;
}
#endif

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & 63;      // 0-63 bits
        int shift2 = (seed * i) & 31;      // 0-31 bits
        int mod1 = (seed + i * 3) & 0xFF;  // 0-255
        int mod2 = (seed * i + 123) & 0x7F; // 0-127
        
        // Path 1: Signed 64-bit comparison
        long long result_signed = wide_int_compute_signed(shift1, mod1);
        
        // This comparison should trigger signed range analysis
        // The constant 0x7FFFFFFFFFFFFFFF is max positive signed 64-bit
        if (result_signed > 0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        // Additional signed comparison with negative bound
        if (result_signed < -0x7FFFFFFFFFFFFFFFLL) {
            checksum += 2;
        }
        
        // Path 2: Unsigned 64-bit comparison  
        unsigned long long result_unsigned = wide_int_compute_unsigned(shift2, mod2);
        
        // Mixed signed/unsigned comparison scenario
        // First check if it could be negative when interpreted as signed
        if ((long long)result_unsigned > 0x3FFFFFFFFFFFFFFFLL) {
            checksum += 4;
        }
        
        // Unsigned comparison against large value
        if (result_unsigned > 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 8;
        }
        
        // Complex condition with both signed and unsigned parts
        // This mimics the structure in the uncovered code
        if ((result_signed > 0x3FFFFFFFFFFFFFFFLL) ||
            ((long long)result_signed == 0x3FFFFFFFFFFFFFFFLL &&
             result_unsigned > 0xFFFFFFFFFFFFFFF0ULL)) {
            checksum += 16;
        }
        
#ifdef __SIZEOF_INT128__
        // Path 3: 128-bit signed operations
        __int128 result_128 = wide_int_compute_128bit(shift1, mod1);
        
        // Compare against max positive 128-bit value
        __int128 max_signed_128 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFLL;
        if (result_128 > max_signed_128) {
            checksum += 32;
        }
        
        // Compare against min negative 128-bit value
        __int128 min_signed_128 = ((__int128)0x8000000000000000LL << 64);
        if (result_128 < min_signed_128) {
            checksum += 64;
        }
        
        // Path 4: 128-bit unsigned operations
        unsigned __int128 result_u128 = wide_int_compute_unsigned_128bit(shift2, mod2);
        
        // Max unsigned 128-bit value
        unsigned __int128 max_unsigned_128 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
        
        // Mixed comparison: first signed, then unsigned
        if ((__int128)result_u128 > (__int128)0 ||
            ((__int128)result_u128 == (__int128)0 &&
             result_u128 > (max_unsigned_128 >> 1))) {
            checksum += 128;
        }
#endif
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
