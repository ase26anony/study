#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point variables at type boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 0.9999r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Mixed precision operations that force range analysis
    signed long _Accum mixed_result;
    
    // Operation that should approach maximum boundary
    mixed_result = (_Sat signed long _Accum)(ssf_max * ula_max);
    
    // Control flow dependent on fixed-point ranges
    if (mixed_result > sla_max) {
        // This branch should be taken if overflow occurs
        result |= 1;
    }
    
    // Operation that should approach minimum boundary
    mixed_result = (_Sat signed long _Accum)(ssf_min * ula_max);
    
    if (mixed_result < sla_min) {
        // This branch should be taken if underflow occurs
        result |= 2;
    }
    
    // Test equality at boundaries
    _Sat signed short _Fract boundary_test = 0.5r;
    
    // Loop with fixed-point condition
    for (int i = 0; i < 10; i++) {
        boundary_test = boundary_test * 2.0r;
        
        // Force boundary comparison
        if (boundary_test >= ssf_max) {
            result |= 4;
            boundary_test = ssf_min;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 200.0lk;
    _Sat signed long _Accum c;
    
    // Use built-in overflow checks with fixed-point
    int overflow;
    
    // Multiplication that should overflow
    overflow = __builtin_mul_overflow((long)a, (long)b, (long*)&c);
    if (overflow) {
        result |= 8;
    }
    
    // Addition that should saturate
    _Sat signed short _Fract f1 = 0.9r;
    _Sat signed short _Fract f2 = 0.9r;
    _Sat signed short _Fract sum = f1 + f2;
    
    if (sum >= 1.0r) {
        result |= 16;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Sat unsigned long _Accum ula_array[4];
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = i * 0.25ur;
        container.sla_array[i] = i * 100.0lk;
        container.ula_array[i] = i * 100.0ulk;
    }
    
    // Perform operations on array elements
    _Sat signed long _Accum total = 0.0lk;
    
    for (int i = 0; i < 4; i++) {
        // Mixed type operation
        total += container.sla_array[i] * container.usf_array[i];
        
        // Boundary check
        if (container.ula_array[i] > 200.0ulk) {
            result |= 32;
        }
    }
    
    // Final boundary comparison
    if (total > 500.0lk || total < -500.0lk) {
        result |= 64;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test conversions between different fixed-point types
    _Sat unsigned short _Fract usf_val = 0.75ur;
    _Sat signed long _Accum sla_val;
    
    // Precision conversion that should trigger range analysis
    sla_val = (_Sat signed long _Accum)usf_val * 300.0lk;
    
    // Multiple boundary comparisons
    if (sla_val > 200.0lk) {
        result |= 128;
    }
    
    if (sla_val < -200.0lk) {
        result |= 256;
    }
    
    // Test zero boundary
    _Sat signed short _Fract zero_test = 0.0r;
    zero_test = zero_test - 0.1r;
    
    if (zero_test < 0.0r) {
        result |= 512;
    }
    
    // Test near-maximum boundary
    _Sat unsigned long _Accum near_max = 255.9999998ulk;
    near_max = near_max + 0.0000001ulk;
    
    if (near_max >= 255.9999999ulk) {
        result |= 1024;
    }
}

int main() {
    printf("Starting fixed-point boundary tests...\n");
    
    // Run all test functions
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code executes
    volatile int final_check = result;
    if (final_check > 0) {
        printf("Some boundary conditions were triggered.\n");
    }
    
    return 0;
}
