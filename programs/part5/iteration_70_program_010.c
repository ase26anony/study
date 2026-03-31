#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent optimization from eliminating computations
volatile int result = 0;

// Function with aggressive optimization to force range analysis
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Initialize fixed-point variables at type boundaries
    _Sat unsigned short _Fract usf_min = 0.0ur;
    _Sat unsigned short _Fract usf_max = 0.9999ur;
    _Sat signed long _Accum sla_min = -256.0lk;
    _Sat signed long _Accum sla_max = 255.9999lk;
    _Sat unsigned long _Accum ula_min = 0.0ulk;
    _Sat unsigned long _Accum ula_max = 65535.9999ulk;
    
    // Mixed precision operations to force range conversions
    signed short _Fract sf1 = 0.5hr;
    signed short _Fract sf2 = -0.25hr;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication approaching max boundary
        usf_max = usf_max * 0.9999ur;
        
        // Division approaching min boundary  
        sla_min = sla_min / 2.0lk;
        
        // Mixed-type arithmetic forcing range analysis
        signed long _Accum mixed_result = sla_max * sf1;
        
        // Boundary comparisons that should trigger the uncovered code
        if (mixed_result > sla_max) {
            result += 1;
        }
        
        if (usf_max < usf_min) {
            result += 2;
        }
        
        // Nested comparisons similar to uncovered condition
        signed long _Accum temp1 = sla_max * 0.75lk;
        signed long _Accum temp2 = sla_min * 1.25lk;
        
        if (temp1 > sla_max || (temp1 == sla_max && temp2 > sla_min)) {
            result += 4;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks(void) {
    _Sat signed long _Accum a = 200.0lk;
    _Sat signed long _Accum b = 1.5lk;
    _Sat signed long _Accum c;
    
    // Use builtins to check for overflow
    int overflow;
    
    // Multiplication overflow check
    overflow = __builtin_mul_overflow(a, b, &c);
    if (overflow) {
        result += 8;
    }
    
    // Addition overflow check with boundary values
    _Sat signed long _Accum max_val = 255.9999lk;
    _Sat signed long _Accum inc = 0.0001lk;
    
    for (int i = 0; i < 1000; i++) {
        overflow = __builtin_add_overflow(max_val, inc, &max_val);
        if (overflow) {
            result += 16;
            break;
        }
    }
    
    // Division with saturation
    _Sat signed long _Accum small = 0.0001lk;
    _Sat signed long _Accum div_result = a / small;
    
    if (div_result > max_val) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat unsigned short _Fract usf_array[4];
    _Sat signed long _Accum sla_array[4];
    _Sat unsigned long _Accum ula_array[4];
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container;
    
    // Initialize array with boundary values
    for (int i = 0; i < 4; i++) {
        container.usf_array[i] = i * 0.25ur;
        container.sla_array[i] = -256.0lk + i * 128.0lk;
        container.ula_array[i] = i * 16384.0ulk;
    }
    
    // Perform operations on array elements
    for (int i = 0; i < 3; i++) {
        // Multiplication that may exceed boundaries
        container.sla_array[i] = container.sla_array[i] * container.sla_array[i+1];
        
        // Division that may underflow
        container.ula_array[i] = container.ula_array[i] / 2.0ulk;
        
        // Comparison similar to uncovered code
        if (container.sla_array[i] > container.sla_array[i+1] ||
            (container.sla_array[i] == container.sla_array[i+1] && 
             container.usf_array[i] > container.usf_array[i+1])) {
            result += 64;
        }
    }
    
    // Chain operations to force complex range analysis
    _Sat signed long _Accum chain_result = container.sla_array[0];
    for (int i = 1; i < 4; i++) {
        chain_result = chain_result * container.usf_array[i];
        
        // Boundary check after each operation
        if (chain_result > 255.0lk || chain_result < -256.0lk) {
            result += 128;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Test conversions between different fixed-point types
    unsigned short _Fract usf_val = 0.75ur;
    signed long _Accum sla_val = 100.0lk;
    
    // Mixed precision multiplication
    signed long _Accum mixed_mul = sla_val * usf_val;
    
    // Force range analysis with comparisons
    _Sat signed long _Accum sat_val = mixed_mul;
    
    // Create comparison scenario similar to uncovered code
    _Sat signed long _Accum reference_max = 255.9999lk;
    _Sat signed long _Accum reference_min = -256.0lk;
    
    // Complex condition to trigger the specific comparison pattern
    if (sat_val > reference_max || 
        (sat_val == reference_max && usf_val > 0.5ur)) {
        result += 256;
    }
    
    // Test with negative values approaching minimum
    signed short _Fract negative_fract = -0.75hr;
    signed long _Accum negative_accum = -200.0lk;
    
    signed long _Accum negative_result = negative_accum * negative_fract;
    
    if (negative_result < reference_min ||
        (negative_result == reference_min && negative_fract < -0.5hr)) {
        result += 512;
    }
}

int main(void) {
    // Run all test functions to exercise different aspects of fixed-point analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_mixed_precision_conversions();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int check = result;
    if (check > 1000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
