#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent dead code elimination
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 1.0ur - 0x1p-8ur;  // Max for 8-bit fractional
    
    _Sat signed short _Accum ssa_min = -128.0k;  // Minimum for 8-bit integer part
    _Sat signed short _Accum ssa_max = 127.0k + 0.99609375k;  // Max for 8-bit integer + 8-bit fractional
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 4294967295.0ulk + 0.9999999997671694ulk;  // Max for 32-bit integer + 32-bit fractional
    
    // Mixed precision operations that force range analysis
    _Sat signed long _Accum sla1 = ssa_max * 0.5lk;
    _Sat signed long _Accum sla2 = ssa_min / 0.25lk;
    
    // Force comparisons that should trigger the uncovered code
    if (ssa_max > sla1) {
        result += 1;
    }
    
    if (ssa_min < sla2) {
        result += 2;
    }
    
    // Complex boundary comparison
    _Sat signed short _Accum intermediate = (ssa_max + ssa_min) * 0.5k;
    if (intermediate > 0.0k && intermediate < 64.0k) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_operations() {
    // Different fractional bit configurations
    _Sat unsigned short _Fract usf1 = 0.5ur;
    _Sat signed short _Accum ssa1 = 100.0k;
    _Sat unsigned long _Accum ula1 = 1000000.0ulk;
    
    // Mixed precision multiplication - forces precision conversion
    _Sat signed long _Accum mixed_result1 = ssa1 * usf1;
    _Sat signed long _Accum mixed_result2 = ula1 * usf1;
    
    // Use builtins for overflow detection
    _Sat signed short _Accum overflow_test;
    int overflow_flag = __builtin_mul_overflow(ssa1, ssa1, &overflow_test);
    
    if (overflow_flag) {
        result += 8;
    }
    
    // Boundary value comparisons
    if (mixed_result1 > 50.0lk && mixed_result1 < 150.0lk) {
        result += 16;
    }
    
    // Division near boundaries
    _Sat signed short _Accum div_result = ssa1 / 0.01k;
    if (div_result > 10000.0k) {
        result += 32;
    }
}

__attribute__((optimize("O3")))
void test_struct_arrays() {
    // Struct containing mixed fixed-point types
    struct FixedPointStruct {
        _Sat unsigned short _Fract usf;
        _Sat signed short _Accum ssa;
        _Sat unsigned long _Accum ula;
    };
    
    // Array of structs with boundary values
    struct FixedPointStruct fps[4] = {
        {0.0ur, -128.0k, 0.0ulk},
        {0.5ur, 0.0k, 2147483647.5ulk},
        {0.999ur, 127.996k, 4294967295.999ulk},
        {0.25ur, 64.0k, 1000000000.0ulk}
    };
    
    // Operations on array elements
    for (int i = 0; i < 3; i++) {
        // Multiplication that may approach boundaries
        _Sat signed long _Accum temp = fps[i].ssa * fps[i+1].usf;
        
        // Comparison that should trigger range analysis
        if (temp > fps[i].ssa || temp < fps[i+1].ssa) {
            result += (1 << (i + 6));  // 64, 128, 256
        }
    }
    
    // Array of just _Fract types
    _Sat unsigned short _Fract fract_array[8];
    for (int i = 0; i < 8; i++) {
        fract_array[i] = i * 0.125ur;  // 0, 0.125, 0.25, ..., 0.875
    }
    
    // Cumulative product that approaches 1.0
    _Sat unsigned short _Fract product = 1.0ur;
    for (int i = 0; i < 8; i++) {
        product *= fract_array[i] + 0.125ur;  // Avoid zero multiplication
    }
    
    if (product > 0.5ur && product < 1.0ur) {
        result += 512;
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries() {
    // Values at exact boundaries
    _Sat signed short _Accum exact_min = -128.0k;
    _Sat signed short _Accum exact_max = 127.0k + 0.99609375k;
    
    _Sat unsigned long _Accum ula_exact_max = 4294967295.0ulk + 0.9999999997671694ulk;
    
    // Operations that should saturate
    _Sat signed short _Accum saturated_sum = exact_max + exact_max;  // Should saturate
    _Sat signed short _Accum saturated_diff = exact_min - exact_max;  // Should saturate
    
    // Comparisons with boundary constants
    if (saturated_sum == exact_max) {  // Saturated to max
        result += 1024;
    }
    
    if (saturated_diff == exact_min) {  // Saturated to min
        result += 2048;
    }
    
    // Division by small numbers near boundaries
    _Sat signed short _Accum large_value = exact_max / 0.0001k;
    if (large_value == exact_max) {  // Should saturate
        result += 4096;
    }
    
    // Test the specific comparison pattern from uncovered code
    // by creating values that would exercise a_high.sgt(max_r) logic
    _Sat signed long _Accum val1 = ula_exact_max * 0.999999999lk;
    _Sat signed long _Accum val2 = ula_exact_max * 0.000000001lk;
    
    if (val1 > ula_exact_max / 2) {
        result += 8192;
    }
    
    if (val2 < ula_exact_max / 1000000000) {
        result += 16384;
    }
}

int main() {
    // Reset result
    result = 0;
    
    // Execute all test functions
    test_boundary_comparisons();
    test_mixed_precision_operations();
    test_struct_arrays();
    test_extreme_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int check = result;
    if (check > 0) {
        printf("Tests executed successfully\n");
    }
    
    return 0;
}
