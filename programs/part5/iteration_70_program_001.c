#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from removing computations
volatile int result = 0;

// Function with aggressive optimization to ensure fixed-point contraction
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point variables at type boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    _Sat signed short _Fract sf_min = -1.0r;
    _Sat signed short _Fract sf_max = 0.9999r;
    
    // Mixed precision operations
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 255.999999999ulk;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.999999999lk;
    
    // Force range analysis with boundary value arithmetic
    _Sat unsigned short _Fract usf1 = usf_max * 0.5ur;
    _Sat signed short _Fract sf1 = sf_max * 0.5r;
    
    // Mixed-type operations that require precision conversions
    _Sat signed long _Accum sla1 = (_Sat signed long _Accum)sf_max * 100.0lk;
    _Sat unsigned long _Accum ula1 = (_Sat unsigned long _Accum)usf_max * 200.0ulk;
    
    // Control flow dependent on fixed-point ranges
    if (usf1 > 0.25ur) {
        result += 1;
    }
    
    if (sf1 < 0.0r) {
        result += 2;
    }
    
    // Complex boundary comparison
    if (sla1 > 50.0lk || (sla1 == 50.0lk && ula1 > 100.0ulk)) {
        result += 4;
    }
    
    // Near-boundary operations
    _Sat signed short _Fract sf_near_max = sf_max * 0.9999r;
    _Sat signed short _Fract sf_near_min = sf_min * 0.9999r;
    
    if (sf_near_max > 0.9r) {
        result += 8;
    }
    
    if (sf_near_min < -0.9r) {
        result += 16;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 200.0lk;
    _Sat signed long _Accum c;
    
    // Use built-in overflow checks with fixed-point
    int overflow = __builtin_mul_overflow((long)a, (long)b, (long*)&c);
    if (overflow) {
        result += 32;
    }
    
    _Sat unsigned short _Fract x = 0.8ur;
    _Sat unsigned short _Fract y = 0.9ur;
    _Sat unsigned short _Fract z;
    
    // This should saturate rather than overflow
    z = x * y;
    if (z > 0.7ur) {
        result += 64;
    }
}

// Struct containing fixed-point values
struct FixedPointData {
    _Sat signed short _Fract fract_data[4];
    _Sat unsigned long _Accum accum_data[2];
    int flags;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointData data;
    
    // Initialize array with boundary values
    data.fract_data[0] = -1.0r;  // min
    data.fract_data[1] = 0.0r;   // zero
    data.fract_data[2] = 0.5r;   // mid
    data.fract_data[3] = 0.9999r; // near max
    
    data.accum_data[0] = 0.0ulk;  // min
    data.accum_data[1] = 255.999999999ulk;  // near max
    
    // Operations on struct members
    for (int i = 0; i < 4; i++) {
        data.fract_data[i] = data.fract_data[i] * 0.5r;
        
        // Control flow based on array values
        if (data.fract_data[i] > 0.0r) {
            data.flags |= (1 << i);
        }
    }
    
    // Mixed array operations
    _Sat signed long _Accum sum = 0.0lk;
    for (int i = 0; i < 2; i++) {
        sum += (_Sat signed long _Accum)data.accum_data[i];
    }
    
    if (sum > 100.0lk) {
        result += 128;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test various precision conversions
    _Sat unsigned short _Fract usf = 0.75ur;
    _Sat signed short _Fract sf = -0.5r;
    
    // Convert to higher precision
    _Sat unsigned long _Accum ula = (_Sat unsigned long _Accum)usf;
    _Sat signed long _Accum sla = (_Sat signed long _Accum)sf;
    
    // Operations with different precisions
    _Sat signed long _Accum mixed1 = sla * ula;
    _Sat signed long _Accum mixed2 = (_Sat signed long _Accum)usf * sf;
    
    // Boundary comparisons after conversions
    if (ula > 0.5ulk && sla < 0.0lk) {
        result += 256;
    }
    
    // Test division near boundaries
    _Sat signed short _Fract div_test = 0.9999r / 2.0r;
    if (div_test > 0.49r) {
        result += 512;
    }
}

__attribute__((optimize("O3")))
void test_edge_cases() {
    // Operations that should trigger max/min comparisons
    _Sat signed short _Fract zero = 0.0r;
    _Sat signed short _Fract one = 1.0r;  // Will saturate to max
    
    // These operations should test the boundary comparison logic
    _Sat signed short _Fract test1 = zero * one;
    _Sat signed short _Fract test2 = one / 0.5r;  // Should saturate
    
    // Force comparisons with computed maxima
    _Sat signed long _Accum max_val = 255.999999999lk;
    _Sat signed long _Accum test_val = 200.0lk * 1.5lk;  // Should saturate
    
    if (test1 == 0.0r) {
        result += 1024;
    }
    
    // This should exercise the a_high.sgt(max_r) type logic
    if (test_val > 300.0lk || (test_val == 300.0lk && test2 > 0.5r)) {
        result += 2048;
    }
}

int main() {
    // Run all tests to exercise fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_edge_cases();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
