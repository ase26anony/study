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

// Prevent inlining to ensure the analysis runs on the actual operations
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_amount bounded but non-constant
    int bounded_shift = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Perform a left shift that could overflow into sign bit
    wide_int_t shifted = base << bounded_shift;
    
    // Add/subtract based on modifier to create range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)(modifier * 0x10001LL);
    } else {
        shifted -= (wide_int_t)(modifier * 0xFFFFLL);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i) {
    // Create max_r/max_s like in the uncovered code
    // This comparison should trigger the signed greater-than check
    wide_int_t max_signed = ((wide_int_t)1 << (WIDE_SHIFT_MAX - 1)) - 1;
    
    // Complex condition with OR that might trigger both comparisons
    if (value > max_signed || (value == max_signed && i > 1000)) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i) {
    // Create min_r/min_s like in the uncovered code
    // This should trigger the unsigned comparison path
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
    
    // Mixed signed/unsigned comparison
    if (value > max_unsigned - (i & 0xFF)) {
        return 1;
    }
    return 0;
}

int main() {
    volatile int seed = 0xDEADBEEF;  // Prevent constant propagation
    uint64_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded but non-constant shift amounts
        int shift1 = (seed + i * 3) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed - i * 5) & (WIDE_SHIFT_MAX - 1);
        
        // Compute wide integers with complex transformations
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i ^ 0x5555);
        
        // Perform arithmetic that creates range analysis complexity
        wide_int_t result = val1 + val2;
        result = result << ((i & 7) + 1);  // Variable shift
        
        // Subtract to potentially create negative values
        result -= (wide_int_t)(i * 0x100000001LL);
        
        // Trigger signed comparison analysis
        if (check_signed_range(result, i)) {
            checksum += 1;
        }
        
        // Trigger unsigned comparison analysis
        uwide_int_t uresult = (uwide_int_t)result;
        // Apply mask to ensure non-negative for unsigned comparison
        uresult &= ~((uwide_int_t)1 << (WIDE_SHIFT_MAX - 1));
        
        if (check_unsigned_range(uresult, i)) {
            checksum += 2;
        }
        
        // Additional complex condition that mixes both
        wide_int_t temp = result >> ((i & 3) + 1);  // Logical right shift
        if (result > 0 && (uwide_int_t)temp > (uwide_int_t)(i * 0x1000)) {
            checksum += 4;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
