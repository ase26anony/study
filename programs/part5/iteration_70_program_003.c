#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent dead code elimination
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point variables at boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    _Sat signed short _Fract ssf_min = -1.0r;
    _Sat signed short _Fract ssf_max = 0.9999r;
    
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Mixed precision operations that should trigger range analysis
    signed long _Accum mixed_result;
    
    // Operation that approaches upper boundary
    mixed_result = (_Accum)usf_max * (_Accum)ula_max;
    
    // Force boundary comparison
    if (mixed_result > sla_max) {
        result += 1;
    }
    
    // Operation that approaches lower boundary
    mixed_result = (_Accum)ssf_min * (_Accum)ula_max;
    
    if (mixed_result < sla_min) {
        result += 2;
    }
    
    // Equality comparison at boundary
    signed short _Fract boundary_val = 0.5r;
    for (int i = 0; i < 10; i++) {
        boundary_val = boundary_val * 0.9r;
        
        if (boundary_val == 0.0r) {
            result += 4;
            break;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat unsigned long _Accum a = 200.0ulk;
    _Sat unsigned long _Accum b = 100.0ulk;
    _Sat unsigned long _Accum c;
    
    // Use builtin overflow checks with fixed-point
    int overflow;
    
    // Multiplication near overflow boundary
    overflow = __builtin_mul_overflow((unsigned long)a, (unsigned long)b, 
                                     (unsigned long*)&c);
    if (overflow) {
        result += 8;
    }
    
    // Addition near overflow boundary
    a = 250.0ulk;
    b = 10.0ulk;
    overflow = __builtin_add_overflow((unsigned long)a, (unsigned long)b,
                                     (unsigned long*)&c);
    if (overflow) {
        result += 16;
    }
    
    // Division creating boundary conditions
    _Sat signed short _Fract dividend = 0.9999r;
    _Sat signed short _Fract divisor = 0.5r;
    _Sat signed short _Fract quotient = dividend / divisor;
    
    if (quotient > 1.0r) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Fract middle_values[2];
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    container.usf_array[0] = 0.0ur;
    container.usf_array[1] = 0.25ur;
    container.usf_array[2] = 0.75ur;
    container.usf_array[3] = 0.9999ur;
    
    container.sla_array[0] = -256.0lk;
    container.sla_array[1] = -128.0lk;
    container.sla_array[2] = 128.0lk;
    container.sla_array[3] = 255.999999999lk;
    
    container.middle_values[0] = 0.5r;
    container.middle_values[1] = -0.5r;
    
    // Perform operations on array elements
    for (int i = 0; i < 3; i++) {
        // Mixed type operations
        _Accum temp = (_Accum)container.usf_array[i] * 
                     (_Accum)container.sla_array[i+1];
        
        // Boundary comparison that should trigger the uncovered code
        if (temp > container.sla_array[3]) {
            result += 64;
        }
        
        if (temp < container.sla_array[0]) {
            result += 128;
        }
        
        // Equality comparison at specific boundary
        if (container.usf_array[i] == 0.0ur && 
            container.sla_array[i+1] == 0.0lk) {
            result += 256;
        }
    }
    
    // Test precision conversion with boundary values
    _Sat unsigned short _Fract usf_boundary = 0.9999ur;
    _Sat signed long _Accum converted = (_Sat signed long _Accum)usf_boundary;
    
    // This should trigger max_s.zext() logic
    if (converted > 0.9lk) {
        result += 512;
    }
}

__attribute__((optimize("O3")))
void test_min_max_boundaries() {
    // Direct boundary value testing
    _Sat unsigned long _Accum max_ula = 255.999999999ulk;
    _Sat unsigned long _Accum min_ula = 0.0ulk;
    
    _Sat signed long _Accum max_sla = 255.999999999lk;
    _Sat signed long _Accum min_sla = -256.0lk;
    
    // Create values that should trigger min_s shift/extend operations
    _Sat signed short _Fract near_zero = 0.0001r;
    _Sat signed short _Fract near_one = 0.9999r;
    
    // Operations that create boundary conditions
    for (int i = 0; i < 5; i++) {
        near_zero = near_zero / 2.0r;
        near_one = near_one * 1.1r;
        
        // These comparisons should exercise the uncovered comparison logic
        _Accum test_val = (_Accum)near_zero * (_Accum)max_ula;
        
        if (test_val > max_sla) {
            result += 1024;
        }
        
        if (test_val < min_sla) {
            result += 2048;
        }
        
        // Test equality at boundaries
        _Accum boundary_test = (_Accum)near_one * (_Accum)max_sla;
        if (boundary_test == max_sla) {
            result += 4096;
        }
    }
    
    // Test with negative boundaries
    _Sat signed short _Fract negative_boundary = -0.9999r;
    _Accum negative_test = (_Accum)negative_boundary * (_Accum)min_sla;
    
    // This should trigger a_high.sgt(max_r) type comparisons
    if (negative_test > 0.0lk) {
        result += 8192;
    }
}

int main() {
    // Reset result
    result = 0;
    
    // Run all tests to trigger fixed-value range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_min_max_boundaries();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int check = result;
    if (check > 10000) {
        printf("High boundary reached\n");
    } else if (check == 0) {
        printf("Zero boundary case\n");
    }
    
    return 0;
}
