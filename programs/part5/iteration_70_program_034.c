#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int n) {
    // Initialize with boundary values
    unsigned short _Fract max_val = 0.9999r;  // Near maximum
    unsigned short _Fract min_val = 0.0r;     // Minimum
    unsigned short _Fract mid_val = 0.5r;
    
    for (int i = 0; i < n; i++) {
        // Mixed operations to force range analysis
        unsigned short _Fract val = min_val + (max_val - min_val) * (i / (n - 1.0r));
        
        // Boundary comparisons that should trigger the uncovered code
        if (val > max_val) {
            results[i] = max_val;
        } else if (val < min_val) {
            results[i] = min_val;
        } else {
            // Multiplication that may approach boundaries
            results[i] = val * mid_val;
        }
        
        // Update for next iteration
        min_val = results[i] * 0.9r;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_boundaries(_Sat signed long _Accum *acc, int count) {
    // Initialize with extreme values
    _Sat signed long _Accum max_sat = 0.9999999999999999999lk;  // Near max
    _Sat signed long _Accum min_sat = -0.9999999999999999999lk; // Near min
    _Sat signed long _Accum zero = 0.0lk;
    
    for (int i = 0; i < count; i++) {
        // Operations that will saturate at boundaries
        _Sat signed long _Accum temp = acc[i];
        
        // Force saturation arithmetic
        temp = temp * 1.5lk;  // May overflow
        
        // Explicit boundary comparisons
        if (temp > max_sat) {
            acc[i] = max_sat;
        } else if (temp < min_sat) {
            acc[i] = min_sat;
        } else if (temp == zero) {
            acc[i] = zero + 0.1lk;
        } else {
            // Mixed precision operation
            unsigned short _Fract f = 0.7r;
            _Sat signed long _Accum converted = (_Sat signed long _Accum)f;
            acc[i] = temp * converted;
        }
        
        // Built-in overflow check with fixed-point
        _Sat signed long _Accum check;
        if (__builtin_mul_overflow(acc[i], 2.0lk, &check)) {
            acc[i] = (acc[i] > 0) ? max_sat : min_sat;
        }
    }
}

__attribute__((optimize("O3")))
_Sat unsigned long _Accum test_mixed_precision_ops(
    _Sat unsigned long _Accum a,
    _Sat unsigned long _Accum b,
    unsigned short _Fract c) {
    
    // Mixed precision operations
    _Sat unsigned long _Accum result;
    
    // Convert and scale
    _Sat unsigned long _Accum converted = (_Sat unsigned long _Accum)c;
    
    // Operations that force range analysis
    result = a * b;
    
    // Division that may produce small values
    if (b != 0.0ulk) {
        _Sat unsigned long _Accum div_result = a / b;
        
        // Comparison that should trigger the uncovered condition
        if (div_result > result) {
            result = div_result;
        } else if (div_result == result) {
            result = result * converted;
        }
    }
    
    // Built-in addition overflow check
    _Sat unsigned long _Accum sum;
    if (__builtin_add_overflow(result, converted, &sum)) {
        result = 0.9999999999999999999ulk;  // Max saturated value
    } else {
        result = sum;
    }
    
    return result;
}

// Struct containing fixed-point values
struct FixedPointContainer {
    _Sat signed long _Accum accum;
    unsigned short _Fract fract;
    _Sat unsigned long _Accum uaccum;
    int counter;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointContainer *arr, int size) {
    // Initialize array with boundary values
    for (int i = 0; i < size; i++) {
        arr[i].accum = (i % 2 == 0) ? 
            0.9999999999999999999lk : 
            -0.9999999999999999999lk;
        arr[i].fract = (i % 3 == 0) ? 0.9999r : 0.0001r;
        arr[i].uaccum = 0.9999999999999999999ulk * (i / (size - 1.0ulk));
        arr[i].counter = i;
    }
    
    // Perform operations on struct members
    for (int i = 0; i < size - 1; i++) {
        // Mixed operations between struct members
        _Sat signed long _Accum temp = arr[i].accum;
        
        // Convert and mix types
        _Sat signed long _Accum converted = (_Sat signed long _Accum)arr[i].fract;
        
        // Operation that may approach boundaries
        arr[i].accum = temp * converted;
        
        // Comparison that should trigger range analysis
        if (arr[i].accum > arr[i + 1].accum) {
            // Swap or adjust values
            _Sat signed long _Accum swap = arr[i].accum;
            arr[i].accum = arr[i + 1].accum;
            arr[i + 1].accum = swap;
        }
        
        // Additional boundary check
        if (arr[i].uaccum > 0.8ulk && arr[i].fract > 0.5r) {
            arr[i].uaccum = arr[i].uaccum * 0.9ulk;
        }
    }
}

int main() {
    // Test 1: Short fract range analysis
    unsigned short _Fract fract_results[10];
    test_short_fract_range(fract_results, 10);
    
    // Test 2: Saturated accum boundaries
    _Sat signed long _Accum accum_array[5] = {
        0.5lk, -0.5lk, 0.9lk, -0.9lk, 0.0lk
    };
    test_sat_accum_boundaries(accum_array, 5);
    
    // Test 3: Mixed precision operations
    _Sat unsigned long _Accum mixed_result = test_mixed_precision_ops(
        0.7ulk, 0.3ulk, 0.8r);
    
    // Test 4: Struct operations
    struct FixedPointContainer fp_array[5];
    test_struct_operations(fp_array, 5);
    
    // Aggregate results to prevent dead code elimination
    volatile _Sat signed long _Accum total = 0.0lk;
    
    // Sum fract results (converted)
    for (int i = 0; i < 10; i++) {
        total += (_Sat signed long _Accum)fract_results[i];
    }
    
    // Sum accum array
    for (int i = 0; i < 5; i++) {
        total += accum_array[i];
    }
    
    // Add mixed result
    total += (_Sat signed long _Accum)mixed_result;
    
    // Add struct accum values
    for (int i = 0; i < 5; i++) {
        total += fp_array[i].accum;
    }
    
    // Print to ensure no dead code elimination
    printf("Total (volatile): %lld\n", (long long)(total * 1000000lk));
    
    return 0;
}
