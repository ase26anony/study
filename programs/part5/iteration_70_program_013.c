#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating code
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types at or near boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 1.0ur - 0.0001ur;
    
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 1.0r - 0.0001r;
    
    _Sat unsigned long _Accum ula_min = 0.0uk;
    _Sat unsigned long _Accum ula_max = 255.9999999999999999uk;
    
    _Sat signed long _Accum sla_min = -256.0k;
    _Sat signed long _Accum sla_max = 255.9999999999999999k;
    
    // Mixed precision operations that should trigger range analysis
    signed long _Accum mixed_result;
    unsigned short _Fract usf_temp;
    
    // Operation 1: Multiplication near boundaries
    usf_temp = usf_max * 0.5ur;
    mixed_result = (_Sat signed long _Accum)usf_temp * sla_max;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (mixed_result > (_Sat signed long _Accum)100.0k) {
        result += 1;
    }
    
    if (usf_max > 0.9ur && usf_max < 1.0ur) {
        result += 2;
    }
    
    // Force comparisons with zero and negative values
    if (ssf_min < -0.5r && ssf_min > -1.0r) {
        result += 4;
    }
    
    // Test overflow behavior with builtins
    _Sat signed short _Fract ssf1 = 0.8r;
    _Sat signed short _Fract ssf2 = 0.9r;
    int overflow_flag;
    
    // This should trigger overflow analysis
    __builtin_mul_overflow((int)(ssf1 * 1000), (int)(ssf2 * 1000), &overflow_flag);
    if (overflow_flag) {
        result += 8;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test precision conversions between different fixed-point types
    _Sat unsigned short _Fract usf_val = 0.75ur;
    _Sat signed long _Accum sla_val;
    
    // Conversion that should trigger range extension logic
    sla_val = (_Sat signed long _Accum)usf_val;
    
    // Operations with shifted ranges
    sla_val = sla_val * 2.0k;
    
    // Comparisons that should exercise the a_high.sgt(max_r) logic
    if (sla_val > 1.0k) {
        result += 16;
    }
    
    // Test with values near zero
    _Sat signed short _Fract ssf_near_zero = 0.0001r;
    if (ssf_near_zero > 0.0r && ssf_near_zero < 0.001r) {
        result += 32;
    }
    
    // Division that creates extreme values
    _Sat signed long _Accum div_result;
    if (ssf_near_zero != 0.0r) {
        div_result = 1.0k / (_Sat signed long _Accum)ssf_near_zero;
        if (div_result > 1000.0k) {
            result += 64;
        }
    }
}

// Struct containing fixed-point values
struct FixedPointStruct {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_value;
    _Sat signed short _Fract ssf_value;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointStruct fps;
    
    // Initialize struct with boundary values
    fps.usf_array[0] = 0.0ur;
    fps.usf_array[1] = 0.25ur;
    fps.usf_array[2] = 0.75ur;
    fps.usf_array[3] = 1.0ur - 0.00001ur;  // Just below max
    
    fps.sla_value = 128.5k;
    fps.ssf_value = -0.5r;
    
    // Perform operations on struct members
    for (int i = 0; i < 4; i++) {
        fps.sla_value += (_Sat signed long _Accum)fps.usf_array[i];
        
        // Conditional based on fixed-point comparison
        if (fps.usf_array[i] > 0.5ur) {
            result += 128;
        }
    }
    
    // Mixed operation within struct
    fps.ssf_value = (_Sat signed short _Fract)(fps.sla_value * 0.01k);
    
    if (fps.ssf_value > 0.0r) {
        result += 256;
    }
}

__attribute__((optimize("O3")))
void test_extreme_values() {
    // Create values that should trigger min/max range calculations
    _Sat signed long _Accum extreme_vals[5];
    
    extreme_vals[0] = -256.0k;      // Minimum
    extreme_vals[1] = -128.0k;
    extreme_vals[2] = 0.0k;
    extreme_vals[3] = 128.0k;
    extreme_vals[4] = 255.9999999999999999k;  // Maximum
    
    // Operations that should force range comparisons
    for (int i = 0; i < 4; i++) {
        _Sat signed long _Accum diff = extreme_vals[i+1] - extreme_vals[i];
        
        // This comparison should trigger the uncovered condition logic
        if (diff > 0.0k) {
            result += 512;
        }
        
        // Multiplication that could overflow
        _Sat signed long _Accum product = extreme_vals[i] * 2.0k;
        if (product > 200.0k || product < -200.0k) {
            result += 1024;
        }
    }
    
    // Test with builtin overflow detection
    _Sat signed short _Fract a = 0.9r;
    _Sat signed short _Fract b = 0.95r;
    int overflow;
    
    // This should interact with value range analysis
    __builtin_add_overflow((int)(a * 1000), (int)(b * 1000), &overflow);
    if (overflow) {
        result += 2048;
    }
}

int main() {
    // Initialize result
    result = 0;
    
    // Run all tests to trigger fixed-point range analysis
    test_boundary_comparisons();
    test_precision_conversions();
    test_struct_operations();
    test_extreme_values();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure code isn't optimized away
    volatile int final_check = result;
    if (final_check > 0) {
        printf("Tests executed successfully.\n");
    }
    
    return 0;
}
