#include <stdio.h>
#include <stdint.h>

// Volatile to prevent optimization
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types with boundary values
    _Sat unsigned long _Accum max_accum = 0.99999999999999999999UK;
    _Sat signed long _Accum min_accum = -0.99999999999999999999K;
    unsigned short _Fract max_fract = 0.9999UHR;
    signed short _Fract min_fract = -0.9999HR;
    
    // Mixed precision operations to force range analysis
    signed long _Accum mixed_result = (_Accum)max_fract * (_Accum)min_accum;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (max_accum > (_Accum)0.5UK) {
        result += 1;
    }
    
    if (min_accum < (_Accum)-0.5K) {
        result += 2;
    }
    
    // Complex condition similar to uncovered code
    if (max_accum > (_Accum)0.75UK || 
        (max_accum == (_Accum)0.75UK && max_fract > (_Fract)0.5UHR)) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_overflow_operations() {
    _Sat unsigned short _Fract f1 = 0.8UHR;
    _Sat unsigned short _Fract f2 = 0.9UHR;
    _Sat signed long _Accum a1 = 0.8K;
    _Sat signed long _Accum a2 = 0.9K;
    
    // Operations that may overflow/saturate
    _Sat unsigned short _Fract f_mul = f1 * f2;
    _Sat signed long _Accum a_mul = a1 * a2;
    
    // Built-in overflow checks with fixed-point
    int overflow;
    _Sat signed long _Accum a_sum = __builtin_add_overflow(a1, a2, &overflow) ? 
                                   (_Sat signed long _Accum)0.99999999999999999999K : a1 + a2;
    
    // Range-dependent control flow
    if (f_mul > (_Fract)0.5UHR && a_mul > (_Accum)0.5K) {
        result += 8;
    }
    
    if (overflow) {
        result += 16;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned long _Accum accum;
    unsigned short _Fract fract;
    signed short _Fract signed_fract;
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container = {
        .accum = 0.75UK,
        .fract = 0.6UHR,
        .signed_fract = -0.3HR
    };
    
    struct FixedPointContainer container2 = {
        .accum = 0.25UK,
        .fract = 0.4UHR,
        .signed_fract = 0.7HR
    };
    
    // Array of structs
    struct FixedPointContainer arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i].accum = (_Sat unsigned long _Accum)i * 0.25UK;
        arr[i].fract = (unsigned short _Fract)(i * 0.25UHR);
        arr[i].signed_fract = (signed short _Fract)(i * 0.25HR - 0.5HR);
    }
    
    // Operations on struct members
    _Sat unsigned long _Accum total_accum = container.accum + container2.accum;
    unsigned short _Fract total_fract = container.fract * container2.fract;
    
    // Complex boundary comparisons
    for (int i = 0; i < 4; i++) {
        if (arr[i].accum > (_Accum)0.5UK || 
            (arr[i].accum == (_Accum)0.5UK && arr[i].fract > (_Fract)0.25UHR)) {
            result += 32;
        }
    }
    
    if (total_accum > (_Accum)0.9UK && total_fract > (_Fract)0.2UHR) {
        result += 64;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Different fractional bit configurations
    unsigned _Fract uf1 = 0.7UR;
    signed _Fract sf1 = -0.7R;
    _Sat unsigned long _Accum ula1 = 0.99999999999999999999UK;
    _Sat signed long _Accum sla1 = -0.99999999999999999999K;
    
    // Mixed precision operations forcing conversions
    _Sat signed long _Accum conv1 = (_Sat signed long _Accum)uf1 * sla1;
    _Sat unsigned long _Accum conv2 = (_Sat unsigned long _Accum)sf1 * ula1;
    
    // Division operations that affect range analysis
    _Sat signed long _Accum div_result = conv1 / (_Sat signed long _Accum)0.5K;
    
    // Boundary value comparisons
    if (conv1 > (_Accum)0.0K || conv2 < (_Accum)0.0UK) {
        result += 128;
    }
    
    // Complex condition matching uncovered pattern
    if (div_result > (_Accum)0.75K || 
        (div_result == (_Accum)0.75K && uf1 > (_Fract)0.5UR)) {
        result += 256;
    }
}

__attribute__((optimize("O3")))
void test_extreme_values() {
    // Values at absolute boundaries
    _Sat unsigned long _Accum max_val = 0.99999999999999999999UK;
    _Sat unsigned long _Accum min_val = 0.0UK;
    _Sat signed long _Accum max_signed = 0.99999999999999999999K;
    _Sat signed long _Accum min_signed = -0.99999999999999999999K;
    
    // Operations that push boundaries
    _Sat unsigned long _Accum near_max = max_val * (_Sat unsigned long _Accum)0.99999999999999999999UK;
    _Sat signed long _Accum near_min = min_signed * (_Sat signed long _Accum)0.99999999999999999999K;
    
    // Built-in overflow operations
    int mul_overflow;
    _Sat unsigned long _Accum mul_result = __builtin_mul_overflow(
        max_val, (_Sat unsigned long _Accum)1.1UK, &mul_overflow) ? 
        max_val : max_val * (_Sat unsigned long _Accum)1.1UK;
    
    // Multiple boundary comparisons
    if (near_max > (_Accum)0.9UK) {
        result += 512;
    }
    
    if (near_min < (_Accum)-0.9K || mul_overflow) {
        result += 1024;
    }
    
    // Complex nested conditions
    if ((max_val > (_Accum)0.5UK && min_val < (_Accum)0.1UK) ||
        (max_signed > (_Accum)0.0K && min_signed < (_Accum)-0.5K)) {
        result += 2048;
    }
}

int main() {
    // Initialize volatile to prevent dead code elimination
    volatile int final_result = 0;
    
    // Execute all test functions
    test_boundary_comparisons();
    test_overflow_operations();
    test_struct_operations();
    test_mixed_precision_conversions();
    test_extreme_values();
    
    // Aggregate results
    final_result = result;
    
    // Print to prevent optimization
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
