#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from removing critical code
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types at boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 1.0ur - 0.0001ur;
    
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 1.0r - 0.0001r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.9999999999999999ulk;
    
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.9999999999999999lk;
    
    // Mixed precision operations that should trigger range analysis
    _Sat signed long _Accum mixed1 = (_Sat signed long _Accum)usf_max * sla_max;
    _Sat unsigned long _Accum mixed2 = (_Sat unsigned long _Accum)ssf_max * ula_max;
    
    // Boundary comparisons - these should trigger the uncovered comparison logic
    if (mixed1 > sla_max) {
        result |= 1;
    }
    
    if (mixed2 < ula_max) {
        result |= 2;
    }
    
    // Near-boundary operations
    _Sat signed short _Fract near_bound = ssf_max * 0.999r;
    if (near_bound == ssf_max) {
        result |= 4;
    }
    
    // Division near boundaries
    _Sat unsigned long _Accum div_result = ula_max / 2.0ulk;
    if (div_result > 127.0ulk) {
        result |= 8;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 200.0lk;
    _Sat signed long _Accum c;
    
    // Use built-in overflow checks with fixed-point
    int overflow = 0;
    overflow |= __builtin_mul_overflow((long)a, (long)b, (long*)&c);
    
    if (overflow) {
        result |= 16;
    }
    
    // Test addition overflow
    _Sat signed long _Accum d = 250.0lk;
    _Sat signed long _Accum e = 250.0lk;
    _Sat signed long _Accum f;
    
    overflow |= __builtin_add_overflow((long)d, (long)e, (long*)&f);
    
    if (overflow) {
        result |= 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Sat unsigned long _Accum ula_value;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = i * 0.25ur;
        container.sla_array[i] = (i - 2) * 100.0lk;
    }
    container.ula_value = 255.9999999999999999ulk;
    
    // Operations on struct members
    _Sat signed long _Accum sum = 0.0lk;
    for (int i = 0; i < 4; i++) {
        sum += container.sla_array[i] * (_Sat signed long _Accum)container.usf_array[i];
    }
    
    // Comparison that should trigger range analysis
    if (sum > container.sla_array[3]) {
        result |= 64;
    }
    
    // Array boundary comparison
    if (container.ula_value == 255.9999999999999999ulk) {
        result |= 128;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Test various conversions that require range analysis
    _Sat unsigned short _Fract usf = 0.75ur;
    _Sat signed long _Accum sla = 100.0lk;
    
    // Mixed precision multiplication
    _Sat signed long _Accum mixed = sla * (_Sat signed long _Accum)usf;
    
    // These comparisons should exercise the uncovered comparison logic
    if (mixed < 75.0lk) {
        result |= 256;
    }
    
    if (mixed > 74.0lk) {
        result |= 512;
    }
    
    // Test with values near type boundaries
    _Sat unsigned long _Accum ula_near_max = 255.9999999999999999ulk;
    _Sat unsigned short _Fract usf_half = 0.5ur;
    
    _Sat unsigned long _Accum scaled = ula_near_max * (_Sat unsigned long _Accum)usf_half;
    
    // Boundary comparison
    if (scaled > 127.0ulk) {
        result |= 1024;
    }
}

__attribute__((optimize("O3")))
void test_control_flow_ranges() {
    // Create control flow that depends on fixed-point ranges
    _Sat signed long _Accum accumulator = 0.0lk;
    
    for (int i = 0; i < 10; i++) {
        _Sat signed short _Fract increment = 0.1r;
        accumulator += (_Sat signed long _Accum)increment * 10.0lk;
        
        // Conditional based on accumulator value
        if (accumulator > 5.0lk && accumulator < 10.0lk) {
            result |= 2048;
        }
        
        // Boundary check
        if (accumulator >= 0.0lk && accumulator <= 100.0lk) {
            result |= 4096;
        }
    }
    
    // Final boundary comparison
    if (accumulator == 10.0lk) {
        result |= 8192;
    }
}

int main() {
    // Run all tests to exercise the fixed-value range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_mixed_precision_conversions();
    test_control_flow_ranges();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
