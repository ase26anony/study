#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent dead code elimination
volatile int result = 0;

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Fixed-point types with different sizes and signedness
    _Sat unsigned short _Fract usf1 = 0.9999r;
    _Sat signed short _Fract ssf1 = -0.9999r;
    _Sat unsigned long _Accum ula1 = 255.9999k;
    _Sat signed long _Accum sla1 = -255.9999k;
    
    // Approach boundaries through arithmetic
    for (int i = 0; i < 10; i++) {
        // Operations that approach type boundaries
        usf1 = usf1 * 1.1r;  // Will saturate at 1.0r
        ssf1 = ssf1 * 1.1r;  // Will saturate at -1.0r
        ula1 = ula1 + 0.5k;  // Will saturate at max
        sla1 = sla1 - 0.5k;  // Will saturate at min
        
        // Boundary comparisons that should trigger range analysis
        if (usf1 > 0.9r) {
            result += 1;
        }
        if (ssf1 < -0.9r) {
            result += 2;
        }
        if (ula1 > 255.0k) {
            result += 4;
        }
        if (sla1 < -255.0k) {
            result += 8;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_operations() {
    // Mixed precision operations
    unsigned short _Fract usf2 = 0.5r;
    signed long _Accum sla2 = 100.5k;
    _Fract f1 = 0.25r;
    
    // Precision conversions and range analysis
    sla2 = sla2 * (_Accum)usf2;  // Convert and multiply
    f1 = f1 / 0.1r;  // Division that increases value
    
    // Comparisons that should trigger the uncovered logic
    // These force evaluation of high/low part comparisons
    if (sla2 > 50.0k && sla2 < 51.0k) {
        result += 16;
    }
    
    if (f1 > 2.0r) {
        result += 32;
    }
    
    // Test overflow builtins with fixed-point
    _Sat signed short _Accum ssa1 = 100.0k;
    _Sat signed short _Accum ssa2 = 100.0k;
    int overflow;
    
    // These builtins interact with value range analysis
    overflow = __builtin_mul_overflow((int)ssa1, (int)ssa2, &result);
    if (overflow) {
        result += 64;
    }
}

__attribute__((optimize("O3")))
void test_struct_array_contexts() {
    // Struct with fixed-point members
    struct FixedPointStruct {
        _Sat unsigned short _Fract usf;
        _Sat signed long _Accum sla;
        _Fract f;
    };
    
    // Array of fixed-point values
    _Sat signed short _Accum ssa_array[4] = {
        -100.0k, -50.0k, 50.0k, 100.0k
    };
    
    struct FixedPointStruct fps = {
        .usf = 0.75r,
        .sla = 200.0k,
        .f = 0.333r
    };
    
    // Operations on struct members
    for (int i = 0; i < 4; i++) {
        ssa_array[i] = ssa_array[i] * 1.5k;
        fps.sla = fps.sla + (_Accum)ssa_array[i];
        
        // Boundary checks in loop
        if (ssa_array[i] > 127.0k || ssa_array[i] < -127.0k) {
            result += 128;
        }
    }
    
    // Final comparison that should trigger the uncovered lines
    if (fps.sla > 500.0k || fps.sla < -500.0k) {
        result += 256;
    }
    
    // More boundary testing
    fps.usf = fps.usf / 0.1r;  // Will saturate
    if (fps.usf == 1.0r) {
        result += 512;
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundary_conditions() {
    // Test values at exact boundaries
    _Sat unsigned short _Fract max_usf = 0.9999r;
    _Sat unsigned short _Fract min_usf = 0.0001r;
    _Sat signed long _Accum max_sla = 255.9999k;
    _Sat signed long _Accum min_sla = -255.9999k;
    
    // Operations that push to boundaries
    max_usf = max_usf * 1.1r;  // Should saturate to 1.0r
    min_usf = min_usf * 0.9r;  // Should approach 0.0r
    max_sla = max_sla + 0.1k;  // Should saturate
    min_sla = min_sla - 0.1k;  // Should saturate
    
    // Complex comparisons that should trigger the specific uncovered logic
    // These mimic the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) pattern
    _Sat signed long _Accum a = 255.5k;
    _Sat signed long _Accum b = 255.9k;
    
    // Multiple comparison chains
    if (a > max_sla || (a == max_sla && b > 255.5k)) {
        result += 1024;
    }
    
    // Test with zero boundaries
    _Sat signed short _Accum zero_test = 0.0k;
    for (int i = 0; i < 5; i++) {
        zero_test = zero_test + 0.2k;
        if (zero_test > 0.0k && zero_test < 1.0k) {
            result += 2048;
        }
    }
}

int main() {
    // Initialize with boundary values
    _Sat unsigned short _Fract init_usf = 0.0r;
    _Sat signed long _Accum init_sla = 0.0k;
    _Fract init_f = 0.5r;
    
    // Call test functions
    test_boundary_comparisons();
    test_mixed_precision_operations();
    test_struct_array_contexts();
    test_extreme_boundary_conditions();
    
    // Additional direct tests
    // Multiplication near boundaries
    _Sat unsigned long _Accum ula_boundary = 250.0k;
    ula_boundary = ula_boundary * 1.1k;  // Should approach max
    
    // Division near zero
    _Sat signed short _Fract ssf_boundary = 0.1r;
    ssf_boundary = ssf_boundary / 10.0r;  // Should approach zero
    
    // Final comparisons to ensure all logic paths are taken
    if (ula_boundary > 255.0k) {
        result += 4096;
    }
    
    if (ssf_boundary < 0.01r && ssf_boundary > 0.0r) {
        result += 8192;
    }
    
    // Print result to prevent elimination
    printf("Result: %d\n", result);
    
    return 0;
}
