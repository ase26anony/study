#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating test cases
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Test with various fixed-point types near boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 1.0ur - 0.0001ur;
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 1.0r - 0.0001r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999999999999ulk;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999999999999lk;
    
    // Mixed precision operations that should trigger range analysis
    _Sat signed long _Accum mixed1 = (_Sat signed long _Accum)usf_max * sla_max;
    _Sat signed long _Accum mixed2 = (_Sat signed long _Accum)ssf_min * ula_max;
    
    // Boundary comparisons - these should exercise the uncovered comparison logic
    if (mixed1 > sla_max) {
        result += 1;
    }
    
    if (mixed2 < sla_min) {
        result += 2;
    }
    
    // Equality comparisons at boundaries
    _Sat signed short _Fract boundary_val = 0.9999r;
    if (boundary_val == ssf_max) {
        result += 4;
    }
    
    // Complex boundary condition
    _Sat unsigned long _Accum ula_mid = 128.0ulk;
    if (ula_mid > ula_min && ula_mid < ula_max) {
        result += 8;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks(void) {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 200.0lk;
    _Sat signed long _Accum c;
    
    // Use builtin overflow checks with fixed-point types
    int overflow = 0;
    
    // These should interact with value range analysis
    if (__builtin_mul_overflow((long)a, (long)b, (long*)&c)) {
        overflow = 1;
    }
    
    _Sat signed short _Fract d = 0.9r;
    _Sat signed short _Fract e = 0.9r;
    _Sat signed short _Fract f;
    
    if (__builtin_add_overflow((short)d, (short)e, (short*)&f)) {
        overflow |= 2;
    }
    
    result += overflow * 16;
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Sat signed short _Fract ssf;
    _Sat unsigned long _Accum ula;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container = {
        .usf_array = {0.1ur, 0.5ur, 0.9ur, 1.0ur},
        .sla_array = {-100.0lk, 0.0lk, 100.0lk, 200.0lk},
        .ssf = 0.75r,
        .ula = 128.5ulk
    };
    
    // Operations on struct members that should propagate ranges
    for (int i = 0; i < 4; i++) {
        container.sla_array[i] = container.sla_array[i] * (_Sat signed long _Accum)container.usf_array[i];
        
        // Boundary checks in loop
        if (container.sla_array[i] > 255.0lk) {
            result += 32 * (i + 1);
        }
        if (container.sla_array[i] < -256.0lk) {
            result += 64 * (i + 1);
        }
    }
    
    // Mixed operation with struct members
    _Sat signed long _Accum mixed_op = container.ula * container.ssf;
    if (mixed_op > 0.0lk && mixed_op < 256.0lk) {
        result += 128;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions(void) {
    // Test precision conversions that should exercise zext and shift operations
    _Sat unsigned short _Fract usf_val = 0.75ur;
    _Sat signed short _Fract ssf_val = -0.5r;
    
    // Convert to higher precision
    _Sat unsigned long _Accum ula_from_usf = (_Sat unsigned long _Accum)usf_val;
    _Sat signed long _Accum sla_from_ssf = (_Sat signed long _Accum)ssf_val;
    
    // Operations with converted values
    _Sat signed long _Accum converted_op = sla_from_ssf * 512.0lk;
    _Sat unsigned long _Accum converted_op2 = ula_from_usf / 0.25ulk;
    
    // Boundary comparisons with converted values
    if (converted_op > 0.0lk) {
        result += 256;
    }
    
    if (converted_op2 < 4.0ulk) {
        result += 512;
    }
    
    // Test near-boundary values
    _Sat unsigned short _Fract near_max = 0.9999ur;
    _Sat signed short _Fract near_min = -0.9999r;
    
    _Sat unsigned long _Accum ula_near = (_Sat unsigned long _Accum)near_max * 256.0ulk;
    _Sat signed long _Accum sla_near = (_Sat signed long _Accum)near_min * 256.0lk;
    
    // These comparisons should trigger the specific uncovered condition
    if (ula_near > 255.0ulk) {
        result += 1024;
    }
    
    if (sla_near < -256.0lk) {
        result += 2048;
    }
}

__attribute__((optimize("O3")))
void test_division_edge_cases(void) {
    // Division operations that create boundary values
    _Sat unsigned long _Accum ula_div1 = 255.0ulk / 2.0ulk;
    _Sat unsigned long _Accum ula_div2 = 1.0ulk / 256.0ulk;
    _Sat signed long _Accum sla_div1 = -256.0lk / 2.0lk;
    _Sat signed long _Accum sla_div2 = 1.0lk / -256.0lk;
    
    // Comparisons that should exercise the high/low part logic
    if (ula_div1 > 127.0ulk) {
        result += 4096;
    }
    
    if (ula_div2 == 0.0ulk) {
        result += 8192;
    }
    
    if (sla_div1 < -128.0lk) {
        result += 16384;
    }
    
    if (sla_div2 > -1.0lk) {
        result += 32768;
    }
}

int main(void) {
    // Initialize result
    result = 0;
    
    // Run all test functions to exercise different aspects of fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_division_edge_cases();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile store to ensure all computations are kept
    volatile int final_result = result;
    
    return final_result != 0 ? 0 : 1;
}
