#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(void) {
    // Use boundary values for short _Fract
    short _Fract sf_min = 0x8000;  // Minimum: -1.0
    short _Fract sf_max = 0x7FFF;  // Maximum: ~0.9999
    short _Fract sf_zero = 0x0000; // Zero
    short _Fract sf_half = 0x4000; // 0.5
    
    // Mixed precision operations to force range analysis
    _Sat unsigned long _Accum ula = 0x8000000000000000UL; // Minimum
    _Sat signed long _Accum sla = 0x7FFFFFFFFFFFFFFF;     // Maximum
    
    // Operations that should trigger range comparisons
    short _Fract result1 = sf_max * sf_half;
    _Sat signed long _Accum result2 = sla / 2;
    
    // Boundary comparisons - should trigger the uncovered logic
    if (result1 > sf_half) {
        // Force evaluation of high/low part comparisons
        volatile short _Fract temp = result1;
        (void)temp;
    }
    
    // Mixed signedness comparison
    if ((_Accum)ula < (_Accum)sla) {
        volatile _Sat signed long _Accum temp = result2;
        (void)temp;
    }
}

__attribute__((optimize("O3")))
void test_accum_boundaries(void) {
    // Initialize at boundaries
    _Sat signed short _Accum ssa_min = -0x8000p8;  // Minimum
    _Sat signed short _Accum ssa_max = 0x7FFFp8;   // Maximum
    _Sat unsigned short _Accum usa_max = 0xFFFFp8; // Unsigned max
    
    // Operations near boundaries
    _Sat signed short _Accum prod1 = ssa_max * 0.5k;
    _Sat signed short _Accum prod2 = ssa_min * 0.5k;
    
    // Division that approaches boundaries
    _Sat signed short _Accum div1 = ssa_max / 0.1k;
    _Sat signed short _Accum div2 = ssa_min / 0.1k;
    
    // Comparisons that should trigger the uncovered condition
    // a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (prod1 > ssa_max) {
        volatile _Sat signed short _Accum temp = prod1;
        (void)temp;
    }
    
    if (prod2 < ssa_min) {
        volatile _Sat signed short _Accum temp = prod2;
        (void)temp;
    }
    
    // Use builtins with fixed-point to force overflow analysis
    _Sat signed short _Accum overflow_check;
    if (__builtin_mul_overflow(ssa_max, 2k, &overflow_check)) {
        volatile _Sat signed short _Accum temp = overflow_check;
        (void)temp;
    }
}

// Struct with fixed-point members to test range tracking through memory
struct FixedPointStruct {
    _Fract f;
    _Sat unsigned _Accum ua;
    signed long _Accum sla;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointStruct fps[4];
    
    // Initialize array with boundary values
    for (int i = 0; i < 4; i++) {
        fps[i].f = i * 0.25r;  // 0, 0.25, 0.5, 0.75
        fps[i].ua = i * 0.25uk;
        fps[i].sla = i * 0.25lk - 0.5lk;  // -0.5, -0.25, 0, 0.25
    }
    
    // Perform operations on struct members
    for (int i = 0; i < 3; i++) {
        // Mixed type operations
        fps[i].sla = fps[i].sla * fps[i+1].f;
        fps[i].ua = fps[i].ua + fps[i+1].ua;
        
        // Comparisons that should trigger range analysis
        if (fps[i].sla > fps[i+1].sla) {
            // Force evaluation of comparison logic
            volatile signed long _Accum temp = fps[i].sla;
            (void)temp;
        }
        
        // Test near boundary conditions
        if (fps[i].ua > 0.9uk && fps[i].ua < 1.0uk) {
            volatile _Sat unsigned _Accum temp = fps[i].ua;
            (void)temp;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Different fractional bit configurations
    unsigned short _Fract usf = 0.5ur;
    signed long _Accum sla = 1000lk;
    _Sat unsigned _Accum ua = 0.75uk;
    
    // Mixed precision operations forcing conversions
    signed long _Accum result1 = sla * usf;  // usf converted to _Accum
    _Sat unsigned _Accum result2 = ua / usf; // usf converted to _Accum
    
    // Operations that should exercise zext/alshift logic
    short _Fract sf_array[8];
    for (int i = 0; i < 8; i++) {
        sf_array[i] = i * 0.125r;
    }
    
    // Accumulate with different precisions
    _Accum accumulator = 0k;
    for (int i = 0; i < 8; i++) {
        accumulator += sf_array[i];
        
        // Boundary check on accumulation
        if (accumulator > 0.9k && accumulator < 1.0k) {
            volatile _Accum temp = accumulator;
            (void)temp;
        }
    }
    
    // Force comparison with computed boundaries
    if (result1 > 500lk || (result1 == 500lk && result2 > 1.0uk)) {
        volatile signed long _Accum temp1 = result1;
        volatile _Sat unsigned _Accum temp2 = result2;
        (void)temp1; (void)temp2;
    }
}

__attribute__((optimize("O3")))
void test_saturation_limits(void) {
    // Test saturation at extremes
    _Sat signed short _Accum ssa1 = 0x7FFFp8;  // Max positive
    _Sat signed short _Accum ssa2 = -0x8000p8; // Max negative
    
    // Operations that should saturate
    _Sat signed short _Accum saturated_sum = ssa1 + ssa1;  // Should saturate
    _Sat signed short _Accum saturated_diff = ssa2 - ssa1; // Should saturate
    
    // Multiplication near saturation point
    _Sat signed short _Accum near_sat = 0x7000p8;
    _Sat signed short _Accum sat_prod = near_sat * 1.5k;
    
    // Comparisons to trigger boundary logic
    if (saturated_sum == ssa1) {  // Check if saturation occurred
        volatile _Sat signed short _Accum temp = saturated_sum;
        (void)temp;
    }
    
    // Complex condition similar to uncovered code
    if (sat_prod > ssa1 || (sat_prod == ssa1 && near_sat > 0k)) {
        volatile _Sat signed short _Accum temp1 = sat_prod;
        volatile _Sat signed short _Accum temp2 = near_sat;
        (void)temp1; (void)temp2;
    }
    
    // Use builtin overflow with fixed-point
    _Sat signed short _Accum overflow_result;
    if (__builtin_add_overflow(ssa1, 0x100p8, &overflow_result)) {
        volatile _Sat signed short _Accum temp = overflow_result;
        (void)temp;
    }
}

int main(void) {
    // Initialize volatile to prevent dead code elimination
    volatile int result = 0;
    
    // Execute all test functions
    test_short_fract_range();
    result += 1;
    
    test_accum_boundaries();
    result += 2;
    
    test_struct_operations();
    result += 4;
    
    test_mixed_precision_conversions();
    result += 8;
    
    test_saturation_limits();
    result += 16;
    
    // Print result to ensure all code executes
    printf("Test result: %d\n", result);
    
    return 0;
}
