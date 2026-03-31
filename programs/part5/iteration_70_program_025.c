#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    volatile short _Fract sf_min = 0x8000;  // Minimum: -1.0
    volatile short _Fract sf_max = 0x7FFF;  // Maximum: ~0.99997
    volatile short _Fract sf_zero = 0x0000; // Zero: 0.0
    volatile short _Fract sf_half = 0x4000; // 0.5
    
    // Operations that approach boundaries
    short _Fract result1 = sf_max * sf_half;      // ~0.49998
    short _Fract result2 = sf_min * sf_half;      // -0.5
    short _Fract result3 = sf_max / sf_half;      // Approaches 2.0 but clamped
    
    // Boundary comparisons that should trigger range analysis
    if (result1 > sf_half) {
        // This branch may be taken depending on rounding
        volatile short _Fract temp = result1;
    }
    
    if (result2 < sf_zero) {
        // Always true, should be optimized
        volatile short _Fract temp = result2;
    }
    
    // Mixed comparison
    if ((result1 > sf_zero) && (result2 < sf_zero)) {
        volatile short _Fract temp = result1 + result2;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_operations(void) {
    // Saturated accumulators at boundaries
    _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFF;  // Max unsigned
    _Sat unsigned long _Accum ula_min = 0x0000000000000000;  // Min unsigned
    _Sat signed long _Accum sla_max = 0x7FFFFFFFFFFFFFFF;    // Max signed
    _Sat signed long _Accum sla_min = 0x8000000000000000;    // Min signed
    
    // Middle values for operations
    _Sat unsigned long _Accum ula_mid = 0x8000000000000000;
    _Sat signed long _Accum sla_mid = 0x4000000000000000;
    
    // Operations that will saturate
    _Sat unsigned long _Accum ula_add = ula_max + ula_mid;  // Should saturate
    _Sat unsigned long _Accum ula_sub = ula_min - ula_mid;  // Should saturate at 0
    _Sat signed long _Accum sla_mul = sla_max * sla_mid;    // May overflow
    
    // Built-in overflow checks with fixed-point
    int overflow1, overflow2;
    _Sat unsigned long _Accum ula_test = __builtin_add_overflow(ula_max, ula_mid, &ula_test);
    _Sat signed long _Accum sla_test = __builtin_mul_overflow(sla_max, sla_mid, &sla_test);
    
    // Complex boundary comparisons
    if (ula_add == ula_max) {
        // Should be true due to saturation
        volatile _Sat unsigned long _Accum temp = ula_add;
    }
    
    if (sla_mul > sla_mid) {
        // Depends on overflow behavior
        volatile _Sat signed long _Accum temp = sla_mul;
    }
    
    // Nested comparisons
    if ((ula_sub == ula_min) || (sla_mul < sla_zero)) {
        volatile _Sat unsigned long _Accum temp = ula_sub;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    short _Fract sf_val;
    _Sat unsigned short _Fract usf_val;
    _Sat signed long _Accum sla_val;
    unsigned long _Accum ula_val;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container[4];
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container[i].sf_val = (i % 2 == 0) ? 0x7FFF : 0x8000;  // Max/Min
        container[i].usf_val = (i % 3 == 0) ? 0xFFFF : 0x0000; // Max/Min unsigned
        container[i].sla_val = (i % 2 == 0) ? 0x7FFFFFFFFFFFFFFF : 0x8000000000000000;
        container[i].ula_val = (i % 2 == 0) ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000;
    }
    
    // Perform operations on array elements
    short _Fract sf_sum = 0x0000;
    _Sat unsigned long _Accum ula_product = 0x0000000000000001;
    
    for (int i = 0; i < 4; i++) {
        // Mixed-type operations
        sf_sum += container[i].sf_val;
        
        // Multiplication that may overflow
        ula_product *= container[i].ula_val;
        
        // Cross-type comparisons
        if (container[i].sf_val > 0x0000) {
            container[i].sla_val += 0x1000000000000000;
        } else {
            container[i].sla_val -= 0x1000000000000000;
        }
    }
    
    // Final boundary check
    if (sf_sum > 0x0000 || ula_product == 0x0000000000000000) {
        volatile short _Fract temp_sf = sf_sum;
        volatile _Sat unsigned long _Accum temp_ula = ula_product;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Different fixed-point types
    unsigned short _Fract usf_val = 0xFFFF;        // Max unsigned short fract
    signed long _Accum sla_val = 0x4000000000000000; // Mid-range signed accum
    
    // Precision conversions
    signed long _Accum converted1 = usf_val;       // Implicit conversion
    unsigned short _Fract converted2 = sla_val;    // May lose precision
    
    // Mixed precision arithmetic
    signed long _Accum mixed_result1 = sla_val * converted1;
    unsigned short _Fract mixed_result2 = usf_val * converted2;
    
    // Operations that force range analysis across different i_f_bits
    if (mixed_result1 > sla_val) {
        // May trigger extended range comparisons
        volatile signed long _Accum temp = mixed_result1;
    }
    
    if (mixed_result2 < usf_val) {
        // Precision loss may cause this
        volatile unsigned short _Fract temp = mixed_result2;
    }
    
    // Complex condition similar to uncovered code pattern
    signed long _Accum a_high = mixed_result1;
    unsigned long _Accum a_low = mixed_result2;
    signed long _Accum max_r = 0x7FFFFFFFFFFFFFFF;
    unsigned long _Accum max_s = 0xFFFFFFFFFFFFFFFF;
    
    // This should trigger the exact comparison pattern
    if (a_high > max_r || (a_high == max_r && a_low > max_s)) {
        volatile signed long _Accum temp = a_high;
    }
}

__attribute__((optimize("O3")))
void test_division_edge_cases(void) {
    // Division operations that create boundary values
    _Sat signed long _Accum sla_near_max = 0x7FFFFFFFFFFFFFFE;
    _Sat signed long _Accum sla_small = 0x0000000000000002;
    
    // Division that approaches maximum
    _Sat signed long _Accum div_result1 = sla_near_max / sla_small;
    
    // Division by values near zero
    _Sat signed long _Accum sla_one = 0x0100000000000000;
    _Sat signed long _Accum sla_tiny = 0x0000000000000001;
    _Sat signed long _Accum div_result2 = sla_one / sla_tiny;  // May overflow
    
    // Comparisons after division
    if (div_result1 > sla_near_max) {
        // Should not happen with saturation
        volatile _Sat signed long _Accum temp = div_result1;
    }
    
    if (div_result2 < sla_one && div_result2 > 0) {
        // Range analysis should determine this
        volatile _Sat signed long _Accum temp = div_result2;
    }
}

int main(void) {
    volatile int result = 0;
    
    // Execute all test functions
    test_short_fract_boundaries();
    result += 1;
    
    test_sat_accum_operations();
    result += 2;
    
    test_struct_operations();
    result += 4;
    
    test_mixed_precision_conversions();
    result += 8;
    
    test_division_edge_cases();
    result += 16;
    
    // Print result to prevent dead code elimination
    printf("Test result: %d\n", result);
    
    return 0;
}
