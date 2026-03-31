#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract zero = 0.0r;
    
    // Operations that force range analysis
    unsigned short _Fract prod = max_val * max_val;
    unsigned short _Fract quot = max_val / min_val;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (prod > max_val) {
        results[(*count)++] = prod;
    }
    
    // Force comparison with extended precision
    if (quot == max_val) {
        results[(*count)++] = quot;
    }
    
    // Nested conditions for complex range analysis
    if (zero < min_val && min_val < max_val) {
        results[(*count)++] = zero;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat unsigned long _Accum *results, int *count) {
    // Saturation arithmetic at boundaries
    _Sat unsigned long _Accum sat_max = 0xFFFFFFFFFFFFFFFFP-32;  // Max value
    _Sat unsigned long _Accum sat_min = 0.0k;
    _Sat unsigned long _Accum mid = 0.5k;
    
    // Operations that should saturate
    _Sat unsigned long _Accum saturated_sum = sat_max + sat_max;
    _Sat unsigned long _Accum saturated_prod = sat_max * sat_max;
    
    // Use builtins for overflow detection
    _Sat unsigned long _Accum overflow_check;
    int overflow = __builtin_mul_overflow(sat_max, sat_max, &overflow_check);
    
    // Boundary comparisons
    if (saturated_sum == sat_max) {  // Should be true due to saturation
        results[(*count)++] = saturated_sum;
    }
    
    if (overflow) {
        results[(*count)++] = overflow_check;
    }
    
    // Complex condition with multiple comparisons
    if (saturated_prod > mid && saturated_prod <= sat_max) {
        results[(*count)++] = saturated_prod;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision(_Fract *fract_results, _Accum *accum_results, int *idx) {
    // Mixed precision operations
    _Fract f1 = 0.75r;
    _Fract f2 = 0.25r;
    _Accum a1 = 1.5k;
    _Accum a2 = 2.5k;
    
    // Cross-type operations forcing conversions
    _Accum mixed_prod = (_Accum)f1 * a1;
    _Fract mixed_div = (_Fract)(a2 / (_Accum)f2);
    
    // Range comparisons after conversions
    if (mixed_prod > (_Accum)f1) {
        accum_results[(*idx)++] = mixed_prod;
    }
    
    if (mixed_div < f2) {
        fract_results[(*idx)++] = mixed_div;
    }
    
    // Chain of comparisons
    _Accum temp = mixed_prod + a2;
    if (temp > a1 && temp < a2 * 2) {
        accum_results[(*idx)++] = temp;
    }
}

// Struct with fixed-point members
struct FixedPointContainer {
    _Fract fract_member;
    _Sat unsigned short _Accum sat_member;
    signed long _Accum accum_member;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointContainer *container, 
                           struct FixedPointContainer *result) {
    // Operations on struct members
    container->fract_member = 0.8r;
    container->sat_member = 0.9hk;
    container->accum_member = 1.2lk;
    
    // Arithmetic forcing range analysis
    result->fract_member = container->fract_member * 0.5r;
    result->sat_member = container->sat_member + 0.1hk;
    result->accum_member = container->accum_member / 2.0lk;
    
    // Comparisons that should trigger boundary checks
    if (result->fract_member > container->fract_member) {
        result->fract_member = container->fract_member;
    }
    
    if (result->sat_member < container->sat_member) {
        result->sat_member = container->sat_member;
    }
}

__attribute__((optimize("O3")))
void test_array_boundaries(_Fract arr[], int size, _Fract *output) {
    // Initialize array with boundary values
    for (int i = 0; i < size; i++) {
        arr[i] = (_Fract)i / (_Fract)size;
    }
    
    // Operations on array elements
    _Fract sum = 0.0r;
    _Fract prod = 1.0r;
    
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        prod *= arr[i];
        
        // Conditional based on array values
        if (arr[i] > 0.5r) {
            *output = arr[i];
        }
    }
    
    // Final comparison that might trigger the uncovered logic
    if (sum > 0.5r && prod < 0.1r) {
        *output = sum;
    }
}

__attribute__((optimize("O3")))
void test_complex_conditions(_Sat signed short _Accum *vals, int n) {
    // Create values that approach boundaries
    for (int i = 0; i < n; i++) {
        vals[i] = (_Sat signed short _Accum)(i - n/2) / (_Sat signed short _Accum)n;
    }
    
    // Complex nested conditions
    for (int i = 1; i < n - 1; i++) {
        if (vals[i] > vals[i-1]) {
            if (vals[i] < vals[i+1]) {
                if (vals[i] > 0.0hk && vals[i] < 1.0hk) {
                    // This should force range analysis with high/low parts
                    vals[i] = vals[i] * 2.0hk;
                }
            }
        }
    }
}

int main() {
    // Arrays to store results (prevent dead code elimination)
    volatile unsigned short _Fract fract_results[10];
    volatile _Sat unsigned long _Accum sat_results[10];
    volatile _Fract mixed_fract_results[5];
    volatile _Accum mixed_accum_results[5];
    volatile _Fract array_output;
    volatile _Sat signed short _Accum complex_vals[20];
    
    int fract_count = 0;
    int sat_count = 0;
    int mixed_idx = 0;
    
    // Test 1: Short fract range analysis
    test_short_fract_range((unsigned short _Fract *)fract_results, &fract_count);
    
    // Test 2: Saturation accum range analysis
    test_sat_accum_range((_Sat unsigned long _Accum *)sat_results, &sat_count);
    
    // Test 3: Mixed precision operations
    test_mixed_precision((_Fract *)mixed_fract_results, 
                        (_Accum *)mixed_accum_results, &mixed_idx);
    
    // Test 4: Struct operations
    struct FixedPointContainer container1, container2;
    test_struct_operations(&container1, &container2);
    
    // Test 5: Array boundaries
    _Fract arr[10];
    test_array_boundaries(arr, 10, (_Fract *)&array_output);
    
    // Test 6: Complex conditions
    test_complex_conditions((_Sat signed short _Accum *)complex_vals, 20);
    
    // Aggregate results to prevent optimization
    _Fract final_result = 0.0r;
    for (int i = 0; i < fract_count && i < 10; i++) {
        final_result += (_Fract)fract_results[i];
    }
    
    // Print to prevent dead code elimination
    printf("Test completed. Final aggregated value influenced range analysis.\n");
    printf("Fract results count: %d\n", fract_count);
    printf("Sat results count: %d\n", sat_count);
    
    return 0;
}
