#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point variables at type boundaries
    _Sat unsigned short _Fract usf_max = 0.9999r;  // Near maximum
    _Sat unsigned short _Fract usf_min = 0.0001r;  // Near minimum
    _Sat signed long _Accum sla_max = 0.999999999k;  // Near maximum
    _Sat signed long _Accum sla_min = -0.999999999k; // Near minimum
    
    // Mixed precision operations to force range analysis
    _Sat unsigned short _Fract usf_mid = 0.5r;
    _Sat signed long _Accum sla_mid = 0.5k;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication that can saturate
        _Sat unsigned short _Fract usf_prod = usf_mid * usf_max;
        _Sat signed long _Accum sla_prod = sla_mid * sla_max;
        
        // Division that can approach extremes
        _Sat unsigned short _Fract usf_div = usf_max / usf_min;
        _Sat signed long _Accum sla_div = sla_max / sla_min;
        
        // Control flow dependent on fixed-point ranges
        if (usf_prod > 0.9r) {
            result += 1;  // Should trigger when product is near max
        }
        
        if (sla_prod.sgt(0.8k) || (sla_prod == 0.8k && usf_prod.ugt(0.7r))) {
            result += 2;  // Direct comparison to trigger uncovered logic
        }
        
        // Adjust values for next iteration
        usf_mid = usf_mid * 1.1r;
        sla_mid = sla_mid * 1.1k;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Different fixed-point types to force precision conversions
    unsigned _Fract uf_val = 0.75r;
    signed _Accum sa_val = -0.25k;
    _Sat long _Fract lf_val = 0.999999r;
    
    // Mixed operations with overflow checks
    int overflow = 0;
    signed _Accum sa_result;
    
    // Use builtins to introduce overflow analysis
    if (__builtin_mul_overflow(sa_val, 2.0k, &sa_result)) {
        overflow = 1;
    }
    
    // Range-dependent control flow
    if (uf_val > 0.5r && sa_val < 0k) {
        result += 4;
    }
    
    // Operations that might saturate
    _Sat unsigned _Fract uf_sat = uf_val * 2.0r;
    if (uf_sat == 1.0r) {  // Should be saturated to max
        result += 8;
    }
}

// Struct containing fixed-point values to test range tracking through memory
struct FixedPointStruct {
    _Sat unsigned short _Fract usf;
    _Sat signed long _Accum sla;
    unsigned _Fract uf;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointStruct fps[4];
    
    // Initialize array with boundary values
    for (int i = 0; i < 4; i++) {
        fps[i].usf = i * 0.25r;
        fps[i].sla = (i - 2) * 0.5k;
        fps[i].uf = (3 - i) * 0.33r;
    }
    
    // Perform operations on struct members
    for (int i = 0; i < 3; i++) {
        // Multiplication that could saturate
        _Sat unsigned short _Fract usf_prod = fps[i].usf * fps[i+1].usf;
        _Sat signed long _Accum sla_sum = fps[i].sla + fps[i+1].sla;
        
        // Comparisons that should trigger range analysis
        if (usf_prod > 0.5r || sla_sum < -0.8k) {
            result += 16;
        }
        
        // More complex condition similar to uncovered code
        if (fps[i].sla.sgt(0.9k) || 
            (fps[i].sla == 0.9k && fps[i].usf.ugt(0.8r))) {
            result += 32;
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundary_conditions() {
    // Values at exact boundaries
    _Sat unsigned short _Fract usf_zero = 0.0r;
    _Sat unsigned short _Fract usf_one = 1.0r;
    _Sat signed long _Accum sla_pos_one = 1.0k;
    _Sat signed long _Accum sla_neg_one = -1.0k;
    
    // Operations designed to trigger specific comparison patterns
    _Sat signed long _Accum sla_test = 0.999999999k;  // Just below max
    
    // This should trigger the high/low part comparison logic
    for (int i = 0; i < 5; i++) {
        // Create values that approach boundaries from different directions
        _Sat signed long _Accum val1 = sla_test - (i * 0.1k);
        _Sat signed long _Accum val2 = sla_neg_one + (i * 0.1k);
        
        // Use builtin overflow operations
        int add_overflow = 0;
        _Sat signed long _Accum sum;
        if (__builtin_add_overflow(val1, val2, &sum)) {
            add_overflow = 1;
        }
        
        // Complex condition to match uncovered code pattern
        if (val1.sgt(0.9k) || (val1 == 0.9k && usf_one.ugt(0.5r))) {
            result += 64;
        }
        
        // Division that can produce extreme values
        if (val2 != 0k) {
            _Sat signed long _Accum div_result = val1 / val2;
            if (div_result < -0.5k) {
                result += 128;
            }
        }
    }
}

int main() {
    // Run all test functions to exercise different aspects of fixed-point analysis
    test_boundary_comparisons();
    test_mixed_precision_conversions();
    test_struct_operations();
    test_extreme_boundary_conditions();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
