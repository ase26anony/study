#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract zero_val = 0.0r;
    
    // Mixed operations to force range analysis
    for (int i = 0; i < 8; i++) {
        unsigned short _Fract x = max_val - (i * 0.125r);
        unsigned short _Fract y = min_val + (i * 0.125r);
        
        // Multiplication near boundaries
        unsigned short _Fract prod = x * y;
        
        // Division that approaches extremes
        unsigned short _Fract div = x / (y + 0.001r);
        
        // Boundary comparisons - should trigger range analysis
        if (prod > 0.5r) {
            results[*count] = prod;
            (*count)++;
        }
        
        if (div < 0.25r && div > 0.0r) {
            results[*count] = div;
            (*count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat signed long _Accum *results, int *count) {
    // Initialize saturation accumulators at boundaries
    _Sat signed long _Accum max_sat = 0.999999999999999999lk;
    _Sat signed long _Accum min_sat = -0.999999999999999999lk;
    _Sat signed long _Accum mid_sat = 0.0lk;
    
    // Operations that should saturate
    for (int i = 0; i < 4; i++) {
        _Sat signed long _Accum a = max_sat + (i * 0.25lk);
        _Sat signed long _Accum b = min_sat - (i * 0.25lk);
        
        // These should trigger saturation and range analysis
        _Sat signed long _Accum sum1 = a + 0.5lk;  // Should saturate
        _Sat signed long _Accum sum2 = b - 0.5lk;  // Should saturate
        
        // Mixed precision multiplication
        _Sat signed long _Accum prod = a * b;
        
        // Boundary comparisons - critical for uncovered code
        if (sum1 > 0.9lk || sum1 == max_sat) {
            results[*count] = sum1;
            (*count)++;
        }
        
        if (sum2 < -0.9lk || sum2 == min_sat) {
            results[*count] = sum2;
            (*count)++;
        }
        
        // Product range check
        if (prod > 0.0lk && prod < 0.25lk) {
            results[*count] = prod;
            (*count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_range(void) {
    // Mixed fixed-point types to force conversions
    unsigned short _Fract usf = 0.75r;
    signed short _Fract ssf = -0.5r;
    _Sat unsigned long _Accum ula = 0.8lk;
    _Sat signed long _Accum sla = -0.3lk;
    
    // Cross-type operations
    signed long _Accum mixed1 = (_Sat signed long _Accum)(usf * ula);
    signed long _Accum mixed2 = (_Sat signed long _Accum)(ssf / sla);
    
    // Use builtins with fixed-point to check overflow
    signed long _Accum result1, result2;
    int overflow1 = __builtin_mul_overflow((signed long _Accum)usf, 
                                          (signed long _Accum)ula, &result1);
    int overflow2 = __builtin_add_overflow((signed long _Accum)ssf, 
                                          (signed long _Accum)sla, &result2);
    
    // Complex boundary conditions
    if ((mixed1 > 0.6lk && mixed1 < 0.9lk) || 
        (mixed2 < -0.1lk && mixed2 > -0.5lk)) {
        volatile signed long _Accum v = mixed1 + mixed2;
        (void)v;  // Prevent elimination
    }
    
    if (overflow1 || overflow2) {
        volatile int v = overflow1 + overflow2;
        (void)v;
    }
}

// Struct with fixed-point members
struct FixedPointContainer {
    _Sat unsigned short _Fract sat_fract;
    signed long _Accum accum;
    unsigned short _Fract fract_array[4];
};

__attribute__((optimize("O3")))
void test_struct_range_analysis(struct FixedPointContainer *container) {
    // Initialize struct with boundary values
    container->sat_fract = 0.9999r;
    container->accum = -0.999999999999999999lk;
    
    // Array operations
    for (int i = 0; i < 4; i++) {
        container->fract_array[i] = i * 0.25r;
    }
    
    // Operations on struct members
    _Sat unsigned short _Fract temp = container->sat_fract;
    for (int i = 0; i < 4; i++) {
        temp = temp * container->fract_array[i];
        
        // Boundary comparisons in loop
        if (temp > 0.5r || temp < 0.1r) {
            container->accum += (_Sat signed long _Accum)temp;
        }
    }
    
    // Final boundary check
    if (container->accum > 0.0lk || container->accum < -1.0lk) {
        volatile signed long _Accum v = container->accum;
        (void)v;
    }
}

__attribute__((optimize("O3")))
int main(void) {
    // Arrays to store results
    unsigned short _Fract fract_results[16];
    _Sat signed long _Accum accum_results[16];
    int fract_count = 0;
    int accum_count = 0;
    
    // Test 1: Short fract range analysis
    test_short_fract_range(fract_results, &fract_count);
    
    // Test 2: Saturation accum range analysis
    test_sat_accum_range(accum_results, &accum_count);
    
    // Test 3: Mixed precision operations
    test_mixed_precision_range();
    
    // Test 4: Struct-based range analysis
    struct FixedPointContainer container;
    test_struct_range_analysis(&container);
    
    // Aggregate results to prevent dead code elimination
    volatile unsigned short _Fract total_fract = 0.0r;
    volatile _Sat signed long _Accum total_accum = 0.0lk;
    
    for (int i = 0; i < fract_count; i++) {
        total_fract += fract_results[i];
    }
    
    for (int i = 0; i < accum_count; i++) {
        total_accum += accum_results[i];
    }
    
    // Print to prevent optimization
    printf("Fract results: %d, Accum results: %d\n", 
           fract_count, accum_count);
    printf("Total fract: %u, Total accum: %ld\n", 
           (unsigned)(total_fract * 65536), 
           (long)(total_accum * 1000000));
    
    return 0;
}
