#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract zero = 0.0r;
    
    // Operations that force range analysis
    unsigned short _Fract prod1 = max_val * max_val;
    unsigned short _Fract prod2 = min_val * max_val;
    unsigned short _Fract div1 = max_val / min_val;
    
    // Boundary comparisons that should trigger the uncovered logic
    if (prod1 > 0.9r) {
        results[0] = prod1;
    }
    if (prod2 < 0.1r) {
        results[1] = prod2;
    }
    // This division may approach infinity, testing max range
    if (div1 > 0.5r) {
        results[2] = div1;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat unsigned long _Accum *results) {
    // Saturation arithmetic at boundaries
    _Sat unsigned long _Accum sat_max = 0xFFFFFFFFFFFFFFFFP-32;  // Max value
    _Sat unsigned long _Accum sat_min = 0.0000000001k;
    _Sat unsigned long _Accum sat_mid = 0.5k;
    
    // Operations that should saturate
    _Sat unsigned long _Accum sum1 = sat_max + sat_mid;  // Should saturate
    _Sat unsigned long _Accum sum2 = sat_min + sat_mid;
    _Sat unsigned long _Accum prod1 = sat_max * sat_mid;
    
    // Built-in overflow checks that interact with range analysis
    int overflow1, overflow2;
    _Sat unsigned long _Accum test1 = __builtin_mul_overflow(sat_max, 2.0k, &overflow1);
    _Sat unsigned long _Accum test2 = __builtin_add_overflow(sat_max, sat_max, &overflow2);
    
    // Conditional logic based on saturation results
    if (sum1 == sat_max) {  // Check if saturated to max
        results[0] = sum1;
    }
    if (prod1 > 0.25k) {
        results[1] = prod1;
    }
    if (overflow1 || overflow2) {
        results[2] = test1 + test2;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision(_Fract *results) {
    // Mixed precision operations
    unsigned short _Fract usf = 0.75r;
    signed long _Accum sla = -0.5k;
    _Fract f1 = 0.25r;
    
    // Cross-type operations forcing conversions
    signed long _Accum mixed1 = sla * (_Accum)usf;
    _Fract mixed2 = f1 * (_Fract)sla;
    
    // Range-dependent control flow
    if (mixed1 < 0.0k) {
        results[0] = (_Fract)mixed1;
    }
    if (mixed2 > -0.1r && mixed2 < 0.1r) {
        results[1] = mixed2;
    }
    
    // Boundary value testing
    _Fract boundary_test = 0.9999r * 0.9999r;
    if (boundary_test > 0.99r) {
        results[2] = boundary_test;
    }
}

// Struct with fixed-point members
struct FixedPointStruct {
    _Sat unsigned short _Fract saturated;
    signed _Accum signed_acc;
    unsigned _Fract unsigned_frac;
    _Sat signed long _Accum long_sat;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int size) {
    for (int i = 0; i < size; i++) {
        // Operations on struct members
        arr[i].saturated = arr[i].saturated * 0.9r;
        arr[i].signed_acc = arr[i].signed_acc + (_Accum)arr[i].unsigned_frac;
        arr[i].long_sat = arr[i].long_sat / 2.0k;
        
        // Conditional based on struct member comparisons
        if (arr[i].signed_acc > (_Accum)arr[i].unsigned_frac) {
            arr[i].unsigned_frac = arr[i].unsigned_frac * 0.5r;
        }
        
        // Boundary check that should trigger the uncovered comparison logic
        if (arr[i].long_sat > 0.9k || arr[i].long_sat < -0.9k) {
            arr[i].saturated = 0.5r;
        }
    }
}

// Array-based fixed-point operations
__attribute__((optimize("O3")))
void test_array_range(_Fract arr[], int size) {
    _Fract max_val = 0.0r;
    _Fract min_val = 1.0r;
    
    for (int i = 0; i < size; i++) {
        // Find min/max in array - forces range comparisons
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
        
        // Transform values
        arr[i] = arr[i] * 0.9r + 0.1r;
    }
    
    // Final comparison that should exercise the uncovered block
    _Fract threshold = 0.5r;
    if (max_val > threshold || (max_val == threshold && min_val < 0.1r)) {
        arr[0] = max_val - min_val;
    }
}

int main() {
    // Volatile to prevent dead code elimination
    volatile _Fract final_result = 0.0r;
    
    // Test 1: Short fract range analysis
    unsigned short _Fract fract_results[3] = {0.0r, 0.0r, 0.0r};
    test_short_fract_range(fract_results);
    final_result += (_Fract)fract_results[0] + (_Fract)fract_results[1] + (_Fract)fract_results[2];
    
    // Test 2: Saturation accum range analysis
    _Sat unsigned long _Accum sat_results[3] = {0.0k, 0.0k, 0.0k};
    test_sat_accum_range(sat_results);
    final_result += (_Fract)sat_results[0] + (_Fract)sat_results[1] + (_Fract)sat_results[2];
    
    // Test 3: Mixed precision operations
    _Fract mixed_results[3] = {0.0r, 0.0r, 0.0r};
    test_mixed_precision(mixed_results);
    final_result += mixed_results[0] + mixed_results[1] + mixed_results[2];
    
    // Test 4: Struct operations
    struct FixedPointStruct struct_arr[4] = {
        {0.9r, 0.5k, 0.25r, 0.75k},
        {0.1r, -0.5k, 0.75r, -0.25k},
        {0.5r, 0.9k, 0.1r, 0.9k},
        {0.75r, -0.9k, 0.9r, -0.9k}
    };
    test_struct_operations(struct_arr, 4);
    final_result += (_Fract)struct_arr[0].saturated + (_Fract)struct_arr[1].signed_acc 
                   + (_Fract)struct_arr[2].unsigned_frac + (_Fract)struct_arr[3].long_sat;
    
    // Test 5: Array range analysis
    _Fract array_data[8] = {0.1r, 0.2r, 0.3r, 0.4r, 0.5r, 0.6r, 0.7r, 0.8r};
    test_array_range(array_data, 8);
    for (int i = 0; i < 8; i++) {
        final_result += array_data[i];
    }
    
    // Print result to prevent optimization
    printf("Final fixed-point result: %f\n", (double)final_result);
    
    return 0;
}
