#include <stdio.h>
#include <stdint.h>

// Use volatile to prevent dead code elimination
volatile int result = 0;

// Function with aggressive optimization
__attribute__((optimize("O3")))
void test_boundary_comparisons() {
    // Initialize fixed-point types at boundaries
    _Sat unsigned long _Accum max_accum = 0.99999999999999999999UK;
    _Sat signed long _Accum min_accum = -0.99999999999999999999K;
    unsigned short _Fract max_fract = 0.9999999UHR;
    signed short _Fract min_fract = -0.9999999HR;
    
    // Mixed precision operations to force range analysis
    _Sat signed long _Accum mixed_result = (_Sat signed long _Accum)max_fract * min_accum;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (max_accum > 0.9UK) {
        result += 1;
    }
    
    if (min_accum < -0.9K) {
        result += 2;
    }
    
    // Complex condition similar to uncovered code
    if (mixed_result > 0K || (mixed_result == 0K && max_fract > 0.5UHR)) {
        result += 4;
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks() {
    _Sat unsigned _Accum a = 0.8UHK;
    _Sat unsigned _Accum b = 0.7UHK;
    _Sat unsigned _Accum c;
    
    // Use builtins to check overflow
    int overflow = __builtin_mul_overflow(a, b, &c);
    
    if (overflow) {
        result += 8;
    } else if (c > 0.5UHK) {
        result += 16;
    }
    
    // Test with different fractional bits configurations
    _Fract x = 0.5R;
    _Accum y = 0.25K;
    
    // Mixed type operation forcing precision conversion
    _Accum z = (_Accum)x * y;
    
    if (z > 0.1K && z < 0.2K) {
        result += 32;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat signed short _Accum values[4];
    unsigned _Fract fract_values[2];
};

__attribute__((optimize("O3")))
void test_struct_operations() {
    struct FixedPointContainer container;
    
    // Initialize with boundary values
    container.values[0] = 0.9999HK;
    container.values[1] = -0.9999HK;
    container.values[2] = 0.0HK;
    container.values[3] = 0.5HK;
    
    container.fract_values[0] = 0.9999UHR;
    container.fract_values[1] = 0.0001UHR;
    
    // Operations on array elements
    for (int i = 0; i < 3; i++) {
        container.values[i] = container.values[i] * container.values[i+1];
        
        // Conditional based on fixed-point comparison
        if (container.values[i] > 0K) {
            result += 64;
        } else if (container.values[i] < 0K) {
            result += 128;
        }
    }
    
    // Mixed array operations
    _Accum temp = (_Accum)container.fract_values[0] * container.values[0];
    
    if (temp > 0.4K && temp < 0.6K) {
        result += 256;
    }
}

__attribute__((optimize("O3")))
void test_precision_conversions() {
    // Test various precision conversions
    unsigned short _Fract usf = 0.75UHR;
    signed long _Fract slf = -0.25LR;
    _Sat unsigned _Accum usa = 0.9UHK;
    
    // Chain of conversions and operations
    _Sat signed long _Accum combined = (_Sat signed long _Accum)usf * 
                                      (_Sat signed long _Accum)slf + 
                                      (_Sat signed long _Accum)usa;
    
    // Complex boundary condition
    if (combined > 0.8K || (combined == 0.8K && usf > 0.5UHR)) {
        result += 512;
    }
    
    // Test near-zero boundary
    _Fract small = 0.0000001R;
    _Accum scaled = small * 1000000K;
    
    if (scaled > 0K && scaled < 1K) {
        result += 1024;
    }
}

__attribute__((optimize("O3")))
void test_extreme_values() {
    // Values at absolute boundaries
    _Sat unsigned _Accum max_val = 0.99999999999999999999UHK;
    _Sat unsigned _Accum min_val = 0.00000000000000000001UHK;
    
    // Operations that should saturate
    _Sat unsigned _Accum saturated = max_val * 2UHK;
    _Sat unsigned _Accum underflow = min_val / 2UHK;
    
    // Comparisons triggering range analysis
    if (saturated == max_val) {  // Should be true due to saturation
        result += 2048;
    }
    
    if (underflow > 0UHK) {  // Should be true
        result += 4096;
    }
    
    // Test with builtin overflow
    _Sat signed _Accum a = 0.9999HK;
    _Sat signed _Accum b = 0.9999HK;
    _Sat signed _Accum c;
    
    if (__builtin_mul_overflow(a, b, &c)) {
        result += 8192;
    }
}

int main() {
    // Initialize result
    result = 0;
    
    // Run all tests to exercise fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_precision_conversions();
    test_extreme_values();
    
    // Print result to prevent optimization
    printf("Result: %d\n", result);
    
    // Additional volatile operations to ensure all code paths are considered
    volatile int final_check = result;
    if (final_check > 0) {
        printf("Tests completed successfully\n");
    }
    
    return 0;
}
