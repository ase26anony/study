#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract zero = 0.0r;
    
    // Mixed precision operations forcing range analysis
    for (int i = 0; i < 8; i++) {
        unsigned short _Fract x = max_val - (i * 0.1r);
        unsigned short _Fract y = min_val + (i * 0.05r);
        
        // Multiplication near boundaries
        unsigned short _Fract prod = x * y;
        
        // Division forcing range contraction
        unsigned short _Fract div = (i > 0) ? x / y : zero;
        
        // Conditional based on fixed-point comparison
        if (prod > 0.5r) {
            results[*count] = prod;
            (*count)++;
        } else if (div < 0.25r && div > zero) {
            results[*count] = div;
            (*count)++;
        }
        
        // Boundary value comparisons
        if (x == max_val || y == min_val) {
            results[*count] = (x + y) / 2.0r;
            (*count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat unsigned long _Accum *accums, int *idx) {
    // Initialize at extremes
    _Sat unsigned long _Accum sat_max = 0.99999999999999999999uk;
    _Sat unsigned long _Accum sat_min = 0.00000000000000000001uk;
    _Sat unsigned long _Accum mid = 0.5uk;
    
    // Operations that will saturate
    _Sat unsigned long _Accum overflow_test = sat_max + sat_max;
    _Sat unsigned long _Accum underflow_test = sat_min - mid;
    
    // Store results
    accums[(*idx)++] = overflow_test;  // Should saturate to max
    accums[(*idx)++] = underflow_test; // Should saturate to min
    
    // Multiplication with saturation
    for (int i = 1; i <= 5; i++) {
        _Sat unsigned long _Accum mult = sat_max * (_Sat unsigned long _Accum)(i * 0.2uk);
        
        // Comparison triggering range analysis
        if (mult > 0.8uk) {
            accums[(*idx)++] = mult;
        } else if (mult < 0.2uk && mult > sat_min) {
            accums[(*idx)++] = mult + sat_min;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_ops(void) {
    // Mixed signedness and precision
    signed short _Fract sf1 = 0.5r;
    unsigned short _Fract uf1 = 0.75r;
    signed long _Accum sla1 = 0.123456789lk;
    _Sat unsigned long _Accum sula1 = 0.987654321uk;
    
    // Cross-type operations forcing conversions
    signed long _Accum mixed1 = (signed long _Accum)sf1 * sla1;
    signed long _Accum mixed2 = (signed long _Accum)uf1 + sla1;
    
    // Division with different precisions
    signed short _Fract div_result = sf1 / (signed short _Fract)0.25r;
    
    // Built-in overflow checks with fixed-point
    signed long _Accum overflow_check;
    int overflow_flag = __builtin_mul_overflow(mixed1, mixed2, &overflow_check);
    
    // Control flow dependent on mixed precision comparisons
    if (mixed1 > (signed long _Accum)0.4lk && 
        mixed2 < (signed long _Accum)1.5lk &&
        !overflow_flag) {
        volatile signed long _Accum vol_result = mixed1 + mixed2 + (signed long _Accum)div_result;
        (void)vol_result; // Use to prevent elimination
    }
}

// Struct containing fixed-point values
struct FixedPointContainer {
    unsigned short _Fract fract_array[4];
    _Sat unsigned long _Accum sat_accum;
    signed long _Accum signed_accum;
    int count;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointContainer *container) {
    // Initialize struct with boundary values
    container->fract_array[0] = 0.0r;
    container->fract_array[1] = 0.9999r;
    container->fract_array[2] = 0.5r;
    container->fract_array[3] = 0.0001r;
    
    container->sat_accum = 0.99999999999999999999uk;
    container->signed_accum = -0.5lk;
    container->count = 0;
    
    // Array operations forcing range analysis
    for (int i = 0; i < 4; i++) {
        // Multiplication across array elements
        if (i > 0) {
            container->fract_array[i] = container->fract_array[i] * container->fract_array[i-1];
        }
        
        // Comparison with struct members
        if (container->fract_array[i] > (unsigned short _Fract)0.25r) {
            container->sat_accum = container->sat_accum * (_Sat unsigned long _Accum)0.9uk;
            container->count++;
        }
    }
    
    // Final boundary check
    if (container->sat_accum < (_Sat unsigned long _Accum)0.5uk &&
        container->signed_accum > (signed long _Accum)-1.0lk) {
        container->signed_accum = container->signed_accum * (signed long _Accum)2.0lk;
    }
}

__attribute__((optimize("O3")))
int main(void) {
    // Arrays to store results (prevent dead code elimination)
    unsigned short _Fract fract_results[32];
    _Sat unsigned long _Accum accum_results[32];
    int fract_count = 0;
    int accum_idx = 0;
    
    // Test 1: Short fract range analysis
    test_short_fract_range(fract_results, &fract_count);
    
    // Test 2: Saturated accum operations
    test_sat_accum_range(accum_results, &accum_idx);
    
    // Test 3: Mixed precision operations
    test_mixed_precision_ops();
    
    // Test 4: Struct-based operations
    struct FixedPointContainer container;
    test_struct_operations(&container);
    
    // Aggregate results to volatile to prevent optimization
    volatile unsigned short _Fract total_fract = 0.0r;
    volatile _Sat unsigned long _Accum total_accum = 0.0uk;
    
    for (int i = 0; i < fract_count && i < 32; i++) {
        total_fract = total_fract + fract_results[i];
    }
    
    for (int i = 0; i < accum_idx && i < 32; i++) {
        total_accum = total_accum + accum_results[i];
    }
    
    // Add struct results
    for (int i = 0; i < 4; i++) {
        total_fract = total_fract + container.fract_array[i];
    }
    total_accum = total_accum + container.sat_accum;
    
    // Print to prevent elimination (but don't rely on specific values)
    printf("Fract total: %u\n", (unsigned)(total_fract * 10000));
    printf("Accum total: %llu\n", (unsigned long long)(total_accum * 10000000000000000000ull));
    printf("Container count: %d\n", container.count);
    
    return 0;
}
