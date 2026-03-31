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

// Prevent inlining to ensure analysis happens
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_count, int modifier) {
    // Start with a value that can be negative or positive
    wide_int_t base = (wide_int_t)modifier * 0x123456789ABCDEFLL;
    
    // Non-constant shift - forces range analysis
    if (shift_count > WIDE_SHIFT_MAX / 2) {
        // Path 1: Large shift that could overflow
        wide_int_t shifted = base << (shift_count & (WIDE_SHIFT_MAX - 1));
        
        // Mixed arithmetic that creates complex range
        wide_int_t result = shifted + (modifier * 0xFEDCBA987654321LL);
        
        // Additional transformation that affects signedness
        if (modifier & 1) {
            result = result | 0x8000000000000000LL;  // Could set high bit
        } else {
            result = result & ~0x8000000000000000LL; // Clear high bit
        }
        
        return result;
    } else {
        // Path 2: Smaller shift with different arithmetic
        wide_int_t shifted = base >> (shift_count & 31);
        
        // Arithmetic that creates both positive and negative possibilities
        wide_int_t result = shifted - (modifier * 0x5555555555555555LL);
        
        // Conditional transformation based on shift count
        if (shift_count & 1) {
            result = result * 3;  // Could overflow
        }
        
        return result;
    }
}

__attribute__((noinline))
int check_ranges(wide_int_t value, int iteration) {
    int checksum = 0;
    
    // Complex conditional with mixed signed/unsigned comparisons
    // This should trigger the fixed-value range analysis
    
    // Signed comparison path (sgt)
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        checksum += 1;
    }
    
    // Combined condition with equality check
    if (value == (wide_int_t)0x7FFFFFFFFFFFFFFFLL || 
        value < (wide_int_t)0x8000000000000000LL) {
        checksum += 2;
    }
    
    // Unsigned comparison path (ugt)
    uwide_int_t uvalue = (uwide_int_t)value;
    if (uvalue > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        // This branch should never be taken, but forces unsigned analysis
        checksum += 4;
    }
    
    // More complex condition with AND
    if ((value > 0) && (uvalue < (uwide_int_t)0x8000000000000000ULL)) {
        checksum += 8;
    }
    
    // Nested comparisons that require range analysis
    if (iteration & 1) {
        wide_int_t threshold = (wide_int_t)0x4000000000000000LL << (iteration & 3);
        if (value > threshold || value < -threshold) {
            checksum += 16;
        }
    }
    
    return checksum;
}

int main() {
    volatile int seed = 123456789;  // Prevent constant propagation
    int total_checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift counts
        int shift1 = (seed + i * 3) & (WIDE_SHIFT_MAX - 1);
        int shift2 = (seed ^ i * 5) & 31;
        
        // Modify seed to vary inputs
        seed = seed * 1103515245 + 12345;
        
        // Get a modifier with mixed positive/negative potential
        int modifier = (seed >> 16) & 0xFF;
        if (modifier & 0x80) {
            modifier = -modifier;  // Create negative values
        }
        
        // Perform wide integer computation
        wide_int_t result = wide_int_compute(shift1, modifier);
        
        // Apply additional transformation
        if (i & 1) {
            result = result << (shift2 & 7);
        } else {
            result = result >> (shift2 & 7);
        }
        
        // Check ranges with complex conditions
        total_checksum += check_ranges(result, i);
        
        // Additional comparison that might trigger different analysis
        if ((i % 100) == 0) {
            wide_int_t temp = result * result;  // Could overflow
            if (temp > 0 && temp < (wide_int_t)0x1000000000000000LL) {
                total_checksum ^= (int)temp;
            }
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return total_checksum != 0 ? 0 : 1;
}
