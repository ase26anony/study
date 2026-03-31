#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Initialize fixed-point types with boundary values
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 1.0ur - 0.0001ur;
    
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 1.0r - 0.0001r;
    
    _Sat unsigned long _Accum ula_min = 0.0uk;
    _Sat unsigned long _Accum ula_max = 255.9999999999999999uk;
    
    _Sat signed long _Accum sla_min = -256.0k;
    _Sat signed long _Accum sla_max = 255.9999999999999999k;
    
    // Mixed precision operations to force range analysis
    _Sat signed long _Accum mixed_result;
    
    // Operation that should approach maximum boundary
    mixed_result = (_Sat signed long _Accum)usf_max * sla_max;
    
    // Control flow dependent on fixed-point ranges
    if (mixed_result > sla_max) {
        result += 1;  // Should not happen due to saturation
    }
    
    // Division that approaches minimum boundary
    _Sat signed short _Fract div_result = ssf_min / (_Sat signed short _Fract)0.5r;
    
    if (div_result < ssf_min) {
        result += 2;  // Should not happen due to saturation
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks(void) {
    _Sat unsigned long _Accum ula1 = 200.0uk;
    _Sat unsigned long _Accum ula2 = 100.0uk;
    _Sat unsigned long _Accum ula3;
    
    // Use built-in overflow checks with fixed-point operands
    int overflow;
    
    // Multiplication that could overflow
    overflow = __builtin_mul_overflow((unsigned long)ula1, (unsigned long)ula2, 
                                      (unsigned long*)&ula3);
    if (overflow) {
        result += 4;
    }
    
    // Addition near boundary
    _Sat signed long _Accum sla1 = 250.0k;
    _Sat signed long _Accum sla2 = 10.0k;
    _Sat signed long _Accum sum;
    
    overflow = __builtin_add_overflow((long)sla1, (long)sla2, (long*)&sum);
    if (overflow) {
        result += 8;  // Should trigger due to saturation boundary
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Fract f_array[4];
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = i * 0.25ur;
        container.sla_array[i] = -256.0k + i * 128.0k;
        container.f_array[i] = -1.0r + i * 0.5r;
    }
    
    // Perform operations on array elements
    _Sat signed long _Accum accum = 0.0k;
    
    for (int i = 0; i < 4; i++) {
        // Mixed type multiplication
        accum += container.sla_array[i] * (_Sat signed long _Accum)container.usf_array[i];
        
        // Control flow based on comparisons
        if (container.sla_array[i] > 0.0k && container.usf_array[i] > 0.5ur) {
            result += 16;
        }
    }
    
    // Final boundary check
    if (accum > 255.0k || accum < -256.0k) {
        result += 32;  // Should not happen due to saturation
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions(void) {
    // Test precision conversions that affect i_f_bits calculations
    _Sat unsigned short _Fract usf_val = 0.75ur;
    _Sat signed long _Accum sla_val;
    
    // Conversion with different fractional bits
    sla_val = (_Sat signed long _Accum)usf_val;
    
    // Operations that force range extension
    _Sat signed long _Accum scaled = sla_val * 512.0k;  // Force large range
    
    // Comparisons that should trigger the uncovered condition
    if (scaled > 255.9999999999999999k) {
        result += 64;
    }
    
    // Test near-zero boundary
    _Sat signed short _Fract ssf_small = 0.0001r;
    _Sat signed long _Accum multiplied = sla_val * (_Sat signed long _Accum)ssf_small;
    
    if (multiplied == 0.0k) {
        result += 128;  // May trigger due to precision loss
    }
}

__attribute__((optimize("O3")))
void test_division_boundaries(void) {
    // Division operations that create extreme values
    _Sat signed long _Accum numerator = 255.9999999999999999k;
    _Sat signed long _Accum denominator = 0.0000000000000001k;
    
    // This should saturate at maximum
    _Sat signed long _Accum div_result = numerator / denominator;
    
    if (div_result >= 255.9999999999999999k) {
        result += 256;
    }
    
    // Division by values near 1.0
    _Sat unsigned short _Fract usf_div = 0.9999ur / 1.0000ur;
    
    if (usf_div < 1.0ur) {
        result += 512;
    }
}

int main(void) {
    // Run all tests to exercise fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_division_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int check = result;
    if (check > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
