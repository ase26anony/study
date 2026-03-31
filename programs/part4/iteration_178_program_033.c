#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Prevent inlining to ensure complex analysis happens
__attribute__((noinline))
long long wide_int_compute_signed(int shift_amt, int add_val) {
    // Start with a value that can become negative when shifted
    long long base = 0x123456789ABCDEFLL;
    
    // Non-constant shift - forces range analysis
    long long shifted = base << shift_amt;
    
    // Addition that could cause overflow
    long long result = shifted + (long long)add_val;
    
    return result;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amt, int mask_val) {
    // Start with positive value
    unsigned long long base = 0xFEDCBA987654321ULL;
    
    // Non-constant shift
    unsigned long long shifted = base << shift_amt;
    
    // Mask operation - keeps value non-negative
    unsigned long long result = shifted & (0xFFFFFFFFFFFFFFFFULL >> mask_val);
    
    return result;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_compute_128bit(int shift_amt, int sign_control) {
    // Use 128-bit type for double-int analysis
    __int128 base = (__int128)0x123456789ABCDEFLL;
    
    // Make base negative or positive based on control
    if (sign_control & 1) {
        base = -base;
    }
    
    // Complex transformation with non-constant shift
    __int128 shifted = base << (shift_amt & 63);  // Limit to 0-63 bits
    
    // Arithmetic that creates range uncertainty
    __int128 result = shifted + (shift_amt * 0x100000001LL);
    
    return result;
}
#endif

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis contexts
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant values
        int shift1 = (seed + i) & 31;      // 0-31 bits
        int shift2 = (seed * i) & 31;      // Another bounded shift
        int add_val = (seed + i * 3) & 0xFF;
        int mask_val = (seed + i * 5) & 63;
        
        // Path 1: Signed comparison with potential negative values
        long long signed_result = wide_int_compute_signed(shift1, add_val);
        
        // This comparison should trigger signed range analysis
        if (signed_result > 0x7FFFFFFFFFFFFFFFLL) {
            // Exceeds max positive signed 64-bit
            checksum += 1;
        } else if (signed_result < -0x7FFFFFFFFFFFFFFFLL) {
            // Very negative value
            checksum += 2;
        }
        
        // Path 2: Unsigned comparison (always non-negative)
        unsigned long long unsigned_result = wide_int_compute_unsigned(shift2, mask_val);
        
        // Mixed signed/unsigned comparison context
        if ((unsigned_result > 0xFFFFFFFFFFFFFFFULL) ||
            ((long long)unsigned_result > 0 && (long long)unsigned_result < 1000)) {
            checksum += 4;
        }
        
        // Complex conditional that forces analysis of both bounds
        if (signed_result > 0 && (unsigned long long)signed_result < 0x8000000000000000ULL) {
            checksum += 8;
        }
        
#ifdef __SIZEOF_INT128__
        // Path 3: 128-bit operations for double-int analysis
        int sign_ctrl = (seed + i) & 3;
        __int128 result_128 = wide_int_compute_128bit(shift1, sign_ctrl);
        
        // Comparisons that should trigger the exact uncovered block
        // These use both signed and unsigned comparisons on wide integers
        
        // Create comparison against maximum bound
        __int128 max_bound = ((__int128)0x7FFFFFFFFFFFFFFFLL) << 32;
        
        if (result_128 > max_bound) {
            checksum += 16;
        }
        
        // Create comparison against minimum bound
        __int128 min_bound = -(((__int128)0x7FFFFFFFFFFFFFFFLL) << 32);
        
        if (result_128 < min_bound) {
            checksum += 32;
        }
        
        // Mixed comparison like in the uncovered code
        unsigned __int128 unsigned_128 = (unsigned __int128)result_128;
        if (result_128 > 0 || unsigned_128 > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 64;
        }
#endif
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional complexity: shift amount depends on previous comparison
        if (checksum & 1) {
            shift1 = (shift1 + 1) & 31;
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    // Prevent dead code elimination
    volatile unsigned long long final = checksum;
    return (int)(final & 0x7FFFFFFF);
}
