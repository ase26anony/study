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
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    // Use modulo to keep shift within bounds for meaningful analysis
    int bounded_shift = shift_amount & (WIDE_SHIFT_MAX - 1);
    
    // Perform shift that can overflow into sign bit
    wide_int_t shifted = base << bounded_shift;
    
    // Add/subtract based on modifier to create signed range complexity
    if (modifier & 1) {
        shifted += (wide_int_t)(modifier * 0x100000001LL);
    } else {
        shifted -= (wide_int_t)(modifier * 0x100000001LL);
    }
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i_f_bits) {
    // This comparison structure mimics the uncovered code's logic
    // The compiler will need to compute max_r/max_s for range checking
    
    // Create conditions that require both signed and unsigned comparisons
    wide_int_t signed_threshold = (wide_int_t)1 << (i_f_bits - 1);
    uwide_int_t unsigned_threshold = (uwide_int_t)1 << i_f_bits;
    
    // Mixed signed/unsigned comparisons like in the target code
    if (value > signed_threshold) {
        return 1;
    }
    
    // Unsigned comparison after type conversion
    if ((uwide_int_t)value > unsigned_threshold) {
        return 2;
    }
    
    // Complex condition with AND/OR like in the uncovered lines
    if (value > 0 || (value == 0 && (uwide_int_t)value > 0)) {
        return 3;
    }
    
    return 0;
}

__attribute__((noinline))
int analyze_double_int_range(wide_int_t a, int shift_var) {
    // This function creates the exact scenario for double-int range analysis
    // by performing operations that require computing max_r/min_s bounds
    
    // Simulate the i_f_bits parameter from the uncovered code
    int i_f_bits = (shift_var & 31) + 1;
    
    // Operations that force wide integer range computation
    wide_int_t shifted = a << i_f_bits;
    
    // Zero extension (logical shift right then left)
    uwide_int_t zero_ext = (uwide_int_t)shifted >> i_f_bits;
    zero_ext = zero_ext << i_f_bits;
    
    // Sign extension
    wide_int_t sign_ext = shifted >> i_f_bits;
    sign_ext = sign_ext << i_f_bits;
    
    // Comparisons that should trigger the target block
    // These mirror the structure in the uncovered lines
    wide_int_t max_r_val = 0;
    wide_int_t min_r_val = -1;
    
    int result = 0;
    
    // First comparison path (signed greater than)
    if (shifted > max_r_val) {
        result |= 1;
    }
    
    // Second comparison path (equality with max_r then unsigned greater than)
    uwide_int_t max_s_val = (uwide_int_t)-1;
    max_s_val = max_s_val >> (sizeof(uwide_int_t) * 8 - i_f_bits);
    
    if (shifted == max_r_val && (uwide_int_t)shifted > max_s_val) {
        result |= 2;
    }
    
    // Third comparison path with min values
    uwide_int_t min_s_val = 1;
    min_s_val = min_s_val << i_f_bits;
    
    if (shifted < min_r_val || (shifted == min_r_val && (uwide_int_t)shifted < min_s_val)) {
        result |= 4;
    }
    
    return result;
}

int main() {
    volatile int seed = 0x12345678; // volatile to prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to generate multiple analysis contexts
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant inputs
        int shift = (seed + i) & 31;  // Bounded shift amount
        int mod = (seed * i) & 0xFF;  // Bounded modifier
        
        // Compute wide integer with complex transformations
        wide_int_t result = wide_int_compute(shift, mod);
        
        // Path 1: Signed comparison against large positive value
        // This should trigger signed range analysis
        wide_int_t signed_limit = (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
        if (result > signed_limit) {
            checksum += 1;
        }
        
        // Path 2: Unsigned comparison against max value
        // This should trigger unsigned range analysis
        uwide_int_t unsigned_limit = (uwide_int_t)-1;
        if ((uwide_int_t)result > unsigned_limit) {
            checksum += 2;
        }
        
        // Path 3: Mixed signed/unsigned comparisons in conditional expression
        // Mimics the structure in the uncovered lines
        if (result > 0 || (result == 0 && (uwide_int_t)result > 0)) {
            checksum += 4;
        }
        
        // Path 4: Direct call to function that should trigger the exact uncovered block
        int range_result = analyze_double_int_range(result, shift);
        checksum += range_result;
        
        // Path 5: Additional complexity with conditional shift amounts
        int conditional_shift;
        if (result > 0) {
            conditional_shift = shift + 1;
        } else {
            conditional_shift = shift - 1;
        }
        
        // Bounded conditional shift
        conditional_shift &= 31;
        wide_int_t conditionally_shifted = result << conditional_shift;
        
        // Comparison that requires range analysis of the shifted value
        if (conditionally_shifted > (wide_int_t)0x1000000000000000LL) {
            checksum += 8;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    // Use checksum to prevent dead code elimination
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
