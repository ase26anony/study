#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization
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
    _Sat unsigned long _Accum ula_max = 255.999999999999999999ulk;
    
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999999999999lk;
    
    // Mixed precision operations to force range analysis
    _Sat signed long _Accum mixed1 = (_Sat signed long _Accum)usf_max * sla_max;
    _Sat signed long _Accum mixed2 = (_Sat signed long _Accum)ssf_min * ula_max;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (mixed1 > sla_max) {
        result |= 1;
    }
    
    if (mixed2 < sla_min) {
        result |= 2;
    }
    
    // Equality comparisons at boundaries
    _Sat signed short _Fract boundary_val = 0.9999r;
    if (boundary_val == ssf_max) {
        result |= 4;
    }
    
    // Near-boundary operations
    _Sat unsigned long _Accum near_max = ula_max * 0.999999999999999999ulk;
    if (near_max > ula_max / 2) {
        result |= 8;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 200.0lk;
    _Sat signed long _Accum c;
    
    // Use builtins to check for overflow
    int overflow = __builtin_mul_overflow(a, b, &c);
    if (overflow) {
        result |= 16;
    }
    
    _Sat unsigned short _Fract x = 0.9ur;
    _Sat unsigned short _Fract y = 0.9ur;
    _Sat unsigned short _Fract z;
    
    overflow = __builtin_add_overflow(x, y, &z);
    if (overflow) {
        result |= 32;
    }
    
    // Division near boundaries
    _Sat signed long _Accum div_test = sla_max / 0.5lk;
    if (div_test > sla_max) {
        result |= 64;
    }
}

// Struct containing fixed-point values
struct FixedPointStruct {
    _Sat signed short _Fract f1;
    _Sat unsigned long _Accum f2;
    _Sat signed long _Accum f3;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointStruct fps[4];
    
    // Initialize array with boundary values
    for (int i = 0; i < 4; i++) {
        fps[i].f1 = (i % 2) ? 0.9999r : -0.9999r;
        fps[i].f2 = (i * 64.0ulk);
        fps[i].f3 = (i - 2) * 100.0lk;
    }
    
    // Perform operations on struct members
    _Sat signed long _Accum sum = 0.0lk;
    for (int i = 0; i < 4; i++) {
        sum += (_Sat signed long _Accum)fps[i].f1 * fps[i].f3;
        
        // Boundary comparison within loop
        if (fps[i].f2 > 128.0ulk) {
            result |= 128;
        }
    }
    
    // Final boundary check
    if (sum > 1000.0lk || sum < -1000.0lk) {
        result |= 256;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test various precision conversions
    _Sat unsigned short _Fract usf = 0.75ur;
    _Sat signed short _Fract ssf = -0.5r;
    _Sat unsigned long _Accum ula = 100.5ulk;
    _Sat signed long _Accum sla = -100.5lk;
    
    // Mixed operations forcing conversions
    _Sat signed long _Accum conv1 = (_Sat signed long _Accum)usf * sla;
    _Sat signed long _Accum conv2 = (_Sat signed long _Accum)ssf * ula;
    
    // Comparisons that should exercise range analysis
    if (conv1 > 0.0lk && conv2 < 0.0lk) {
        result |= 512;
    }
    
    // Test with different fractional bits configurations
    _Sat signed long _Accum scaled = sla * 0.25lk;
    if (scaled > sla_min / 4 && scaled < sla_max / 4) {
        result |= 1024;
    }
}

__attribute__((optimize("O3")))
void test_edge_cases() {
    // Zero and near-zero cases
    _Sat signed short _Fract zero_test = 0.0r;
    _Sat signed short _Fract epsilon = 0.0001r;
    
    if (zero_test == 0.0r && epsilon > 0.0r) {
        result |= 2048;
    }
    
    // Maximum saturation
    _Sat unsigned long _Accum max_sat = ula_max + 100.0ulk;
    if (max_sat == ula_max) {  // Should saturate
        result |= 4096;
    }
    
    // Minimum saturation
    _Sat signed long _Accum min_sat = sla_min - 100.0lk;
    if (min_sat == sla_min) {  // Should saturate
        result |= 8192;
    }
    
    // Division by values near zero
    _Sat signed long _Accum div_by_small = sla_max / 0.0000001lk;
    if (div_by_small > sla_max) {
        result |= 16384;
    }
}

int main() {
    // Run all test functions
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_edge_cases();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int check = result;
    if (check > 0) {
        printf("Some conditions triggered\n");
    }
    
    return 0;
}
