#include <stdio.h>
#include <stdint.h>

// Force aggressive optimization on specific functions
__attribute__((optimize("O3")))
void test_short_fract_range(unsigned short _Fract *results, int *count) {
    // Initialize with boundary values
    unsigned short _Fract max_val = 0.9999r;
    unsigned short _Fract min_val = 0.0r;
    unsigned short _Fract mid_val = 0.5r;
    
    // Operations that approach boundaries
    for (int i = 0; i < 10; i++) {
        // Multiplication that can saturate
        unsigned short _Fract product = mid_val * max_val;
        
        // Division that can approach limits
        unsigned short _Fract quotient = max_val / (mid_val + 0.1r);
        
        // Conditional based on range comparisons
        if (product > 0.8r) {
            results[*count] = product;
            (*count)++;
        }
        
        if (quotient < 0.2r || quotient > 0.9r) {
            results[*count] = quotient;
            (*count)++;
        }
        
        // Update mid_val to approach boundaries
        mid_val = mid_val * 1.1r;
        if (mid_val > 0.9r) {
            mid_val = 0.1r;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_range(_Sat signed long _Accum *results, int *count) {
    // Boundary initialization for signed accum
    _Sat signed long _Accum max_pos = 0x7FFFFFFFFFFFFFFFlk;
    _Sat signed long _Accum max_neg = -0x8000000000000000lk;
    _Sat signed long _Accum zero = 0.0lk;
    
    // Mixed precision operations
    for (int i = 0; i < 8; i++) {
        // Operations that will saturate
        _Sat signed long _Accum saturated_add = max_pos + max_pos;
        _Sat signed long _Accum saturated_sub = max_neg - max_pos;
        
        // Multiplication near boundaries
        _Sat signed long _Accum large_product = max_pos * 0.999lk;
        _Sat signed long _Accum small_product = max_neg * 0.999lk;
        
        // Range comparisons that should trigger the uncovered logic
        if (saturated_add > max_pos || saturated_add == max_pos) {
            results[*count] = saturated_add;
            (*count)++;
        }
        
        if (saturated_sub < max_neg || (saturated_sub == max_neg && zero > 0.0lk)) {
            results[*count] = saturated_sub;
            (*count)++;
        }
        
        // Test overflow builtins with fixed-point
        _Sat signed long _Accum overflow_test;
        if (__builtin_mul_overflow(max_pos, 2.0lk, &overflow_test)) {
            results[*count] = overflow_test;
            (*count)++;
        }
        
        // Adjust values for next iteration
        max_pos = max_pos * 0.9lk;
        max_neg = max_neg * 0.9lk;
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_operations(void) {
    // Different fixed-point types
    unsigned short _Fract usf1 = 0.75r;
    signed short _Fract ssf1 = -0.5r;
    _Sat unsigned long _Accum ula1 = 0xFFFFFFFFFFFFFFFlk;
    signed long _Accum sla1 = -0x7FFFFFFFFFFFFFFFlk;
    
    // Mixed precision arithmetic
    for (int i = 0; i < 5; i++) {
        // Convert and combine different types
        signed long _Accum mixed1 = (_Accum)usf1 * sla1;
        _Sat unsigned long _Accum mixed2 = ula1 + (_Accum)ssf1;
        
        // Complex conditional with mixed comparisons
        if ((mixed1 > 0.0lk && usf1 > 0.5r) || 
            (mixed1 < 0.0lk && ssf1 < -0.25r)) {
            volatile _Accum temp = mixed1 * 2.0lk;
            (void)temp;
        }
        
        // Boundary checks with different types
        if (mixed2 == ula1 || mixed2 < (_Accum)usf1) {
            volatile _Accum temp = mixed2 / 2.0lk;
            (void)temp;
        }
        
        // Update values
        usf1 = usf1 * 1.1r;
        ssf1 = ssf1 * 0.9r;
        ula1 = ula1 * 0.95lk;
        sla1 = sla1 * 1.05lk;
    }
}

// Struct containing fixed-point values
struct FixedPointStruct {
    _Sat signed short _Accum x;
    unsigned _Fract y;
    signed long _Accum z;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int size) {
    // Initialize array with boundary values
    for (int i = 0; i < size; i++) {
        arr[i].x = (i % 2 == 0) ? 0x7FFFlk : -0x8000lk;
        arr[i].y = (i % 3 == 0) ? 0.9999r : 0.0001r;
        arr[i].z = (i % 4 == 0) ? 0x7FFFFFFFFFFFFFFFlk : -0x8000000000000000lk;
    }
    
    // Perform operations on struct elements
    for (int i = 0; i < size - 1; i++) {
        // Cross-structure operations
        _Sat signed short _Accum sum_x = arr[i].x + arr[i+1].x;
        unsigned _Fract product_y = arr[i].y * arr[i+1].y;
        signed long _Accum diff_z = arr[i].z - arr[i+1].z;
        
        // Complex conditionals that should trigger range analysis
        if (sum_x > arr[i].x || (sum_x == arr[i].x && product_y > arr[i+1].y)) {
            volatile _Accum temp = (_Accum)sum_x * (_Accum)product_y;
            (void)temp;
        }
        
        if (diff_z < arr[i].z || (diff_z == arr[i].z && diff_z > 0.0lk)) {
            volatile _Accum temp = diff_z / 2.0lk;
            (void)temp;
        }
        
        // Use overflow builtins
        signed long _Accum overflow_result;
        if (__builtin_add_overflow(arr[i].z, arr[i+1].z, &overflow_result)) {
            volatile _Accum temp = overflow_result;
            (void)temp;
        }
    }
}

int main(void) {
    // Arrays to store results (prevent dead code elimination)
    unsigned short _Fract fract_results[20];
    _Sat signed long _Accum accum_results[20];
    struct FixedPointStruct struct_array[10];
    
    int fract_count = 0;
    int accum_count = 0;
    
    // Execute test functions
    test_short_fract_range(fract_results, &fract_count);
    test_sat_accum_range(accum_results, &accum_count);
    test_mixed_precision_operations();
    test_struct_operations(struct_array, 10);
    
    // Aggregate results to prevent optimization removal
    volatile unsigned short _Fract fract_sum = 0.0r;
    volatile _Sat signed long _Accum accum_sum = 0.0lk;
    
    for (int i = 0; i < fract_count; i++) {
        fract_sum += fract_results[i];
    }
    
    for (int i = 0; i < accum_count; i++) {
        accum_sum += accum_results[i];
    }
    
    // Print to prevent dead code elimination
    printf("Fract sum: %u\n", (unsigned)(fract_sum * 65536));
    printf("Accum sum: %lld\n", (long long)(accum_sum * 1000000));
    
    return 0;
}
