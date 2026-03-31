#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point variables at type boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 0.9999r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Mixed precision operations that force range analysis
    _Sat signed long _Accum mixed1 = (_Sat signed long _Accum)usf_max * sla_max;
    _Sat signed long _Accum mixed2 = (_Sat signed long _Accum)ssf_min * ula_max;
    
    // Boundary value comparisons - should trigger the uncovered comparison logic
    if (mixed1 > sla_max) {
        result += 1;
    }
    
    if (mixed2 < sla_min) {
        result += 2;
    }
    
    // Equality comparisons at boundaries
    if (usf_max == 0.9999ur) {
        result += 4;
    }
    
    if (ssf_min == -1.0r) {
        result += 8;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed short _Fract a = 0.8r;
    _Sat signed short _Fract b = 0.9r;
    _Sat signed short _Fract c;
    
    // Use builtins to introduce overflow checks
    int overflow = 0;
    overflow |= __builtin_mul_overflow(a, b, &c);
    
    _Sat signed short _Fract d = 0.99r;
    _Sat signed short _Fract e;
    overflow |= __builtin_add_overflow(c, d, &e);
    
    // Conditional based on overflow results
    if (overflow) {
        result += 16;
    }
    
    // Force comparisons near saturation boundaries
    _Sat unsigned long _Accum x = 255.5ulk;
    _Sat unsigned long _Accum y = 0.5ulk;
    
    if (x + y > 255.999999999ulk) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointData {
    _Sat signed short _Fract fract_array[4];
    _Sat unsigned long _Accum accum_value;
    _Sat signed long _Accum signed_accum;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointData data = {
        .fract_array = {0.1r, 0.5r, 0.9r, -0.5r},
        .accum_value = 128.5ulk,
        .signed_accum = -128.5lk
    };
    
    // Operations on struct members
    for (int i = 0; i < 4; i++) {
        data.signed_accum += (_Sat signed long _Accum)data.fract_array[i];
        
        // Boundary comparisons in loop
        if (data.signed_accum > 127.0lk) {
            result += 64;
            data.signed_accum = 127.0lk;
        }
        
        if (data.signed_accum < -128.0lk) {
            result += 128;
            data.signed_accum = -128.0lk;
        }
    }
    
    // Comparison that should trigger the uncovered logic
    if (data.accum_value > 255.0ulk || 
        (data.accum_value == 255.0ulk && data.signed_accum > 0.0lk)) {
        result += 256;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test various precision conversions
    _Sat unsigned short _Fract usf = 0.75ur;
    _Sat signed short _Fract ssf = -0.25r;
    _Sat unsigned long _Accum ula = 100.25ulk;
    _Sat signed long _Accum sla = -100.25lk;
    
    // Mixed operations forcing precision conversions
    _Sat signed long _Accum conv1 = sla * (_Sat signed long _Accum)usf;
    _Sat signed long _Accum conv2 = (_Sat signed long _Accum)ssf * ula;
    
    // Division operations that affect range analysis
    _Sat signed long _Accum div_result = conv1 / (_Sat signed long _Accum)0.5r;
    
    // Complex boundary condition
    if (conv1 > sla || (conv1 == sla && conv2 > ula)) {
        result += 512;
    }
    
    if (div_result < conv2 || (div_result == conv2 && sla < conv1)) {
        result += 1024;
    }
}

__attribute__((optimize("O3")))
void test_edge_cases() {
    // Zero and near-zero values
    _Sat signed short _Fract zero_fract = 0.0r;
    _Sat unsigned long _Accum zero_accum = 0.0ulk;
    
    // Maximum values
    _Sat signed short _Fract max_fract = 0.9999r;
    _Sat unsigned long _Accum max_accum = 255.999999999ulk;
    
    // Operations that should saturate
    _Sat signed short _Fract saturated_mul = max_fract * max_fract;
    _Sat unsigned long _Accum saturated_add = max_accum + max_accum;
    
    // Comparisons that should exercise the uncovered lines
    if (saturated_mul > max_fract) {
        result += 2048;
    }
    
    if (saturated_add > max_accum || 
        (saturated_add == max_accum && zero_fract > zero_accum)) {
        result += 4096;
    }
    
    // Test with negative boundaries
    _Sat signed long _Accum neg_max = -255.999999999lk;
    _Sat signed long _Accum neg_min = -256.0lk;
    
    if (neg_max * (_Sat signed long _Accum)0.5r < neg_min) {
        result += 8192;
    }
}

int main() {
    // Run all test functions to exercise different aspects of fixed-point analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_edge_cases();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
