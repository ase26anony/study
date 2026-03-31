#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    // Use _Fract types to trigger fractional bit analysis
    volatile short _Fract sf_min = 0x0.0001r;  // Near minimum
    volatile short _Fract sf_max = 0x0.FFFCr;  // Near maximum
    volatile short _Fract sf_mid = 0x0.8000r;  // 0.5
    
    // Mixed precision operations forcing range analysis
    _Sat unsigned long _Accum ula = 0x0.00000001k;  // Small positive
    _Sat signed long _Accum sla = -0x0.80000000k;   // -0.5
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication that can saturate
        _Sat unsigned short _Fract usf_result = (_Sat unsigned short _Fract)ula * sf_max;
        
        // Division that can approach limits
        _Sat signed short _Fract ssf_result = (_Sat signed short _Fract)sla / sf_mid;
        
        // Conditional based on fixed-point comparison
        if (usf_result > 0x0.FF00r) {
            // Force range analysis for high values
            volatile _Sat unsigned long _Accum temp = usf_result;
            temp = temp * 2.0k;  // May saturate
        }
        
        // Update for next iteration
        ula = ula + 0x0.00000001k;
        sla = sla + 0x0.00000001k;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_operations(void) {
    // Different fixed-point types with varying i_f_bits
    volatile _Fract f1 = 0.5r;
    volatile _Accum a1 = 10.5k;
    volatile long _Accum la1 = -5.25lk;
    
    // Struct containing fixed-point values
    struct FixedPointStruct {
        _Sat short _Fract min_val;
        _Sat short _Fract max_val;
        _Sat unsigned _Accum mid_val;
    } fps = {
        .min_val = 0x0.0001r,
        .max_val = 0x0.FFFCr,
        .mid_val = 0x0.80000000k
    };
    
    // Array operations
    _Sat signed _Accum arr[4] = {
        -1.0k, 0.0k, 0.5k, 1.0k
    };
    
    // Mixed operations that force precision conversions
    for (int i = 0; i < 4; i++) {
        // Multiplication across different types
        _Sat long _Accum result = (_Sat long _Accum)arr[i] * (_Sat long _Accum)fps.mid_val;
        
        // Division with boundary checking
        if (arr[i] != 0.0k) {
            _Sat short _Fract div_result = (_Sat short _Fract)f1 / (_Sat short _Fract)arr[i];
            
            // Comparison that should trigger the uncovered condition
            if (result > 0x0.FFFFFFFFk || result < -0x0.FFFFFFFFk) {
                volatile _Sat long _Accum clamped = result;
                // Force saturation
                clamped = clamped + 10.0lk;
            }
        }
        
        // Built-in overflow checks with fixed-point
        _Sat signed _Accum sum;
        if (__builtin_add_overflow(arr[i], 0.1k, &sum)) {
            // Handle overflow - should trigger saturation analysis
            volatile _Sat signed _Accum overflow_val = sum;
        }
    }
}

__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Values at extreme boundaries
    volatile _Sat unsigned long _Accum ula_max = 0xFFFFFFFF.FFFFFFFFk;  // Max unsigned
    volatile _Sat signed long _Accum sla_min = -0x80000000.00000000lk;  // Min signed
    volatile _Sat signed long _Accum sla_max = 0x7FFFFFFF.FFFFFFFFlk;   // Max signed
    
    // Operations that push to boundaries
    _Sat unsigned long _Accum ula_test = ula_max * 0.99999999k;
    _Sat signed long _Accum sla_test = sla_min / 0.5lk;
    
    // Complex comparisons that should trigger the uncovered logic
    // These mimic the structure: a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    
    // Test near upper boundary
    for (int i = 0; i < 5; i++) {
        _Sat unsigned short _Fract usf1 = 0x0.FFF0r + i * 0x0.0001r;
        _Sat unsigned short _Fract usf2 = 0x0.FFE0r;
        
        // Comparison that may trigger high/low part analysis
        if (usf1 > usf2) {
            // Additional operations to force range tracking
            volatile _Sat unsigned long _Accum temp = (_Sat unsigned long _Accum)usf1;
            temp = temp * temp;  // Square - may overflow
        }
    }
    
    // Test near lower boundary
    for (int i = 0; i < 5; i++) {
        _Sat signed short _Fract ssf1 = -0x0.8000r + i * 0x0.0001r;
        _Sat signed short _Fract ssf2 = -0x0.7F00r;
        
        // Another boundary comparison
        if (ssf1 < ssf2 || ssf1 == ssf2) {
            volatile _Sat signed long _Accum temp = (_Sat signed long _Accum)ssf1;
            temp = temp / 0.25lk;  // Division - may underflow
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_scenarios(void) {
    // Use builtins with fixed-point types
    _Sat signed _Accum a = 0x7FFFFFFF.FFFFFFFEk;  // Almost max
    _Sat signed _Accum b = 0x0.00000002k;         // Small increment
    
    _Sat signed _Accum result;
    
    // These should trigger overflow analysis
    if (__builtin_add_overflow(a, b, &result)) {
        volatile _Sat signed _Accum overflowed = result;
    }
    
    if (__builtin_mul_overflow(a, 2.0k, &result)) {
        volatile _Sat signed _Accum overflowed = result;
    }
    
    // Test with different fractional bit configurations
    _Sat unsigned short _Fract usf_a = 0x0.FF00r;
    _Sat unsigned short _Fract usf_b = 0x0.0100r;
    
    _Sat unsigned short _Fract usf_result;
    if (__builtin_add_overflow(usf_a, usf_b, &usf_result)) {
        // Should saturate
        volatile _Sat unsigned short _Fract saturated = usf_result;
    }
}

int main(void) {
    volatile _Sat unsigned long _Accum accumulator = 0.0k;
    
    // Run all tests to exercise different fixed-point scenarios
    test_short_fract_boundaries();
    test_mixed_precision_operations();
    test_boundary_comparisons();
    test_overflow_scenarios();
    
    // Aggregate results to prevent dead code elimination
    accumulator = accumulator + 0.00000001k;
    
    // Print to force side effects
    printf("Fixed-point tests completed. Accumulator: %llx\n", 
           (unsigned long long)accumulator);
    
    return 0;
}
