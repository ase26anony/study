#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned long _Accum max_accum = 0.99999999999999999994UK; // Near max
    _Sat unsigned short _Fract min_fract = 0.0hr; // Minimum
    signed long _Accum mid_accum = 0.5k;
    
    // Mixed precision operations to force range analysis
    signed long _Accum temp = (_Accum)max_accum * (_Accum)min_fract;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (max_accum > 0.99999999999999999993UK) {
        result += 1;
    }
    
    if (min_fract == 0.0hr) {
        result += 2;
    }
    
    // Chain comparisons to force complex condition evaluation
    if (temp < 0.1k && max_accum > 0.9UK) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_saturation_arithmetic() {
    // Test saturation at boundaries
    _Sat unsigned long _Accum sat1 = 1.5UK;  // Should saturate to max
    _Sat signed long _Accum sat2 = -1.5k;    // Should saturate to min
    
    // Operations that might overflow/saturate
    _Sat unsigned long _Accum prod = sat1 * 2.0UK;  // Definitely saturates
    
    // Comparisons near saturation boundaries
    if (sat1 == 0.99999999999999999994UK) {
        result += 8;
    }
    
    if (prod > 0.99999999999999999990UK) {
        result += 16;
    }
    
    // Use built-in overflow checks
    int overflow;
    _Sat unsigned long _Accum sum = __builtin_add_overflow(sat1, sat1, &overflow);
    if (overflow) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract f1;
    signed long _Accum a1;
    _Sat unsigned long _Accum sa1;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container = {
        .f1 = 0.5hr,
        .a1 = -0.25k,
        .sa1 = 0.75UK
    };
    
    // Array of fixed-point values
    _Sat unsigned short _Fract farray[4] = {
        0.0hr, 0.25hr, 0.5hr, 0.75hr
    };
    
    // Operations on struct members
    signed long _Accum temp = container.a1 * (_Accum)container.f1;
    
    // Loop with fixed-point condition
    for (int i = 0; i < 4; i++) {
        farray[i] = farray[i] * 2.0hr;  // Some will saturate
        
        // Comparison that depends on loop variable
        if (farray[i] > 0.9hr) {
            result += 64;
        }
    }
    
    // Complex comparison chain
    if (container.sa1 > 0.5UK && temp < 0.0k) {
        result += 128;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Different fixed-point types
    unsigned short _Fract usf = 0.8hr;
    signed long _Accum sla = -0.3k;
    _Sat unsigned long _Accum sula = 0.9UK;
    
    // Mixed precision operations
    signed long _Accum mixed1 = (_Accum)usf * sla;
    _Sat unsigned long _Accum mixed2 = sula / (_Accum)usf;
    
    // Boundary comparisons with mixed types
    if (mixed1 > -0.1k && mixed1 < 0.1k) {
        result += 256;
    }
    
    // Near-maximum comparison
    if (mixed2 > 0.99999999999999999992UK) {
        result += 512;
    }
    
    // Division near boundaries
    _Sat unsigned short _Fract div_result = usf / 0.1hr;
    if (div_result > 0.9hr) {
        result += 1024;
    }
}

__attribute__((optimize("O3")))
void test_edge_cases() {
    // Values at exact boundaries
    _Sat unsigned long _Accum zero = 0.0UK;
    _Sat unsigned long _Accum near_zero = 0.00000000000000000001UK;
    _Sat unsigned long _Accum near_max = 0.99999999999999999993UK;
    
    // Multiplication that creates extreme values
    _Sat unsigned long _Accum extreme = near_max * 1.00000000000000000001UK;
    
    // Comparisons that should exercise the uncovered logic
    if (zero == 0.0UK) {
        result += 2048;
    }
    
    if (near_zero > 0.0UK && near_zero < 0.00000000000000000002UK) {
        result += 4096;
    }
    
    // Complex condition similar to uncovered code
    if (extreme > 0.99999999999999999990UK || 
        (extreme == 0.99999999999999999990UK && near_zero > 0.0UK)) {
        result += 8192;
    }
    
    // Use built-in multiplication overflow
    int mul_overflow;
    _Sat unsigned long _Accum mul_result = __builtin_mul_overflow(
        near_max, near_max, &mul_overflow);
    if (mul_overflow) {
        result += 16384;
    }
}

int main() {
    // Run all test functions
    test_boundary_comparisons();
    test_saturation_arithmetic();
    test_struct_operations();
    test_mixed_precision_conversions();
    test_edge_cases();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
