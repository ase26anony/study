#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize with boundary values
    unsigned short _Fract max_val = 0.9999r;  // Near maximum
    unsigned short _Fract min_val = 0.0r;     // Minimum
    unsigned short _Fract mid_val = 0.5r;
    
    // Operations that force range analysis
    for (int i = 0; i < 8; i++) {
        unsigned short _Fract x = min_val + (i * 0.125r);
        
        // Multiplication near boundaries
        unsigned short _Fract prod = x * max_val;
        
        // Division that could approach limits
        unsigned short _Fract div_result = (i > 0) ? max_val / x : 0.0r;
        
        // Conditional based on fixed-point comparison
        if (prod > 0.75r) {
            results[*count] = prod;
            (*count)++;
        }
        
        // Another boundary check
        if (div_result < 0.25r && div_result > 0.0r) {
            results[*count] = div_result;
            (*count)++;
        }
        
        // Update for next iteration
        min_val = min_val + 0.1r;
        if (min_val > 0.9r) min_val = 0.0r;
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat signed long _Accum *accum_results, int *acc_count) {
    // Boundary values for signed long _Accum
    _Sat signed long _Accum pos_max = 9223372036854775.807k;  // Near max
    _Sat signed long _Accum neg_max = -9223372036854775.807k; // Near min
    _Sat signed long _Accum zero = 0.0k;
    
    // Mixed precision operations
    for (int i = -5; i <= 5; i++) {
        _Sat signed long _Accum val = i * 1000.0k;
        
        // Operations that saturate at boundaries
        _Sat signed long _Accum sum = val + pos_max;
        _Sat signed long _Accum diff = val - neg_max;
        
        // Multiplication that could overflow
        _Sat signed long _Accum prod = val * 1000000.0k;
        
        // Use builtins for overflow detection
        int overflow = 0;
        _Sat signed long _Accum builtin_result = __builtin_add_overflow(val, pos_max, &prod) ? pos_max : prod;
        
        // Complex conditional with boundary comparisons
        if (sum > 0.0k && sum < pos_max) {
            accum_results[*acc_count] = sum;
            (*acc_count)++;
        }
        
        // This should trigger the a_high.sgt(max_r) comparison logic
        if (diff < 0.0k || diff == neg_max) {
            accum_results[*acc_count] = diff;
            (*acc_count)++;
        }
        
        // Force evaluation of max_s.zext(i_f_bits) type logic
        if (prod == pos_max || prod == neg_max) {
            accum_results[*acc_count] = builtin_result;
            (*acc_count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision(_Sat unsigned short _Fract *fract_arr, 
                          _Sat signed long _Accum *accum_arr,
                          int size) {
    // Mixed type operations
    for (int i = 0; i < size && i < 10; i++) {
        // Convert between precisions (forces i_f_bits calculations)
        _Sat signed long _Accum accum_val = (_Sat signed long _Accum)fract_arr[i] * 10000.0k;
        
        // Operations that require range extension
        _Sat signed long _Accum scaled = accum_val << 2;  // Similar to alshift in uncovered code
        
        // Division with different precisions
        if (accum_val > 0.0k) {
            _Sat unsigned short _Fract reciprocal = 1.0r / fract_arr[i];
            
            // Conditional that depends on boundary values
            if (reciprocal > 0.95r && reciprocal < 1.0r) {
                accum_arr[i] = scaled;
            } else if (reciprocal == 1.0r || reciprocal == 0.0r) {
                accum_arr[i] = accum_val;
            }
        }
        
        // Force min_s.alshift() type logic
        _Sat signed long _Accum shifted = accum_val * (1 << 16);  // Simulate shift
        if (shifted < accum_val) {  // Check for overflow in "shift"
            accum_arr[i] = -accum_val;
        }
    }
}

// Struct containing fixed-point values
struct FixedPointStruct {
    _Sat signed short _Accum acc;
    unsigned _Fract frac;
    _Sat signed long _Accum large_acc;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int len) {
    for (int i = 0; i < len; i++) {
        // Operations on struct members
        arr[i].large_acc = (_Sat signed long _Accum)arr[i].acc * 1000.0k;
        arr[i].frac = (unsigned _Fract)(arr[i].large_acc / 10000.0k);
        
        // Boundary checks on struct members
        if (arr[i].large_acc > 500000.0k && arr[i].large_acc < 1000000.0k) {
            arr[i].acc = (_Sat signed short _Accum)(arr[i].frac * 100.0k);
        }
        
        // Force a_high == max_r && a_low.ugt(max_s) type comparison
        if (arr[i].acc == 0.0k && arr[i].frac > 0.5r) {
            arr[i].large_acc = -arr[i].large_acc;
        }
    }
}

// Array-based operations
__attribute__((optimize("O3")))
_Sat signed long _Accum process_fixed_array(_Sat signed long _Accum *arr, int size) {
    _Sat signed long _Accum result = 0.0k;
    _Sat signed long _Accum max_val = 9223372036854775.807k;
    _Sat signed long _Accum min_val = -9223372036854775.807k;
    
    for (int i = 0; i < size; i++) {
        // Operations that approach boundaries
        result = result + arr[i];
        
        // Force saturation at boundaries
        if (result > max_val) {
            result = max_val;
        } else if (result < min_val) {
            result = min_val;
        }
        
        // Multiplication that could trigger range analysis
        _Sat signed long _Accum prod = arr[i] * 2.0k;
        
        // Comparison similar to uncovered code's condition
        if (prod > max_val || (prod == max_val && arr[i] > 0.0k)) {
            result = result - arr[i];
        }
    }
    
    return result;
}

int main() {
    // Initialize arrays with boundary values
    unsigned short _Fract fract_results[20] = {0.0r};
    _Sat signed long _Accum accum_results[20] = {0.0k};
    int fract_count = 0;
    int accum_count = 0;
    
    // Test 1: Short fract range analysis
    test_short_fract_range(fract_results, &fract_count);
    
    // Test 2: Saturated accum range analysis  
    test_sat_accum_range(accum_results, &accum_count);
    
    // Test 3: Mixed precision operations
    _Sat unsigned short _Fract mixed_fract[10] = {
        0.0r, 0.25r, 0.5r, 0.75r, 0.9999r,
        0.1r, 0.33r, 0.66r, 0.9r, 0.125r
    };
    _Sat signed long _Accum mixed_accum[10] = {0.0k};
    test_mixed_precision(mixed_fract, mixed_accum, 10);
    
    // Test 4: Struct operations
    struct FixedPointStruct struct_arr[5];
    for (int i = 0; i < 5; i++) {
        struct_arr[i].acc = (_Sat signed short _Accum)(i * 0.2k);
        struct_arr[i].frac = (unsigned _Fract)(i * 0.1r);
        struct_arr[i].large_acc = (_Sat signed long _Accum)(i * 1000.0k);
    }
    test_struct_operations(struct_arr, 5);
    
    // Test 5: Array processing
    _Sat signed long _Accum test_array[8] = {
        100.0k, 1000.0k, 10000.0k, 100000.0k,
        -100.0k, -1000.0k, -10000.0k, -100000.0k
    };
    _Sat signed long _Accum final_result = process_fixed_array(test_array, 8);
    
    // Aggregate results to prevent dead code elimination
    volatile _Sat signed long _Accum volatile_sum = 0.0k;
    for (int i = 0; i < fract_count; i++) {
        volatile_sum += (_Sat signed long _Accum)fract_results[i];
    }
    for (int i = 0; i < accum_count; i++) {
        volatile_sum += accum_results[i];
    }
    for (int i = 0; i < 10; i++) {
        volatile_sum += mixed_accum[i];
    }
    for (int i = 0; i < 5; i++) {
        volatile_sum += struct_arr[i].large_acc;
    }
    volatile_sum += final_result;
    
    printf("Result: %Lf\n", (long double)volatile_sum);
    
    return 0;
}
