#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from removing critical comparisons
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 0.9999r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Mixed precision operations that force range analysis
    _Sat signed long _Accum mixed1 = sla_max * (_Sat signed long _Accum)ssf_max;
    _Sat unsigned long _Accum mixed2 = ula_max * (_Sat unsigned long _Accum)usf_max;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (mixed1 > sla_max) {
        // This should not happen due to saturation, but forces comparison
        result += 1;
    }
    
    if (mixed2 < ula_min) {
        result += 2;
    }
    
    // Equality comparisons at boundaries
    if (mixed1 == sla_max) {
        result += 4;
    }
    
    if (mixed2 == ula_max) {
        result += 8;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed short _Accum ssa1 = 127.9999hk;
    _Sat signed short _Accum ssa2 = 127.9999hk;
    _Sat signed short _Accum ssa_result;
    
    // Use builtin overflow checks with fixed-point types
    int overflow = __builtin_mul_overflow(ssa1, ssa2, &ssa_result);
    if (overflow) {
        result += 16;
    }
    
    _Sat unsigned short _Accum usa1 = 255.9999uhk;
    _Sat unsigned short _Accum usa2 = 255.9999uhk;
    _Sat unsigned short _Accum usa_result;
    
    overflow = __builtin_add_overflow(usa1, usa2, &usa_result);
    if (overflow) {
        result += 32;
    }
    
    // Division near boundaries
    _Sat signed long _Fract slf1 = 0.999999999lr;
    _Sat signed long _Fract slf2 = 0.000000001lr;
    
    if (slf2 != 0) {
        _Sat signed long _Fract div_result = slf1 / slf2;
        if (div_result > 1000000.0lr) {
            result += 64;
        }
    }
}

// Struct containing fixed-point values to test range tracking through memory
struct FixedPointContainer {
    _Sat signed short _Fract f1;
    _Sat unsigned short _Fract f2;
    _Sat signed long _Accum a1;
    _Sat unsigned long _Accum a2;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container = {
        .f1 = -0.9999r,
        .f2 = 0.9999ur,
        .a1 = -255.999999999lk,
        .a2 = 255.999999999ulk
    };
    
    // Array of structs
    struct FixedPointContainer arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i].f1 = container.f1 * (_Sat signed short _Fract)i;
        arr[i].f2 = container.f2 * (_Sat unsigned short _Fract)i;
        arr[i].a1 = container.a1 * (_Sat signed long _Accum)i;
        arr[i].a2 = container.a2 * (_Sat unsigned long _Accum)i;
        
        // Boundary comparisons on array elements
        if (arr[i].f1 > 0.0r && arr[i].f2 < 1.0ur) {
            result += 128 * (i + 1);
        }
        
        if (arr[i].a1 == container.a1 || arr[i].a2 == container.a2) {
            result += 256 * (i + 1);
        }
    }
}

__attribute__((optimize("O3")))
void test_control_flow_ranges() {
    _Sat signed short _Accum accumulator = 0.0hk;
    _Sat signed short _Accum increment = 0.125hk;  // 1/8
    
    // Loop with fixed-point condition
    for (int i = 0; i < 100; i++) {
        accumulator += increment;
        
        // Multiple boundary checks in loop
        if (accumulator > 10.0hk) {
            result += 512;
            break;
        }
        
        if (accumulator < -10.0hk) {
            result += 1024;
            break;
        }
        
        // Equality check at specific boundary
        if (accumulator == 0.0hk) {
            result += 2048;
        }
    }
    
    // Nested conditionals with fixed-point comparisons
    _Sat unsigned long _Fract ulf1 = 0.5ulr;
    _Sat unsigned long _Fract ulf2 = 0.25ulr;
    
    if (ulf1 > 0.3ulr) {
        if (ulf2 < 0.3ulr) {
            _Sat unsigned long _Fract product = ulf1 * ulf2;
            if (product > 0.1ulr && product < 0.2ulr) {
                result += 4096;
            }
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries() {
    // Test with values at absolute boundaries
    _Sat signed _Fract sf_min = -1.0r;
    _Sat signed _Fract sf_max = 0.9999r;
    
    _Sat unsigned _Fract uf_min = 0.0ur;
    _Sat unsigned _Fract uf_max = 0.9999ur;
    
    // Operations that push boundaries
    _Sat signed _Fract sf_near_max = sf_max * sf_max;
    _Sat unsigned _Fract uf_near_max = uf_max * uf_max;
    
    // These comparisons should trigger the high/low part comparisons
    // similar to a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (sf_near_max > sf_max) {
        result += 8192;
    }
    
    if (uf_near_max < uf_min) {
        result += 16384;
    }
    
    // Test with _Accum types that have more integer bits
    _Sat signed long _Accum sla_boundary = 255.999999999lk;
    _Sat signed long _Accum sla_half = sla_boundary * 0.5lk;
    
    if (sla_half > 127.0lk && sla_half < 128.0lk) {
        result += 32768;
    }
    
    // Force comparison with zero boundary
    _Sat signed short _Accum ssa_zero = 0.0hk;
    if (sla_half > ssa_zero) {
        result += 65536;
    }
}

int main() {
    // Run all test functions to exercise different aspects of fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_control_flow_ranges();
    test_extreme_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
