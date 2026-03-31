#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Initialize fixed-point types at boundaries
    volatile _Sat unsigned short _Fract usf_min = 0.0ur;
    volatile _Sat unsigned short _Fract usf_max = 0.9999ur;
    
    volatile _Sat signed short _Fract ssf_min = -1.0r;
    volatile _Sat signed short _Fract ssf_max = 0.9999r;
    
    volatile _Sat unsigned long _Accum ula_min = 0.0uk;
    volatile _Sat unsigned long _Accum ula_max = 255.999999999uk;
    
    volatile _Sat signed long _Accum sla_min = -256.0k;
    volatile _Sat signed long _Accum sla_max = 255.999999999k;
    
    // Mixed precision operations to force range analysis
    _Sat signed long _Accum mixed_result;
    _Sat unsigned long _Accum umixed_result;
    
    // Operations that should trigger boundary comparisons
    mixed_result = (_Sat signed long _Accum)usf_max * sla_max;
    umixed_result = (_Sat unsigned long _Accum)usf_max * ula_max;
    
    // Conditional branches dependent on fixed-point ranges
    if (mixed_result > (_Sat signed long _Accum)0.5k) {
        // Force comparison with boundary values
        if (mixed_result == sla_max) {
            volatile int marker1 = 1;
        }
    }
    
    if (umixed_result < (_Sat unsigned long _Accum)128.0uk) {
        if (umixed_result == ula_min) {
            volatile int marker2 = 2;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks(void) {
    // Use builtins with fixed-point operands
    _Sat signed short _Accum ssa1 = 0.8k;
    _Sat signed short _Accum ssa2 = 0.9k;
    _Sat signed short _Accum ssa_result;
    int overflow_flag;
    
    // Multiplication with overflow check
    overflow_flag = __builtin_mul_overflow(ssa1, ssa2, &ssa_result);
    
    _Sat unsigned long _Accum ula1 = 200.0uk;
    _Sat unsigned long _Accum ula2 = 200.0uk;
    _Sat unsigned long _Accum ula_result;
    
    // Addition with overflow check
    overflow_flag |= __builtin_add_overflow(ula1, ula2, &ula_result);
    
    // Force comparisons near boundaries
    if (ssa_result > (_Sat signed short _Accum)0.5k) {
        volatile int marker3 = overflow_flag;
    }
    
    if (ula_result < (_Sat unsigned long _Accum)400.0uk) {
        volatile int marker4 = ula_result > ula1;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat signed short _Fract fract_array[4];
    _Sat unsigned long _Accum accum_value;
    _Sat signed long _Accum signed_accum;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container = {
        .fract_array = {0.1r, -0.5r, 0.9r, -0.9r},
        .accum_value = 128.5uk,
        .signed_accum = -128.5k
    };
    
    // Operations on struct members
    for (int i = 0; i < 4; i++) {
        container.signed_accum += (_Sat signed long _Accum)container.fract_array[i];
        
        // Boundary comparisons in loop
        if (container.signed_accum > (_Sat signed long _Accum)0.0k) {
            container.accum_value *= (_Sat unsigned long _Accum)1.1uk;
        } else {
            container.accum_value *= (_Sat unsigned long _Accum)0.9uk;
        }
        
        // Force comparison with extreme values
        if (container.signed_accum == (_Sat signed long _Accum)(-256.0k)) {
            volatile int marker5 = i;
        }
        
        if (container.accum_value == (_Sat unsigned long _Accum)255.999999999uk) {
            volatile int marker6 = i + 10;
        }
    }
    
    // Final boundary check
    if (container.signed_accum.sgt((_Sat signed long _Accum)0.0k) ||
        (container.signed_accum == (_Sat signed long _Accum)0.0k && 
         container.accum_value.ugt((_Sat unsigned long _Accum)0.0uk))) {
        volatile int marker7 = 1;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_range(void) {
    // Create values that should exercise the zext and shift operations
    _Sat unsigned short _Fract usf1 = 0.75ur;
    _Sat unsigned short _Fract usf2 = 0.25ur;
    
    // Mixed operations that require precision conversions
    _Sat unsigned long _Accum ula_result = (_Sat unsigned long _Accum)usf1 / usf2;
    _Sat signed long _Accum sla_result = (_Sat signed long _Accum)usf1 * (_Sat signed short _Fract)usf2;
    
    // Division near boundaries
    _Sat signed short _Fract ssf1 = 0.9999r;
    _Sat signed short _Fract ssf2 = 0.0001r;
    _Sat signed long _Accum div_result = (_Sat signed long _Accum)ssf1 / ssf2;
    
    // Comparisons that should trigger the uncovered logic
    if (ula_result > (_Sat unsigned long _Accum)2.0uk) {
        if (ula_result == (_Sat unsigned long _Accum)255.999999999uk) {
            volatile int marker8 = 1;
        }
    }
    
    if (sla_result < (_Sat signed long _Accum)(-100.0k)) {
        if (sla_result == (_Sat signed long _Accum)(-256.0k)) {
            volatile int marker9 = 1;
        }
    }
    
    // This should exercise the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) logic
    if (div_result > (_Sat signed long _Accum)1000.0k ||
        (div_result == (_Sat signed long _Accum)1000.0k && 
         ula_result > (_Sat unsigned long _Accum)100.0uk)) {
        volatile int marker10 = 1;
    }
}

int main(void) {
    volatile int result = 0;
    
    // Call all test functions
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_mixed_precision_range();
    
    // Aggregate results to prevent dead code elimination
    result += 1;
    
    printf("Test completed with result: %d\n", result);
    
    return 0;
}
