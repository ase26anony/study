#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    // Use short _Fract types near boundaries
    volatile short _Fract sf_min = 0x8000;  // Minimum: -1.0
    volatile short _Fract sf_max = 0x7FFF;  // Maximum: ~0.99997
    volatile short _Fract sf_zero = 0x0000; // Zero
    
    // Operations that force range analysis
    short _Fract result1 = sf_max * sf_max;  // Should approach 1.0
    short _Fract result2 = sf_min * sf_max;  // Should approach -1.0
    short _Fract result3 = sf_zero / sf_max; // Zero division
    
    // Boundary comparisons that should trigger the uncovered logic
    if (result1 > sf_max) {
        // This condition may be evaluated in range analysis
        volatile int marker1 = 1;
    }
    
    if (result2 < sf_min) {
        volatile int marker2 = 1;
    }
    
    // Force evaluation of equality comparisons
    if (result3 == sf_zero) {
        volatile int marker3 = 1;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_mixed_precision(void) {
    // Mixed signedness and saturation
    volatile _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFF;  // Max unsigned
    volatile _Sat signed long _Accum sla_min = 0x8000000000000000;    // Min signed
    volatile _Sat signed long _Accum sla_max = 0x7FFFFFFFFFFFFFFF;    // Max signed
    
    // Mixed precision operations
    _Sat signed long _Accum mixed_result1 = sla_max + ula_max;
    _Sat signed long _Accum mixed_result2 = sla_min * sla_max;
    
    // Operations that should saturate
    _Sat signed long _Accum saturated_sum = sla_max + sla_max;
    _Sat signed long _Accum saturated_prod = sla_max * sla_max;
    
    // Complex boundary comparisons
    if (mixed_result1 > sla_max) {
        volatile int marker4 = 1;
    }
    
    if (mixed_result2 < sla_min) {
        volatile int marker5 = 1;
    }
    
    // Test the specific comparison pattern from uncovered lines
    // This mimics a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (saturated_sum > sla_max || 
        (saturated_sum == sla_max && saturated_prod > sla_max)) {
        volatile int marker6 = 1;
    }
}

__attribute__((optimize("O3")))
void test_fract_arrays_structs(void) {
    // Fixed-point values in arrays
    volatile _Fract f_array[4] = {
        0x7FFFFFFF,  // Near max
        0x80000000,  // Min
        0x00000000,  // Zero
        0x40000000   // 0.5
    };
    
    // Struct with mixed fixed-point types
    struct FixedPointStruct {
        short _Fract sf;
        _Fract f;
        _Sat unsigned short _Fract usf;
        _Sat signed long _Accum sla;
    };
    
    struct FixedPointStruct fps = {
        .sf = 0x7FFF,
        .f = 0x7FFFFFFF,
        .usf = 0xFFFF,
        .sla = 0x7FFFFFFFFFFFFFFF
    };
    
    // Operations on array elements
    _Fract array_result = f_array[0] * f_array[1];  // Max * Min = -Max
    _Fract array_result2 = f_array[2] / f_array[3]; // Zero / 0.5 = Zero
    
    // Operations on struct members
    _Sat signed long _Accum struct_result = fps.sla * (_Sat signed long _Accum)fps.f;
    
    // Boundary checks on container results
    if (array_result < f_array[1]) {  // Check if less than minimum
        volatile int marker7 = 1;
    }
    
    if (struct_result > fps.sla) {
        volatile int marker8 = 1;
    }
}

__attribute__((optimize("O3")))
void test_builtin_overflow_checks(void) {
    // Use builtins with fixed-point types
    _Sat signed long _Accum sla1 = 0x7000000000000000;
    _Sat signed long _Accum sla2 = 0x7000000000000000;
    _Sat signed long _Accum overflow_result;
    
    // These builtins should interact with range analysis
    int overflow_mul = __builtin_mul_overflow(sla1, sla2, &overflow_result);
    int overflow_add = __builtin_add_overflow(sla1, sla2, &overflow_result);
    
    // Force evaluation of overflow conditions
    if (overflow_mul) {
        volatile int marker9 = 1;
    }
    
    if (overflow_add) {
        volatile int marker10 = 1;
    }
    
    // Test with _Fract types
    _Fract f1 = 0x7FFFFFFF;
    _Fract f2 = 0x7FFFFFFF;
    _Fract f_result;
    
    // Multiplication that should approach boundary
    f_result = f1 * f2;
    
    if (f_result > f1) {
        volatile int marker11 = 1;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions(void) {
    // Test conversions between different fixed-point types
    volatile unsigned short _Fract usf = 0xFFFF;  // Max unsigned short _Fract
    volatile signed long _Accum sla = 0x7FFFFFFFFFFFFFFF;  // Max signed long _Accum
    
    // Mixed precision operations forcing conversions
    _Sat signed long _Accum conv_result1 = sla + (_Sat signed long _Accum)usf;
    _Sat unsigned long _Accum conv_result2 = (_Sat unsigned long _Accum)sla * usf;
    
    // Operations that should exercise zext/alshift logic
    short _Fract sf_small = 0x4000;  // 0.5
    _Fract f_large = 0x7FFFFFFF;     // Near 1.0
    
    // Mixed size multiplication
    _Fract mixed_size_result = sf_small * f_large;  // Should be ~0.5
    
    // Boundary comparisons with converted values
    if (conv_result1 > sla) {
        volatile int marker12 = 1;
    }
    
    if (conv_result2 < usf) {
        volatile int marker13 = 1;
    }
    
    // Test the specific pattern that should trigger the uncovered code
    // This creates high/low comparisons similar to the uncovered lines
    if (mixed_size_result > f_large || 
        (mixed_size_result == f_large && sf_small > f_large)) {
        volatile int marker14 = 1;
    }
}

int main(void) {
    // Initialize volatile accumulator to prevent dead code elimination
    volatile int result = 0;
    
    // Execute all test functions
    test_short_fract_boundaries();
    result += 1;
    
    test_sat_accum_mixed_precision();
    result += 2;
    
    test_fract_arrays_structs();
    result += 3;
    
    test_builtin_overflow_checks();
    result += 4;
    
    test_precision_conversions();
    result += 5;
    
    // Print result to ensure all code executes
    printf("Test result: %d\n", result);
    
    return 0;
}
