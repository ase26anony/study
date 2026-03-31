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

// Prevent inlining to ensure arithmetic isn't optimized away before analysis
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_count, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Make shift_count bounded but non-constant
    shift_count = shift_count & (WIDE_SHIFT_MAX - 1);
    
    // Complex transformation: shift then add/subtract
    wide_int_t shifted = base << shift_count;
    
    // Add or subtract based on modifier, creating signed range complexity
    if (modifier & 1) {
        shifted = shifted + (wide_int_t)(modifier * 0x1001);
    } else {
        shifted = shifted - (wide_int_t)(modifier * 0x1001);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value) {
    // Signed comparison path - triggers sgt comparison
    // Compare against large positive signed value
#ifdef __SIZEOF_INT128__
    wide_int_t max_signed = ((wide_int_t)1 << 127) - 1;
#else
    wide_int_t max_signed = 0x7FFFFFFFFFFFFFFFLL;
#endif
    
    if (value > max_signed) {
        return 1;
    }
    
    // Also check negative range
    if (value < -max_signed / 2) {
        return -1;
    }
    
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value) {
    // Unsigned comparison path - triggers ugt comparison
    // Compare against max unsigned value
#ifdef __SIZEOF_INT128__
    uwide_int_t max_unsigned = ~(uwide_int_t)0;
#else
    uwide_int_t max_unsigned = 0xFFFFFFFFFFFFFFFFULL;
#endif
    
    if (value > max_unsigned / 2) {
        return 1;
    }
    
    return 0;
}

int main() {
    volatile int seed = 0x1234;  // Prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to create multiple analysis paths
    for (int i = 0; i < 1000; i++) {
        // Create bounded but non-constant shift counts
        int shift1 = (seed + i) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed * i + 12345) & (WIDE_SHIFT_MAX - 1);
        
        // Compute wide integer values with complex transformations
        wide_int_t val1 = wide_int_compute(shift1, i);
        wide_int_t val2 = wide_int_compute(shift2, i + 1);
        
        // Mixed signed/unsigned comparisons in conditional expressions
        int signed_check = 0;
        int unsigned_check = 0;
        
        // Complex conditional with both signed and unsigned comparisons
        // This encourages the compiler to evaluate multiple range constraints
        if (val1 > 0) {
            // Signed comparison
            signed_check = check_signed_range(val1);
            
            // Also check after arithmetic operation
            wide_int_t val3 = val1 + val2;
            if (val3 > val1 && val3 < 0) {  // Mixed conditions
                signed_check += 1;
            }
        }
        
        // Convert to unsigned for unsigned comparison
        uwide_int_t uval1 = (uwide_int_t)val1;
        uwide_int_t uval2 = (uwide_int_t)val2;
        
        // Unsigned comparison with logical operations
        if ((uval1 & 0xFFFFFFFF) > (uval2 & 0xFFFFFFFF)) {
            unsigned_check = check_unsigned_range(uval1);
        }
        
        // Combine results in a way that prevents optimization
        checksum += (unsigned long long)signed_check;
        checksum += (unsigned long long)unsigned_check;
        checksum += (uval1 & 0xFF) + (uval2 & 0xFF);
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Additional control flow that influences value ranges
        if (i % 3 == 0) {
            // Create dependency between shift count and comparison
            int temp_shift = (i & 31);
            wide_int_t temp = (wide_int_t)0x8000000000000000LL >> temp_shift;
            if (temp > 0 && temp < 100) {  // Impossible condition, but creates analysis complexity
                checksum += 1;
            }
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
