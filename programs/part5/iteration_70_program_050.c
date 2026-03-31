#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization for specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(void) {
    // Use boundary values for short _Fract
    volatile short _Fract sf_min = 0x0.0001p-15;  // Minimum positive
    volatile short _Fract sf_max = 0x0.fffbp-15;  // Maximum (1 - epsilon)
    volatile short _Fract sf_mid = 0x0.8000p-15;  // 0.5
    
    // Mixed precision operations to force range analysis
    _Fract f1 = sf_min * sf_mid;
    _Fract f2 = sf_max / sf_mid;
    
    // Control flow dependent on fixed-point ranges
    if (f1 > 0.1r) {
        // This branch should be taken for certain ranges
        f2 = f2 * 0.9r;
    }
    
    // Boundary value comparison
    if (sf_max > 0.99r && sf_min < 0.01r) {
        // Force range comparison logic
        volatile _Fract temp = f1 + f2;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(void) {
    // Saturated accum types at boundaries
    volatile _Sat unsigned long _Accum ula_max = 0xFFFFFFFFFFFFFFFFp-32;  // Max
    volatile _Sat unsigned long _Accum ula_min = 0x0p-32;                 // Min
    volatile _Sat signed long _Accum sla_max = 0x7FFFFFFFFFFFFFFFp-32;    // Max signed
    volatile _Sat signed long _Accum sla_min = 0x8000000000000000p-32;    // Min signed
    
    // Mixed signedness operations
    _Sat signed long _Accum result1 = sla_max * 0.5k;
    _Sat unsigned long _Accum result2 = ula_max / 2uk;
    
    // Operations that should saturate
    _Sat signed long _Accum saturated = sla_max + sla_max;  // Should saturate
    _Sat unsigned long _Accum usaturated = ula_max + ula_max;  // Should saturate
    
    // Control flow with boundary comparisons
    if (sla_max > 0x7FFFFFFFFFFFFFFFp-33) {  // Compare with half max
        result1 = result1 * 0.75k;
    }
    
    if (ula_min < 0x1p-32) {
        result2 = result2 - 0.25uk;
    }
    
    // Use builtins with fixed-point to force overflow analysis
    _Sat signed long _Accum mul_result;
    int overflow = __builtin_mul_overflow(sla_max, 2k, &mul_result);
    
    _Sat unsigned long _Accum add_result;
    int no_overflow = __builtin_add_overflow(ula_max, ula_min, &add_result);
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions(void) {
    // Different fixed-point types to force conversions
    unsigned short _Fract usf = 0x0.8000p-15;  // 0.5
    signed long _Accum sla = 0x4000000000000000p-32;  // 0.5 in different format
    
    // Mixed precision multiplication forces range analysis with different i_f_bits
    _Accum mixed_result = (_Accum)usf * sla;
    
    // Division with different types
    _Fract div_result = (_Fract)sla / usf;
    
    // Comparisons that should exercise the uncovered comparison logic
    if (mixed_result > 0.25k) {
        div_result = div_result * 2.0r;
    }
    
    // Nested comparisons
    if (usf > 0.25ur && sla < 0.75k) {
        mixed_result = mixed_result + 0.1k;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat signed short _Accum sat_accum;
    unsigned _Fract ufract;
    signed long _Accum long_accum;
    _Sat unsigned _Fract sat_ufract;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer container[4];
    
    // Initialize with boundary values
    for (int i = 0; i < 4; i++) {
        container[i].sat_accum = (i % 2) ? 0x7FFp-15 : 0x800p-15;  // Max/min
        container[i].ufract = 0x0.0001p-15 + (i * 0x0.4000p-15);
        container[i].long_accum = 0x4000000000000000p-32 * i;
        container[i].sat_ufract = 0x0.fffbp-15 - (i * 0x0.2000p-15);
    }
    
    // Array operations that force range tracking through memory
    _Accum sum = 0k;
    for (int i = 0; i < 4; i++) {
        sum = sum + container[i].long_accum;
        
        // Conditional based on struct member comparisons
        if (container[i].sat_accum > container[(i + 1) % 4].sat_accum) {
            container[i].ufract = container[i].ufract * 0.5r;
        }
    }
    
    // Force range analysis with array element comparisons
    if (container[0].sat_ufract > container[3].sat_ufract) {
        container[0].long_accum = container[0].long_accum * 2k;
    }
}

__attribute__((optimize("O3")))
void test_complex_boundary_conditions(void) {
    // Create values that should exercise the specific comparison pattern
    volatile _Sat signed long _Accum a = 0x7FFFFFFFFFFFFFFFp-32;  // Max positive
    volatile _Sat signed long _Accum b = 0x8000000000000000p-32;  // Min negative
    
    // Operations that create boundary conditions
    _Sat signed long _Accum r1 = a * 0.9999999999k;  // Just below max
    _Sat signed long _Accum r2 = b * 0.9999999999k;  // Just above min
    
    // Multiple comparisons to force the uncovered logic
    if (r1 > 0x7FFFFFFFFFFFFFFEp-32 || r2 < 0x8000000000000001p-32) {
        // This should trigger range comparison with high/low parts
        volatile _Sat signed long _Accum temp = r1 - r2;
    }
    
    // More complex condition similar to uncovered code pattern
    _Sat unsigned long _Accum high_part = 0xFFFFFFFFFFFFFFFEp-32;
    _Sat unsigned long _Accum low_part = 0x1p-32;
    
    if (high_part > 0xFFFFFFFFFFFFFFFFp-33 || 
        (high_part == 0xFFFFFFFFFFFFFFFFp-33 && low_part > 0x0p-32)) {
        // Should exercise the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) logic
        volatile _Sat unsigned long _Accum result = high_part + low_part;
    }
}

int main(void) {
    // Initialize volatile to prevent dead code elimination
    volatile _Accum total_result = 0k;
    
    // Run all test functions
    test_short_fract_range();
    total_result = total_result + 0.1k;
    
    test_sat_accum_range();
    total_result = total_result + 0.2k;
    
    test_mixed_precision_conversions();
    total_result = total_result + 0.3k;
    
    test_struct_operations();
    total_result = total_result + 0.4k;
    
    test_complex_boundary_conditions();
    total_result = total_result + 0.5k;
    
    // Print result to prevent optimization
    printf("Result: %#.10Hhk\n", (short _Accum)total_result);
    
    return 0;
}
