#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to force range analysis
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned long _Accum max_accum = 0.99999999999999999999UK;
    _Sat unsigned short _Fract max_fract = 0.9999999UHR;
    _Sat signed long _Accum min_accum = -0.99999999999999999999K;
    signed short _Fract mid_fract = 0.5HR;
    
    // Mixed precision operations to force range analysis
    _Sat unsigned long _Accum mixed_result = max_accum * (_Accum)max_fract;
    _Sat signed long _Accum signed_result = min_accum / (_Accum)mid_fract;
    
    // Boundary value comparisons - should trigger the uncovered code
    if (mixed_result > (_Accum)0.9) {
        result += 1;
    }
    
    if (signed_result < (_Accum)-0.5) {
        result += 2;
    }
    
    // Force comparisons at exact boundaries
    _Sat unsigned long _Accum boundary_test = max_accum;
    if (boundary_test == (_Accum)0.99999999999999999999UK) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    // Use built-in overflow checks with fixed-point types
    _Sat unsigned long _Accum a = 0.8UK;
    _Sat unsigned long _Accum b = 0.9UK;
    _Sat unsigned long _Accum overflow_result;
    
    // These should interact with value range analysis
    if (__builtin_mul_overflow(a, b, &overflow_result)) {
        result += 8;
    }
    
    _Sat signed long _Accum c = -0.8K;
    _Sat signed long _Accum d = 1.2K;
    _Sat signed long _Accum add_result;
    
    if (__builtin_add_overflow(c, d, &add_result)) {
        result += 16;
    }
    
    // Force comparisons that might trigger the uncovered condition
    if (overflow_result > (_Accum)0.5 || 
        (overflow_result == (_Accum)0.5 && add_result > (_Accum)0)) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract f1;
    signed long _Accum f2;
    _Sat unsigned long _Accum f3;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container[3];
    
    // Initialize with boundary values
    for (int i = 0; i < 3; i++) {
        container[i].f1 = (i == 0) ? 0.0UHR : 0.9999999UHR;
        container[i].f2 = (i == 1) ? -0.99999999999999999999K : 0.5K;
        container[i].f3 = (i == 2) ? 0.99999999999999999999UK : 0.25UK;
    }
    
    // Perform operations on array elements
    _Sat unsigned long _Accum array_result = 0.0UK;
    
    for (int i = 0; i < 3; i++) {
        // Mixed type operations within struct
        array_result += container[i].f3 * (_Accum)container[i].f1;
        
        // Comparison that should trigger range analysis
        if (container[i].f2 > (_Accum)0) {
            array_result -= (_Accum)container[i].f1;
        }
    }
    
    // Final boundary comparison
    if (array_result > (_Accum)0.75 || 
        array_result < (_Accum)-0.75) {
        result += 64;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test different fractional bit configurations
    unsigned short _Fract usf = 0.75UHR;
    signed long _Fract slf = -0.25LK;
    _Sat unsigned _Accum usa = 0.5UHK;
    
    // Mixed precision conversions and operations
    _Sat signed long _Accum conv1 = (_Accum)usf * 2.0K;
    _Sat unsigned long _Accum conv2 = (_Accum)slf + 1.0UK;
    _Sat signed _Accum conv3 = (_Accum)usa / 0.5HK;
    
    // Complex comparison chain to trigger the uncovered logic
    if (conv1 > (_Accum)1.0 || 
        (conv1 == (_Accum)1.0 && conv2 > (_Accum)0.5)) {
        result += 128;
    }
    
    if (conv3 < (_Accum)-1.0 || 
        (conv3 == (_Accum)-1.0 && conv2 < (_Accum)0)) {
        result += 256;
    }
}

__attribute__((optimize("O3")))
void test_loop_dependent_ranges() {
    _Sat unsigned long _Accum accumulator = 0.0UK;
    signed short _Fract increment = 0.1HR;
    
    // Loop with fixed-point condition
    for (int i = 0; i < 10; i++) {
        accumulator += (_Accum)increment;
        
        // Condition that depends on accumulated value
        if (accumulator > (_Accum)0.5) {
            // Nested condition to force complex range analysis
            if (accumulator < (_Accum)0.9 || 
                (accumulator == (_Accum)0.9 && i > 5)) {
                result += 512;
                break;
            }
        }
    }
    
    // Final boundary check
    if (accumulator == (_Accum)1.0UK) {
        result += 1024;
    }
}

int main() {
    // Execute all test functions to cover different code paths
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_loop_dependent_ranges();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
