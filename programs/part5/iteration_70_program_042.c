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
    for (int i = 0; i < 10; i++) {
        unsigned short _Fract x = max_val - (i * 0.1r);
        unsigned short _Fract y = min_val + (i * 0.05r);
        
        // Multiplication near boundaries
        unsigned short _Fract prod = x * y;
        
        // Division forcing range contraction
        unsigned short _Fract div = (i == 0) ? max_val : prod / y;
        
        // Critical comparison that should trigger the uncovered logic
        if (prod > max_val || (prod == max_val && div > zero)) {
            results[*count] = prod;
            (*count)++;
        }
        
        // Force boundary value comparison
        if (x > max_val || (x == max_val && y > zero)) {
            results[*count] = x;
            (*count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat unsigned long _Accum *results, int *count) {
    // Saturation arithmetic at extremes
    _Sat unsigned long _Accum sat_max = 0x7FFFFFFFFFFFFFFFk;  // Near max
    _Sat unsigned long _Accum sat_min = 0.0000000000000001k;  // Near min
    _Sat unsigned long _Accum mid = 0.5k;
    
    // Operations that should saturate
    for (int i = 0; i < 5; i++) {
        _Sat unsigned long _Accum a = sat_max + (i * 0.1k);
        _Sat unsigned long _Accum b = sat_min * (i + 1);
        
        // Built-in overflow checks interacting with range analysis
        int overflow;
        _Sat unsigned long _Accum mul_result = __builtin_mul_overflow(a, b, &overflow) ? sat_max : a * b;
        
        // Mixed with non-sat for comparison
        unsigned long _Accum non_sat = 0.75k;
        
        // Boundary comparisons triggering the uncovered code
        if (mul_result > sat_max || (mul_result == sat_max && b > sat_min)) {
            results[*count] = mul_result;
            (*count)++;
        }
        
        // Additional overflow check
        _Sat unsigned long _Accum add_result;
        if (__builtin_add_overflow(a, mid, &add_result)) {
            results[*count] = sat_max;
            (*count)++;
        }
    }
}

// Struct with mixed fixed-point types
struct FixedPointStruct {
    signed short _Fract f1;
    unsigned long _Accum a1;
    _Sat signed _Fract sf1;
    signed _Accum a2;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int size, 
                           signed _Accum *results, int *res_count) {
    // Initialize array with boundary values
    for (int i = 0; i < size; i++) {
        arr[i].f1 = (i % 2) ? 0.9999r : -0.9999r;
        arr[i].a1 = (i * 0.125k);
        arr[i].sf1 = (i % 3) ? 0.5r : -0.5r;
        arr[i].a2 = 0.0k;
    }
    
    // Perform operations on struct members
    for (int i = 0; i < size - 1; i++) {
        // Mixed type operations forcing conversions
        signed _Accum temp = (signed _Accum)arr[i].f1 * arr[i + 1].a1;
        
        // Saturation operation
        _Sat signed _Fract sat_temp = arr[i].sf1 * arr[i + 1].sf1;
        
        // Update with mixed operation
        arr[i].a2 = temp + (signed _Accum)sat_temp;
        
        // Boundary comparison that should trigger the uncovered logic
        if (arr[i].a2 > 0.999k || (arr[i].a2 == 0.999k && temp > 0.0k)) {
            results[*res_count] = arr[i].a2;
            (*res_count)++;
        }
        
        // Additional comparison with negative boundary
        if (arr[i].a2 < -0.999k || (arr[i].a2 == -0.999k && temp < 0.0k)) {
            results[*res_count] = arr[i].a2;
            (*res_count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Different fractional bit configurations
    unsigned short _Fract usf = 0.75r;
    signed _Fract sf = -0.25r;
    unsigned _Accum ua = 0.9999999k;
    signed long _Accum sla = -0.0000001k;
    
    // Chain of mixed operations forcing range analysis
    signed _Accum result1 = (signed _Accum)usf * (signed _Accum)sf;
    unsigned long _Accum result2 = (unsigned long _Accum)ua + (unsigned long _Accum)(-sla);
    
    // Division near boundaries
    signed _Fract div_result = (sf == 0.0r) ? 0.0r : usf / sf;
    
    // Comparison that should exercise the double-int range logic
    volatile int cmp1 = (result1 > 0.5k) || (result1 == 0.5k && result2 > 0.0k);
    volatile int cmp2 = (div_result < -1.0r) || (div_result == -1.0r && result1 < 0.0k);
    
    // Use results to prevent elimination
    (void)cmp1;
    (void)cmp2;
}

int main() {
    // Arrays to store results (volatile to prevent optimization)
    volatile unsigned short _Fract fract_results[20];
    volatile _Sat unsigned long _Accum sat_results[10];
    volatile signed _Accum struct_results[15];
    
    int fract_count = 0;
    int sat_count = 0;
    int struct_count = 0;
    
    // Test 1: Short fract with boundary comparisons
    test_short_fract_range((unsigned short _Fract *)fract_results, &fract_count);
    
    // Test 2: Saturation accum with overflow checks
    test_sat_accum_range((_Sat unsigned long _Accum *)sat_results, &sat_count);
    
    // Test 3: Struct operations with mixed types
    struct FixedPointStruct fp_array[10];
    test_struct_operations(fp_array, 10, (signed _Accum *)struct_results, &struct_count);
    
    // Test 4: Mixed precision conversions
    test_mixed_precision_conversions();
    
    // Aggregate results to prevent dead code elimination
    volatile signed long _Accum final_result = 0.0k;
    
    for (int i = 0; i < fract_count; i++) {
        final_result += (signed long _Accum)fract_results[i];
    }
    
    for (int i = 0; i < sat_count; i++) {
        final_result += (signed long _Accum)sat_results[i];
    }
    
    for (int i = 0; i < struct_count; i++) {
        final_result += (signed long _Accum)struct_results[i];
    }
    
    // Print to ensure execution
    printf("Final aggregated result (as double): %f\n", (double)final_result);
    printf("Test counts: fract=%d, sat=%d, struct=%d\n", 
           fract_count, sat_count, struct_count);
    
    return 0;
}
