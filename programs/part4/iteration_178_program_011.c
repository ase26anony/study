#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 wide_uint_t;
#define WIDE_INT_MAX (((wide_int_t)1 << 126) - 1 + ((wide_int_t)1 << 126))
#define WIDE_UINT_MAX ((wide_uint_t)-1)
#else
typedef long long wide_int_t;
typedef unsigned long long wide_uint_t;
#define WIDE_INT_MAX 0x7FFFFFFFFFFFFFFFLL
#define WIDE_UINT_MAX 0xFFFFFFFFFFFFFFFFULL
#endif

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier, int use_signed) {
    volatile wide_int_t base;
    
    // Non-constant base value that depends on inputs
    if (use_signed) {
        // Create potentially negative values for signed analysis
        base = (wide_int_t)(modifier - 100) * 1000000007LL;
    } else {
        // Create non-negative values for unsigned analysis
        base = (wide_int_t)modifier * 1000000007LL;
    }
    
    // Complex transformation with non-constant shift
    // This forces range analysis to track possible values
    wide_int_t shifted = base << (shift_amount & 63);
    
    // Additional arithmetic to create more complex range
    wide_int_t result = shifted + (wide_int_t)(shift_amount * 123456789);
    
    // Conditional transformation based on shift amount
    if (shift_amount & 1) {
        result = result - (wide_int_t)(modifier * 987654321);
    } else {
        result = result + (wide_int_t)(modifier * 555555555);
    }
    
    return result;
}

__attribute__((noinline))
int analyze_ranges(wide_int_t value, int path_selector) {
    int checksum = 0;
    
    // Mixed signed/unsigned comparisons that should trigger the target code
    if (path_selector & 1) {
        // Signed comparison path - triggers sgt comparison
        if (value > (WIDE_INT_MAX >> 2)) {
            checksum += 1;
        }
        
        // Another signed comparison with different bound
        if (value < -(WIDE_INT_MAX >> 3)) {
            checksum += 2;
        }
    }
    
    if (path_selector & 2) {
        // Unsigned comparison path - triggers ugt comparison
        wide_uint_t uval = (wide_uint_t)value;
        
        // Complex unsigned comparison with shifted bounds
        if (uval > (WIDE_UINT_MAX >> 4)) {
            checksum += 4;
        }
        
        // Additional unsigned comparison
        if ((uval & (WIDE_UINT_MAX >> 8)) == 0) {
            checksum += 8;
        }
    }
    
    // Combined condition similar to the uncovered code's structure
    if ((value > (WIDE_INT_MAX >> 5)) || 
        ((wide_uint_t)value > (WIDE_UINT_MAX >> 6))) {
        checksum += 16;
    }
    
    return checksum;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    int total_checksum = 0;
    
    // Loop with varying inputs to explore different paths
    for (int i = 0; i < 10000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & 63;      // 0-63 range
        int shift2 = (seed * i) & 31;      // 0-31 range
        int modifier = (seed + i * 3) & 255; // 0-255 range
        
        // Alternate between signed and unsigned analysis
        int use_signed = i & 1;
        
        // Compute wide integer values with complex transformations
        wide_int_t val1 = wide_int_compute(shift1, modifier, use_signed);
        wide_int_t val2 = wide_int_compute(shift2, modifier ^ 0xAA, !use_signed);
        
        // Perform arithmetic that creates dependency between values
        wide_int_t combined = val1 + (val2 >> (shift1 & 15));
        
        // Additional transformation to create more analysis complexity
        if (i & 2) {
            combined = combined << ((shift2 + 1) & 31);
        } else {
            combined = combined >> ((shift1 + 1) & 31);
        }
        
        // Analyze with different path selectors
        int path_selector = (seed + i) & 3;
        total_checksum += analyze_ranges(combined, path_selector);
        
        // Also analyze the individual values
        total_checksum += analyze_ranges(val1, (path_selector + 1) & 3);
        total_checksum += analyze_ranges(val2, (path_selector + 2) & 3);
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Prevent loop unrolling from simplifying too much
        if (i % 100 == 0) {
            asm volatile("" : "+r"(total_checksum) : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
