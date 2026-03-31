#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int n) {
    // Initialize near boundaries
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0001r;
    unsigned short _Fract mid_val = 0.5r;
    
    for (int i = 0; i < n; i++) {
        // Operations that approach boundaries
        unsigned short _Fract val = mid_val;
        
        // Multiply to approach max boundary
        for (int j = 0; j < 3; j++) {
            val = val * 1.8r;  // Will saturate for unsigned fract
        }
        
        // Compare with boundary values - triggers range analysis
        if (val > max_val) {
            val = max_val;
        } else if (val < min_val) {
            val = min_val;
        }
        
        // Division to approach min boundary
        val = val / 2.0r;
        
        // Boundary comparison that should trigger the uncovered code
        if (val == max_val || val == min_val) {
            results[i] = val;
        } else {
            results[i] = mid_val;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat signed long _Accum *vals, int count) {
    // Initialize with boundary values
    _Sat signed long _Accum max_bound = 9223372036854775.807k;  // Near max
    _Sat signed long _Accum min_bound = -9223372036854775.808k; // Near min
    _Sat signed long _Accum zero = 0.0k;
    
    for (int i = 0; i < count; i++) {
        // Create values that will saturate
        _Sat signed long _Accum a = max_bound * 1.5k;  // Should saturate
        _Sat signed long _Accum b = min_bound * 1.5k;  // Should saturate
        
        // Mixed precision operations
        signed short _Fract c = 0.7r;
        _Sat signed long _Accum d = a * c;  // Type conversion with range analysis
        
        // Use builtins for overflow detection
        _Sat signed long _Accum result;
        int overflow = __builtin_mul_overflow(a, b, &result);
        
        // Complex boundary comparison - should trigger the uncovered logic
        if (a > max_bound || (a == max_bound && d > zero)) {
            vals[i] = max_bound;
        } else if (b < min_bound || (b == min_bound && d < zero)) {
            vals[i] = min_bound;
        } else {
            vals[i] = result;
        }
    }
}

// Struct containing mixed fixed-point types
struct FixedPointStruct {
    unsigned short _Fract fract_val;
    _Sat signed long _Accum accum_val;
    signed _Fract signed_fract;
    _Sat unsigned long _Accum usat_accum;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int size) {
    // Initialize array with boundary values
    for (int i = 0; i < size; i++) {
        arr[i].fract_val = (i % 2) ? 0.9999r : 0.0001r;
        arr[i].accum_val = (i % 3) ? 9223372036854775.807k : -9223372036854775.808k;
        arr[i].signed_fract = (i % 4) ? 0.9999r : -0.9999r;
        arr[i].usat_accum = (i % 5) ? 18446744073709551.615uk : 0.0uk;
    }
    
    // Perform operations that require range analysis
    for (int i = 0; i < size - 1; i++) {
        // Mixed type operations
        _Sat signed long _Accum temp = arr[i].accum_val * arr[i].signed_fract;
        
        // Boundary comparisons in struct context
        if (arr[i].accum_val > arr[i + 1].accum_val) {
            // Operations that might trigger the specific comparison pattern
            _Sat signed long _Accum diff = arr[i].accum_val - arr[i + 1].accum_val;
            
            // Complex condition similar to uncovered code
            if (temp > diff || (temp == diff && arr[i].fract_val > arr[i + 1].fract_val)) {
                arr[i].accum_val = diff / 2.0k;
            }
        }
        
        // Saturation arithmetic with overflow check
        _Sat unsigned long _Accum sum;
        int overflow = __builtin_add_overflow(arr[i].usat_accum, 
                                            arr[i + 1].usat_accum, 
                                            &sum);
        
        if (overflow) {
            arr[i].usat_accum = 18446744073709551.615uk;  // Max value
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_conversions() {
    // Create values at different precision boundaries
    unsigned short _Fract usf1 = 0.9999r;
    signed _Fract sf1 = -0.9999r;
    _Sat signed long _Accum sla1 = 9223372036854775.807k;
    _Sat unsigned long _Accum ula1 = 18446744073709551.615uk;
    
    // Chain of mixed-precision operations
    _Sat signed long _Accum result1 = sla1 * usf1;  // Conversion usf1 -> _Accum
    signed _Fract result2 = sf1 * 0.5r;
    _Sat unsigned long _Accum result3 = ula1 + (ula1 * 0.25uk);
    
    // Nested comparisons that should trigger range analysis
    for (int i = 0; i < 10; i++) {
        // Vary the values
        usf1 = usf1 / 1.1r;
        sla1 = sla1 * 0.9k;
        
        // Complex boundary condition - similar to uncovered code
        if (sla1 > result1 || (sla1 == result1 && usf1 > result2)) {
            // Force evaluation of both high and low parts
            _Sat signed long _Accum temp = sla1 + result1;
            
            // Additional boundary check
            if (temp < -9223372036854775.808k) {
                sla1 = -9223372036854775.808k;
            }
        }
    }
}

int main() {
    const int ARRAY_SIZE = 100;
    
    // Test 1: Short fract range analysis
    unsigned short _Fract fract_results[ARRAY_SIZE];
    test_short_fract_range(fract_results, ARRAY_SIZE);
    
    // Test 2: Saturated accum range analysis
    _Sat signed long _Accum accum_vals[ARRAY_SIZE];
    test_sat_accum_range(accum_vals, ARRAY_SIZE);
    
    // Test 3: Struct operations with mixed types
    struct FixedPointStruct struct_arr[ARRAY_SIZE];
    test_struct_operations(struct_arr, ARRAY_SIZE);
    
    // Test 4: Mixed precision conversions
    test_mixed_precision_conversions();
    
    // Aggregate results to prevent dead code elimination
    volatile _Sat signed long _Accum total = 0.0k;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total += accum_vals[i];
        total += fract_results[i];  // Implicit conversion
        total += struct_arr[i].accum_val;
    }
    
    // Print to prevent optimization
    printf("Result: %Lf\n", (long double)total);
    
    return 0;
}
