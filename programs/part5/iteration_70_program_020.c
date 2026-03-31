#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_boundary_comparisons(void) {
    // Use various fixed-point types to trigger different i_f_bits configurations
    volatile _Sat unsigned short _Fract usf1 = 0.5r;
    volatile _Sat unsigned short _Fract usf2 = 0.75r;
    volatile _Sat signed short _Fract ssf1 = -0.5r;
    volatile _Sat signed short _Fract ssf2 = 0.5r;
    
    // _Accum types with different precisions
    volatile _Sat unsigned long _Accum ula1 = 100.0k;
    volatile _Sat unsigned long _Accum ula2 = 200.0k;
    volatile _Sat signed long _Accum sla1 = -100.0k;
    volatile _Sat signed long _Accum sla2 = 100.0k;
    
    // _Fract types
    volatile _Sat unsigned _Fract uf1 = 0.1r;
    volatile _Sat unsigned _Fract uf2 = 0.9r;
    volatile _Sat signed _Fract sf1 = -0.1r;
    volatile _Sat signed _Fract sf2 = 0.1r;
    
    // Mixed precision operations to force range analysis
    // These should trigger conversions and boundary checks
    for (int i = 0; i < 10; i++) {
        // Operations that approach boundaries
        usf1 = usf1 * usf2;
        ssf1 = ssf1 * ssf2;
        
        // Mixed type operations
        ula1 = ula1 + (_Sat unsigned long _Accum)usf1;
        sla1 = sla1 + (_Sat signed long _Accum)ssf1;
        
        // Boundary comparisons - should trigger the uncovered condition
        if (ula1 > ula2) {
            // Force evaluation of high/low part comparisons
            volatile _Sat unsigned long _Accum temp = ula1 - ula2;
            (void)temp;
        }
        
        if (sla1 == sla2) {
            // Another boundary case
            sla1 = sla1 * 0.5k;
        }
        
        // Complex condition similar to uncovered code
        if ((ula1 > 250.0k) || (ula1 == 250.0k && usf1 > 0.8r)) {
            ula1 = ula1 * 0.9k;
        }
    }
}

__attribute__((optimize("O3")))
void test_overflow_checks(void) {
    // Use builtins with fixed-point to trigger overflow analysis
    _Sat unsigned long _Accum ula = 300.0k;
    _Sat unsigned long _Accum ulb = 200.0k;
    _Sat unsigned long _Accum ulc;
    
    // These builtins should interact with value range analysis
    int overflow_mul = __builtin_mul_overflow(ula, ulb, &ulc);
    int overflow_add = __builtin_add_overflow(ula, ulb, &ulc);
    
    // Boundary values that should trigger max_r/max_s initialization
    _Sat unsigned short _Fract max_usf = 0.9999r;  // Near maximum
    _Sat unsigned short _Fract min_usf = 0.0001r;  // Near minimum
    
    // Operations that push to boundaries
    for (int i = 0; i < 5; i++) {
        max_usf = max_usf * 1.1r;  // Should saturate
        min_usf = min_usf * 0.5r;  // Should approach zero
        
        // Comparisons at boundaries
        if (max_usf > 0.99r || (max_usf == 0.99r && min_usf < 0.01r)) {
            // This should trigger the a_high.sgt(max_r) type logic
            volatile _Sat unsigned short _Fract temp = max_usf / min_usf;
            (void)temp;
        }
    }
    
    (void)overflow_mul;
    (void)overflow_add;
}

// Struct with fixed-point members to test range tracking through memory
struct FixedPointContainer {
    _Sat signed long _Accum accum;
    _Sat unsigned short _Fract fract;
    _Sat signed _Fract sfract;
};

__attribute__((optimize("O3")))
void test_struct_operations(void) {
    struct FixedPointContainer containers[4];
    
    // Initialize with boundary values
    containers[0].accum = -500.0k;
    containers[0].fract = 0.1r;
    containers[0].sfract = -0.5r;
    
    containers[1].accum = 0.0k;
    containers[1].fract = 0.5r;
    containers[1].sfract = 0.0r;
    
    containers[2].accum = 500.0k;
    containers[2].fract = 0.9r;
    containers[2].sfract = 0.5r;
    
    containers[3].accum = 1000.0k;
    containers[3].fract = 0.99r;
    containers[3].sfract = 0.9r;
    
    // Perform operations that should trigger range analysis
    for (int i = 0; i < 4; i++) {
        // Mixed operations
        containers[i].accum = containers[i].accum * 1.1k;
        containers[i].fract = containers[i].fract * 1.1r;
        containers[i].sfract = containers[i].sfract * 1.1r;
        
        // Boundary comparisons in struct context
        if (containers[i].accum > 800.0k || 
            (containers[i].accum == 800.0k && containers[i].fract > 0.8r)) {
            // This should exercise the comparison logic
            containers[i].accum = containers[i].accum * 0.5k;
        }
        
        // Another boundary case for negative values
        if (containers[i].accum < -400.0k || 
            (containers[i].accum == -400.0k && containers[i].sfract < -0.4r)) {
            containers[i].accum = containers[i].accum * 0.5k;
        }
    }
}

__attribute__((optimize("O3")))
void test_extreme_boundaries(void) {
    // Test with values at exact boundaries
    volatile _Sat unsigned long _Accum max_ula = 9223372036854775.807k;  // Near max
    volatile _Sat unsigned long _Accum min_ula = 0.000k;                 // Min
    
    volatile _Sat signed long _Accum max_sla = 4611686018427387.903k;   // Near max signed
    volatile _Sat signed long _Accum min_sla = -4611686018427388.903k;  // Near min signed
    
    // Operations that should trigger saturation and boundary checks
    for (int i = 0; i < 3; i++) {
        // Push to boundaries
        max_ula = max_ula * 1.1k;  // Should saturate
        min_ula = min_ula * 0.9k;  // Should stay at min
        
        max_sla = max_sla * 1.1k;  // Should saturate positive
        min_sla = min_sla * 1.1k;  // Should become less negative (or saturate)
        
        // Complex boundary comparisons
        // This should trigger the exact logic in the uncovered code
        if (max_ula > 9000000000000000.0k || 
            (max_ula == 9000000000000000.0k && min_ula > 0.001k)) {
            volatile _Sat unsigned long _Accum diff = max_ula - min_ula;
            (void)diff;
        }
        
        // Signed version
        if (max_sla > 4000000000000000.0k || 
            (max_sla == 4000000000000000.0k && min_sla < -4000000000000000.0k)) {
            volatile _Sat signed long _Accum sum = max_sla + min_sla;
            (void)sum;
        }
    }
}

int main(void) {
    // Initialize volatile result to prevent dead code elimination
    volatile _Sat signed long _Accum result = 0.0k;
    
    // Run all tests to trigger different aspects of fixed-point range analysis
    test_boundary_comparisons();
    test_overflow_checks();
    test_struct_operations();
    test_extreme_boundaries();
    
    // Aggregate some results (even if artificial) to ensure code isn't optimized away
    _Sat unsigned short _Fract f1 = 0.5r;
    _Sat signed long _Accum a1 = 100.0k;
    
    for (int i = 0; i < 10; i++) {
        f1 = f1 * 1.1r;
        a1 = a1 + (_Sat signed long _Accum)f1;
        
        // Boundary check on each iteration
        if (a1 > 150.0k || (a1 == 150.0k && f1 > 0.7r)) {
            a1 = a1 * 0.9k;
        }
    }
    
    result = a1;
    
    // Print to prevent optimization
    printf("Result: %Lk\n", (long double)result);
    
    return 0;
}
