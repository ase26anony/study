#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from removing computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Fixed-point types with different sizes and signedness
    _Sat unsigned short _Fract usf1 = 0.5r;
    _Sat unsigned short _Fract usf2 = 0.75r;
    _Sat signed short _Fract ssf1 = -0.5r;
    _Sat signed short _Fract ssf2 = 0.5r;
    
    // Accumulator types with different precisions
    _Sat unsigned long _Accum ula1 = 10.0k;
    _Sat unsigned long _Accum ula2 = 20.0k;
    _Sat signed long _Accum sla1 = -10.0k;
    _Sat signed long _Accum sla2 = 10.0k;
    
    // Mixed precision operations to force range analysis
    // These operations should trigger precision conversions
    _Sat signed long _Accum mixed1 = sla1 * (_Sat signed long _Accum)usf1;
    _Sat signed long _Accum mixed2 = sla2 * (_Sat signed long _Accum)usf2;
    
    // Boundary value comparisons - should trigger the uncovered comparison logic
    if (mixed1 > mixed2) {
        result += 1;
    }
    
    // Approach maximum boundary
    _Sat unsigned short _Fract max_usf = 0.9999r;
    _Sat unsigned short _Fract min_usf = 0.0001r;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        max_usf = max_usf * 0.999r;
        min_usf = min_usf * 1.01r;
        
        // Conditional dependent on boundary values
        if (max_usf > 0.5r && min_usf < 0.5r) {
            result += 2;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    // Values near boundaries to test overflow
    _Sat signed short _Fract near_max = 0.99r;
    _Sat signed short _Fract near_min = -0.99r;
    
    // Use builtins for overflow detection
    _Sat signed short _Fract mul_result;
    int overflow = __builtin_mul_overflow(near_max, near_max, &mul_result);
    
    if (overflow) {
        result += 4;
    }
    
    // Test addition near boundaries
    _Sat signed short _Fract add_result;
    overflow = __builtin_add_overflow(near_max, 0.1r, &add_result);
    
    if (overflow) {
        result += 8;
    }
    
    // Division operations that can produce extreme values
    _Sat signed short _Fract small = 0.001r;
    _Sat signed short _Fract div_result = near_max / small;
    
    // This comparison should trigger boundary analysis
    if (div_result > 100.0r || div_result < -100.0r) {
        result += 16;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf;
    _Sat signed short _Fract ssf;
    _Sat unsigned long _Accum ula;
    _Sat signed long _Accum sla;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container[3];
    
    // Initialize with boundary values
    container[0].usf = 0.0r;
    container[0].ssf = -1.0r;
    container[0].ula = 0.0k;
    container[0].sla = -100.0k;
    
    container[1].usf = 0.5r;
    container[1].ssf = 0.0r;
    container[1].ula = 50.0k;
    container[1].sla = 0.0k;
    
    container[2].usf = 0.9999r;
    container[2].ssf = 0.9999r;
    container[2].ula = 100.0k;
    container[2].sla = 100.0k;
    
    // Perform operations on array elements
    for (int i = 0; i < 2; i++) {
        // Mixed operations between array elements
        _Sat signed long _Accum temp = container[i].sla * (_Sat signed long _Accum)container[i+1].usf;
        
        // Comparison that should trigger range analysis
        if (temp > container[i].sla || temp < container[i+1].sla) {
            result += 32;
        }
        
        // Division with potential for extreme values
        if (container[i].usf > 0.0r) {
            _Sat signed long _Accum div_temp = container[i].sla / (_Sat signed long _Accum)container[i].usf;
            
            // Boundary comparison
            if (div_temp > 1000.0k || div_temp < -1000.0k) {
                result += 64;
            }
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries() {
    // Test with values at exact boundaries
    _Sat unsigned short _Fract zero = 0.0r;
    _Sat unsigned short _Fract max_val = 0.999999r;  // As close to 1.0 as possible
    
    _Sat signed short _Fract neg_max = -1.0r;
    _Sat signed short _Fract pos_max = 0.999999r;
    
    // Operations that test boundary conditions
    _Sat unsigned short _Fract boundary_test = zero * max_val;
    _Sat signed short _Fract signed_boundary_test = neg_max * pos_max;
    
    // These comparisons should specifically trigger the uncovered code
    // by creating conditions where a_high.sgt(max_r) or similar comparisons
    // are evaluated
    if (boundary_test == 0.0r) {
        result += 128;
    }
    
    if (signed_boundary_test < 0.0r) {
        result += 256;
    }
    
    // Create a scenario that might trigger the exact comparison pattern
    // a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    _Sat signed long _Accum large_val = 500.0k;
    _Sat signed long _Accum threshold = 100.0k;
    
    // Multiple comparisons to exercise the logic
    for (int i = 0; i < 5; i++) {
        large_val = large_val * 1.1k;
        
        if (large_val > threshold * 10.0k) {
            result += 512;
            threshold = threshold * 1.5k;
        }
    }
}

int main() {
    // Initialize result
    result = 0;
    
    // Run all test functions to exercise different aspects of fixed-point
    // range analysis and boundary comparisons
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_extreme_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile store
    volatile int final_result = result;
    
    return 0;
}
