#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_boundaries(void) {
    // Test short _Fract near boundaries
    volatile short _Fract sf_min = 0x80;  // Minimum: -1.0
    volatile short _Fract sf_max = 0x7F;  // Maximum: 0.9921875
    volatile short _Fract sf_mid = 0x40;  // 0.5
    
    // Operations that force range analysis
    short _Fract result1 = sf_max * sf_mid;  // ~0.496
    short _Fract result2 = sf_min * sf_mid;  // -0.5
    
    // Boundary comparisons that should trigger the uncovered logic
    if (result1 > sf_mid) {
        // This branch unlikely but forces comparison analysis
        volatile short _Fract tmp = result1;
    }
    
    if (result2 < sf_min) {
        // This should be false but forces range checking
        volatile short _Fract tmp = result2;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_operations(void) {
    // Saturated unsigned long accum with boundary values
    volatile _Sat unsigned long _Accum ula_min = 0.0uk;
    volatile _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFFuk;  // Max
    
    // Mixed precision operations
    volatile unsigned short _Fract usf = 0.5uhr;
    
    // Operations that should trigger range analysis
    _Sat unsigned long _Accum ula_result = ula_max * usf;
    
    // Force saturation by attempting overflow
    _Sat unsigned long _Accum ula_saturated;
    int overflow = __builtin_mul_overflow(ula_max, 2.0uk, &ula_saturated);
    
    // Boundary comparisons
    if (ula_result > ula_max) {
        // Should be false due to saturation
        volatile _Sat unsigned long _Accum tmp = ula_result;
    }
    
    // Compare with mid-range value
    if (ula_result == (ula_max / 2.0uk)) {
        volatile _Sat unsigned long _Accum tmp = ula_result;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Different fixed-point types to force conversions
    volatile signed short _Fract ssf = -0.5hr;
    volatile unsigned long _Accum ula = 1000.0uk;
    volatile _Sat signed long _Accum sla = 500.0k;
    
    // Mixed operations with different fractional bits
    signed long _Accum result1 = sla * ssf;  // -250.0
    unsigned long _Accum result2 = ula + (_Accum)ssf;  // 999.5
    
    // Complex expression with multiple conversions
    _Sat signed long _Accum final_result = result1 + (_Sat signed long _Accum)result2;
    
    // Comparisons that should trigger boundary checks
    if (final_result > sla) {
        volatile _Sat signed long _Accum tmp = final_result;
    }
    
    // Nested comparisons
    if ((result1 < 0.0k) && (result2 > ula)) {
        volatile signed long _Accum tmp1 = result1;
        volatile unsigned long _Accum tmp2 = result2;
    }
}

__attribute__((optimize("O3")))
void test_struct_aggregate_operations(void) {
    // Struct containing fixed-point values
    struct FixedPointData {
        _Sat signed short _Fract fract_data[4];
        unsigned long _Accum accum_data[2];
        signed long _Accum signed_accum;
    };
    
    struct FixedPointData data = {
        .fract_data = {0.25hr, -0.75hr, 0.5hr, -0.125hr},
        .accum_data = {100.0uk, 200.0uk},
        .signed_accum = -50.0k
    };
    
    // Array operations that force range analysis
    for (int i = 0; i < 4; i++) {
        data.fract_data[i] = data.fract_data[i] * 2.0hr;
        
        // Boundary check on each iteration
        if (data.fract_data[i] > 0.9hr || data.fract_data[i] < -0.9hr) {
            // Force saturation analysis
            volatile _Sat signed short _Fract tmp = data.fract_data[i];
        }
    }
    
    // Accumulator operations
    data.accum_data[0] = data.accum_data[0] * data.accum_data[1];
    data.signed_accum = data.signed_accum / (_Accum)data.fract_data[0];
    
    // Final boundary comparison
    if (data.accum_data[0] > 50000.0uk || data.signed_accum < -1000.0k) {
        volatile unsigned long _Accum tmp1 = data.accum_data[0];
        volatile signed long _Accum tmp2 = data.signed_accum;
    }
}

__attribute__((optimize("O3")))
void test_division_edge_cases(void) {
    // Division operations near boundaries
    volatile _Sat signed long _Accum sla_max = 0x7FFFFFFFFFFFFFFFk;  // Max positive
    volatile _Sat signed long _Accum sla_min = 0x8000000000000000k;  // Min negative
    
    // Division by small values to force large results
    _Sat signed long _Accum div_result1 = sla_max / 0.0001k;
    _Sat signed long _Accum div_result2 = sla_min / -0.0001k;
    
    // Check for overflow in division
    int overflow1, overflow2;
    _Sat signed long _Accum div1, div2;
    overflow1 = __builtin_mul_overflow(sla_max, 10000.0k, &div1);
    overflow2 = __builtin_mul_overflow(sla_min, -10000.0k, &div2);
    
    // Comparisons that should trigger the uncovered range logic
    if (div_result1 > sla_max || div_result2 < sla_min) {
        // Should trigger due to saturation
        volatile _Sat signed long _Accum tmp1 = div_result1;
        volatile _Sat signed long _Accum tmp2 = div_result2;
    }
}

__attribute__((optimize("O3")))
void test_loop_dependent_ranges(void) {
    // Loop with fixed-point accumulation
    _Sat unsigned short _Accum usa = 0.0uhk;
    volatile unsigned short _Fract usf_increment = 0.1uhr;
    
    // Loop that accumulates to boundary
    for (int i = 0; i < 20; i++) {
        usa = usa + (_Sat unsigned short _Accum)usf_increment;
        
        // Conditional based on accumulated value
        if (usa > 1.5uhk) {
            // Reset when crossing threshold
            usa = 0.0uhk;
        }
        
        // Nested comparison
        if (i > 10 && usa < 0.5uhk) {
            volatile _Sat unsigned short _Accum tmp = usa;
        }
    }
    
    // Final boundary check
    if (usa == 0.0uhk || usa >= 1.0uhk) {
        volatile _Sat unsigned short _Accum tmp = usa;
    }
}

int main(void) {
    // Initialize volatile result to prevent dead code elimination
    volatile int total_checks = 0;
    
    // Run all test functions
    test_short_fract_boundaries();
    total_checks++;
    
    test_sat_accum_operations();
    total_checks++;
    
    test_mixed_precision_conversions();
    total_checks++;
    
    test_struct_aggregate_operations();
    total_checks++;
    
    test_division_edge_cases();
    total_checks++;
    
    test_loop_dependent_ranges();
    total_checks++;
    
    // Print result to prevent optimization
    printf("Performed %d fixed-point test groups\n", total_checks);
    
    return 0;
}
