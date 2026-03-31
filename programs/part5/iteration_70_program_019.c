#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons_sat() {
    // Saturated fixed-point types
    _Sat unsigned long _Accum usa_max = 0xFFFFFFFFFFFFFFFFP-32;  // Max value
    _Sat signed long _Accum sa_min = -0x8000000000000000P-32;    // Min value
    _Sat signed long _Accum sa_mid = 0x4000000000000000P-32;     // Mid value
    
    // Fixed-point types without saturation
    unsigned short _Fract usf_val = 0.95r;
    signed _Fract sf_val = -0.5r;
    
    // Mixed precision operations forcing range analysis
    _Sat signed long _Accum mixed1 = (_Sat signed long _Accum)usf_val * sa_mid;
    _Sat signed long _Accum mixed2 = (_Sat signed long _Accum)sf_val * usa_max;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (mixed1 > mixed2) {
        // This comparison should force range analysis for a_high.sgt(max_r)
        result += 1;
    }
    
    // Near-boundary values
    _Sat unsigned long _Accum near_max = usa_max * 0.999999999r;
    _Sat signed long _Accum near_min = sa_min / 0.999999999r;
    
    // Complex condition similar to uncovered code
    if (near_max < usa_max && near_min > sa_min) {
        result += 2;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed long _Accum a = 0x7000000000000000P-32;
    _Sat signed long _Accum b = 0x7000000000000000P-32;
    _Sat signed long _Accum c;
    
    // Use builtin overflow checks with fixed-point
    int overflow = 0;
    // Note: __builtin_mul_overflow doesn't directly support fixed-point,
    // but we can simulate with integer operations
    unsigned long long a_int = (unsigned long long)a;
    unsigned long long b_int = (unsigned long long)b;
    
    if (__builtin_mul_overflow(a_int, b_int, (unsigned long long*)&c)) {
        overflow = 1;
    }
    
    // Force range comparison
    _Sat signed long _Accum max_val = 0x7FFFFFFFFFFFFFFFP-32;
    if (c > max_val || (c == max_val && overflow)) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_range() {
    // Different fractional bit configurations
    unsigned short _Fract usf1 = 0.8r;
    unsigned short _Fract usf2 = 0.7r;
    signed _Fract sf1 = -0.3r;
    signed _Fract sf2 = 0.9r;
    
    // Operations that force precision conversions
    _Sat signed long _Accum acc1 = (_Sat signed long _Accum)usf1 * (_Sat signed long _Accum)sf1;
    _Sat signed long _Accum acc2 = (_Sat signed long _Accum)usf2 * (_Sat signed long _Accum)sf2;
    
    // Division near boundaries
    _Sat signed long _Accum div_result = acc1 / (acc2 + 0.000000001r);
    
    // Comparisons that should exercise the uncovered comparison logic
    _Sat signed long _Accum threshold = 0x4000000000000000P-32;
    
    if (div_result > threshold) {
        result += 8;
    } else if (div_result < -threshold) {
        result += 16;
    }
    
    // Zero and one comparisons
    if (div_result == 0) {
        result += 32;
    }
}

// Struct with fixed-point members
struct FixedPointContainer {
    _Sat unsigned short _Accum us_acc;
    signed _Fract sf;
    _Sat signed long _Accum sl_acc;
    unsigned short _Fract usf;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container[4];
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container[i].us_acc = i * 0.25r;
        container[i].sf = (i % 2 == 0) ? 0.5r : -0.5r;
        container[i].sl_acc = (_Sat signed long _Accum)container[i].us_acc * 
                             (_Sat signed long _Accum)container[i].sf;
        container[i].usf = 0.9r - (i * 0.1r);
    }
    
    // Perform operations on array elements
    _Sat signed long _Accum total = 0;
    for (int i = 0; i < 4; i++) {
        total += container[i].sl_acc * (_Sat signed long _Accum)container[i].usf;
        
        // Conditional based on fixed-point comparison
        if (container[i].us_acc > 0.5r && container[i].sf < 0) {
            result += 64;
        }
    }
    
    // Final boundary check
    _Sat signed long _Accum max_range = 0x7FFFFFFFFFFFFFFFP-32;
    if (total > max_range || total < -max_range) {
        result += 128;
    }
}

__attribute__((optimize("O3")))
void test_edge_cases() {
    // Values very close to boundaries
    _Sat unsigned long _Accum epsilon = 0.0000000000000001r;
    _Sat unsigned long _Accum almost_max = 0xFFFFFFFFFFFFFFFFP-32 - epsilon;
    _Sat signed long _Accum almost_min = -0x8000000000000000P-32 + epsilon;
    
    // Operations that might trigger the exact uncovered condition
    // a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    _Sat signed long _Accum test_val = almost_max * 1.0000000000000001r;
    
    // This should force max_r/max_s initialization and comparison
    _Sat signed long _Accum reference = 0x7FFFFFFFFFFFFFFFP-32;
    
    if (test_val > reference) {
        result += 256;
    }
    
    // Test with zero
    _Sat signed long _Accum zero_test = almost_min * 0.0r;
    if (zero_test == 0) {
        result += 512;
    }
}

int main() {
    // Initialize with different boundary values
    _Sat unsigned long _Accum usa_boundary = 0xFFFFFFFFFFFFFFFFP-32;
    _Sat signed long _Accum sa_boundary = -0x8000000000000000P-32;
    unsigned short _Fract usf_boundary = 0.9999r;
    signed _Fract sf_boundary = -0.9999r;
    
    // Call test functions
    test_boundary_comparisons_sat();
    test_overflow_checks();
    test_mixed_precision_range();
    test_struct_operations();
    test_edge_cases();
    
    // Additional direct comparisons to force range analysis
    _Sat signed long _Accum a = (_Sat signed long _Accum)usf_boundary * sa_boundary;
    _Sat signed long _Accum b = (_Sat signed long _Accum)sf_boundary * usa_boundary;
    
    // Complex condition similar to uncovered code
    if (a > b) {
        result += 1024;
    } else if (a == b) {
        result += 2048;
    } else {
        result += 4096;
    }
    
    // Loop with fixed-point condition
    _Sat signed long _Accum accumulator = 0;
    for (int i = 0; i < 10; i++) {
        accumulator += (_Sat signed long _Accum)(i * 0.1r);
        
        // Conditional that depends on accumulated value
        if (accumulator > 2.5r || accumulator < -2.5r) {
            result += 8192;
            break;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
