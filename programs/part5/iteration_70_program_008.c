#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization for specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract mid_val = 0.5r;
    
    // Operations that approach boundaries
    unsigned short _Fract prod = max_val * mid_val;
    unsigned short _Fract quot = max_val / min_val;
    
    // Boundary comparisons - should trigger range analysis
    if (prod > 0.8r) {
        results[(*count)++] = prod;
    }
    
    // This comparison may trigger the uncovered logic
    if (quot == max_val) {
        results[(*count)++] = quot;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat signed long _Accum *results, int *count) {
    // Saturation arithmetic at extremes
    _Sat signed long _Accum max_sat = 0.9999999999999999999lk;
    _Sat signed long _Accum min_sat = -0.9999999999999999999lk;
    _Sat signed long _Accum zero = 0.0lk;
    
    // Operations that should saturate
    _Sat signed long _Accum saturated_sum = max_sat + max_sat;  // Should saturate to max
    _Sat signed long _Accum saturated_diff = min_sat - max_sat; // Should saturate to min
    
    // Mixed precision operations
    signed short _Accum short_acc = 0.5hk;
    _Sat signed long _Accum mixed_prod = max_sat * short_acc;
    
    // Boundary comparisons
    if (saturated_sum == max_sat) {
        results[(*count)++] = saturated_sum;
    }
    
    if (saturated_diff < zero) {
        results[(*count)++] = saturated_diff;
    }
    
    // This comparison may trigger the specific uncovered condition
    if (mixed_prod > 0.4lk || (mixed_prod == 0.4lk && short_acc > 0.0hk)) {
        results[(*count)++] = mixed_prod;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_ops(void) {
    // Different fixed-point types
    unsigned _Fract uf1 = 0.75r;
    signed _Accum sa1 = 0.25k;
    _Sat unsigned long _Accum sula = 0.9999999999999999999ulk;
    
    // Mixed precision arithmetic
    signed _Accum result1 = sa1 * uf1;  // Conversion needed
    _Sat unsigned long _Accum result2 = sula / uf1;
    
    // Built-in overflow checks with fixed-point
    signed _Accum overflow_check;
    int overflow1 = __builtin_mul_overflow(sa1, sa1, &overflow_check);
    int overflow2 = __builtin_add_overflow(sula, sula, &overflow_check);
    
    // Control flow based on ranges
    if (result1 > 0.1k && result2 < 1.0ulk) {
        volatile signed _Accum force_use = result1;
        (void)force_use;
    }
    
    if (overflow1 || overflow2) {
        volatile int force_use = overflow1 + overflow2;
        (void)force_use;
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat signed short _Accum values[4];
    unsigned _Fract fractions[2];
    signed long _Accum accumulator;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointContainer *container) {
    // Initialize with boundary values
    container->values[0] = 0.999hk;
    container->values[1] = -0.999hk;
    container->values[2] = 0.0hk;
    container->values[3] = 0.5hk;
    
    container->fractions[0] = 0.9999r;
    container->fractions[1] = 0.0001r;
    
    // Operations on array elements
    for (int i = 0; i < 3; i++) {
        container->values[i] = container->values[i] * container->values[i+1];
        
        // Boundary comparison in loop
        if (container->values[i] > 0.8hk || 
            (container->values[i] == 0.8hk && container->fractions[0] > 0.5r)) {
            container->accumulator += container->values[i];
        }
    }
    
    // Mixed operations
    container->accumulator = container->accumulator * container->fractions[0];
}

__attribute__((optimize("O3")))
void test_extreme_boundary_cases(void) {
    // Values at exact boundaries
    _Sat unsigned long _Accum max_boundary = 0.9999999999999999999ulk;
    _Sat unsigned long _Accum min_boundary = 0.0000000000000000001ulk;
    signed long _Accum zero_boundary = 0.0lk;
    
    // Operations that test boundary handling
    _Sat unsigned long _Accum test1 = max_boundary * max_boundary;
    _Sat unsigned long _Accum test2 = max_boundary / min_boundary;
    signed long _Accum test3 = zero_boundary * max_boundary;
    
    // Complex boundary comparisons
    // This pattern aims to trigger: a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
    if (test1 > 0.9ulk || 
        (test1 == 0.9ulk && test2 > 0.5ulk)) {
        volatile _Sat unsigned long _Accum force1 = test1;
        (void)force1;
    }
    
    if (test3 == zero_boundary || test3 < zero_boundary) {
        volatile signed long _Accum force2 = test3;
        (void)force2;
    }
    
    // Division near boundaries
    _Sat unsigned long _Accum div_result = max_boundary / 2.0ulk;
    if (div_result > 0.4ulk && div_result < 0.6ulk) {
        volatile _Sat unsigned long _Accum force3 = div_result;
        (void)force3;
    }
}

int main(void) {
    // Arrays to store results (prevent dead code elimination)
    unsigned short _Fract fract_results[10];
    _Sat signed long _Accum accum_results[10];
    int fract_count = 0;
    int accum_count = 0;
    
    // Test different fixed-point scenarios
    test_short_fract_range(fract_results, &fract_count);
    test_sat_accum_range(accum_results, &accum_count);
    test_mixed_precision_ops();
    
    struct FixedPointContainer container;
    test_struct_operations(&container);
    
    test_extreme_boundary_cases();
    
    // Aggregate results to prevent optimization
    volatile unsigned short _Fract fract_sum = 0.0r;
    for (int i = 0; i < fract_count; i++) {
        fract_sum += fract_results[i];
    }
    
    volatile _Sat signed long _Accum accum_sum = 0.0lk;
    for (int i = 0; i < accum_count; i++) {
        accum_sum += accum_results[i];
    }
    
    // Print to prevent dead code elimination
    printf("Fract results: %d, Accum results: %d\n", fract_count, accum_count);
    printf("Container accumulator: %Lf\n", (long double)container.accumulator);
    
    return 0;
}
