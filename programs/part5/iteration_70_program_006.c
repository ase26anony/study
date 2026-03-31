#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(void) {
    // Initialize near boundaries
    volatile short _Fract sf_min = 0x80;  // Minimum: -1.0
    volatile short _Fract sf_max = 0x7F;  // Maximum: ~0.992
    volatile short _Fract sf_zero = 0x00;
    
    // Mixed precision operations
    _Sat unsigned long _Accum ula = 0.5uk;
    _Sat signed long _Accum sla = -0.5k;
    
    // Operations that force range analysis
    for (int i = 0; i < 10; i++) {
        // Multiplication with different precisions
        short _Fract sf_result = sf_max * (_Fract)ula;
        
        // Division near boundaries
        if (sf_min != 0) {
            short _Fract sf_div = sf_max / sf_min;
            (void)sf_div;  // Prevent unused warning
        }
        
        // Control flow dependent on fixed-point ranges
        if (sf_result > 0.5hr) {
            sla = sla + 0.1k;
        } else if (sf_result < -0.5hr) {
            sla = sla - 0.1k;
        }
        
        // Saturation arithmetic
        _Sat short _Fract ssf = sf_result;
        ssf = ssf + ssf;  // May saturate
        
        // Update for next iteration
        sf_max = sf_max * 0.9hr;
    }
}

__attribute__((optimize("O3")))
void test_accum_boundaries(void) {
    // Initialize at extreme values
    _Sat signed short _Accum ssa_min = -0x8000k;  // Minimum
    _Sat signed short _Accum ssa_max = 0x7FFFk;   // Maximum
    _Sat unsigned short _Accum usa_max = 0xFFFFuk;
    
    // Mixed signedness operations
    for (int i = 0; i < 5; i++) {
        // Multiplication that may overflow
        _Sat signed short _Accum ssa_prod = ssa_max * ssa_max;
        
        // Division with different signs
        _Sat signed short _Accum ssa_div = ssa_max / (_Sat signed short _Accum)0.5k;
        
        // Comparisons that should trigger range analysis
        if (ssa_prod > ssa_max) {
            // This should never happen with saturation
            ssa_max = ssa_max - 0.1k;
        }
        
        if (ssa_div < ssa_min) {
            ssa_min = ssa_min + 0.1k;
        }
        
        // Built-in overflow checks
        _Sat signed short _Accum ssa_sum;
        int overflow = __builtin_add_overflow(ssa_max, 0.5k, &ssa_sum);
        if (overflow) {
            ssa_max = ssa_max * 0.9k;
        }
        
        // Update values
        ssa_max = ssa_max * 0.95k;
        usa_max = usa_max * 0.9uk;
    }
}

// Struct with fixed-point members
struct FixedPointData {
    _Sat signed long _Accum accum;
    unsigned short _Fract fract;
    _Sat unsigned long _Accum uaccum;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointData data[4];
    
    // Initialize array with boundary values
    for (int i = 0; i < 4; i++) {
        data[i].accum = (i % 2 == 0) ? -0.999k : 0.999k;
        data[i].fract = (i * 0.25uhk);
        data[i].uaccum = (i * 0.25uk);
    }
    
    // Operations on struct members
    for (int i = 0; i < 3; i++) {
        // Mixed-type operations between struct members
        _Sat signed long _Accum temp = data[i].accum * (_Sat signed long _Accum)data[i].fract;
        
        // Range-dependent control flow
        if (temp > data[i+1].accum) {
            data[i].accum = data[i].accum - 0.1k;
        } else if (temp < -data[i+1].accum) {
            data[i].accum = data[i].accum + 0.1k;
        }
        
        // Multiplication with overflow check
        _Sat unsigned long _Accum utemp;
        int mul_overflow = __builtin_mul_overflow(data[i].uaccum, 2.0uk, &utemp);
        if (!mul_overflow) {
            data[i].uaccum = utemp;
        }
    }
    
    // Final boundary comparisons
    for (int i = 0; i < 4; i++) {
        if (data[i].accum > 0.8k || data[i].accum < -0.8k) {
            data[i].fract = data[i].fract * 0.5uhk;
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_comparisons(void) {
    // Values designed to exercise the specific comparison logic
    volatile _Sat signed long _Accum a = 0.999999999999999k;  // Very close to max
    volatile _Sat unsigned long _Accum b = 0.000000000000001uk; // Very close to min
    
    // Sequences that should trigger high/low part comparisons
    for (int i = 0; i < 8; i++) {
        // Operations that approach boundaries
        a = a * 1.000000000000001k;
        b = b / 1.000000000000001uk;
        
        // Explicit comparisons at boundaries
        if (a > 0.999999999999999k) {
            a = 0.999999999999999k;
        }
        
        if (b < 0.000000000000001uk) {
            b = 0.000000000000001uk;
        }
        
        // Mixed signedness comparison
        _Sat signed long _Accum c = (_Sat signed long _Accum)b;
        if (a > c || (a == c && b > 0.5uk)) {
            a = a - 0.000000000000001k;
        }
    }
}

int main(void) {
    volatile int result = 0;
    
    // Execute all test functions
    test_short_fract_range();
    test_accum_boundaries();
    test_struct_operations();
    test_extreme_comparisons();
    
    // Additional mixed operations in main
    _Sat unsigned short _Fract usf1 = 0.8uhk;
    _Sat signed short _Fract ssf1 = -0.8hk;
    _Sat unsigned long _Accum ula1 = 0.9uk;
    
    // Complex expression to force range analysis
    for (int i = 0; i < 100; i++) {
        usf1 = usf1 * 1.01uhk;
        ssf1 = ssf1 * 0.99hk;
        ula1 = ula1 / 1.0001uk;
        
        // Conditional that depends on all three
        if (usf1 > 0.9uhk && ssf1 < -0.1hk && ula1 < 0.5uk) {
            result++;
        }
        
        // Boundary checks
        if (usf1 > 0.99uhk) {
            usf1 = 0.1uhk;
        }
        if (ssf1 < -0.99hk) {
            ssf1 = -0.1hk;
        }
    }
    
    // Print to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
