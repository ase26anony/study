#include <stdio.h>
#include <stdint.h>

// Prevent inlining to ensure complex arithmetic isn't simplified
__attribute__((noinline))
__int128 wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    __int128 shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (__int128)0x7FFFFFFFFFFFFFFFLL;
    } else {
        shifted -= (__int128)0x7FFFFFFFFFFFFFFFLL;
    }
    
    // Multiply to expand range further
    shifted *= (modifier & 3) + 1;
    
    return shifted;
}

__attribute__((noinline))
unsigned long long mixed_comparisons(__int128 value, int i_f_bits) {
    unsigned long long checksum = 0;
    
    // Create signed comparison scenario
    // This should trigger signed greater-than comparison (sgt)
    if (value > (__int128)0x7FFFFFFFFFFFFFFFLL) {
        checksum += 1;
    }
    
    // Create unsigned comparison scenario
    // Force unsigned comparison by masking
    unsigned __int128 unsigned_val = (unsigned __int128)value;
    unsigned __int128 mask = ((unsigned __int128)1 << i_f_bits) - 1;
    unsigned_val &= mask;
    
    // This should trigger unsigned greater-than comparison (ugt)
    if (unsigned_val > (unsigned __int128)0xFFFFFFFFFFFFFFFFULL) {
        checksum += 2;
    }
    
    // Complex conditional with both signed and unsigned comparisons
    // This mimics the structure in the uncovered code
    __int128 max_r = 0;
    __int128 max_s = -1;
    max_s = max_s << i_f_bits;
    
    __int128 min_r = -1;
    __int128 min_s = 1;
    min_s = min_s << i_f_bits;
    min_s = (min_s << 1) >> 1;  // Simulate sext operation
    
    // Direct replication of the comparison pattern from uncovered lines
    if (value > max_r || (value == max_r && unsigned_val > (unsigned __int128)max_s)) {
        checksum += 4;
    }
    
    return checksum;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    unsigned long long total_checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts (0-63)
        int shift = (seed + i) & 63;
        
        // Generate bounded modifier (0-7)
        int modifier = (seed * i) & 7;
        
        // Generate i_f_bits parameter (1-127)
        int i_f_bits = ((seed + i * 3) & 127) + 1;
        
        // Compute wide integer with complex transformations
        __int128 result = wide_int_compute(shift, modifier);
        
        // Perform mixed signed/unsigned comparisons
        total_checksum += mixed_comparisons(result, i_f_bits);
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    // Print result to prevent dead code elimination
    printf("Final checksum: %llu\n", total_checksum);
    
    return 0;
}
