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
        unsigned short _Fract val = zero;
        
        // Approach maximum from below
        for (int j = 0; j < i; j++) {
            val = val + 0.125r;
        }
        
        // Boundary comparison - should trigger range checks
        if (val > max_val) {
            results[*count] = max_val;
            (*count)++;
        } else if (val == max_val) {
            results[*count] = val;
            (*count)++;
        }
        
        // Force compiler to track both high and low parts
        if (val < min_val && val != zero) {
            results[*count] = min_val;
            (*count)++;
        }
    }
}

__attribute__((optimize("O3")))
void test_sat_accum_boundaries(_Sat unsigned long _Accum *accums, int size) {
    _Sat unsigned long _Accum max_sat = 0x7FFFFFFFFFFFFFFFk; // Near max
    _Sat unsigned long _Accum min_sat = 0.0000000000000001k; // Near min
    _Sat unsigned long _Accum mid = 0.5k;
    
    // Operations that approach boundaries
    for (int i = 0; i < size; i++) {
        _Sat unsigned long _Accum val = mid;
        
        // Multiplication that could overflow/saturate
        for (int j = 0; j < i; j++) {
            val = val * 1.5k;
            
            // Boundary comparisons forcing range analysis
            if (val > max_sat) {
                accums[i] = max_sat;
                break;
            }
            if (val < min_sat && val > 0.0k) {
                accums[i] = min_sat;
                break;
            }
        }
        
        // Division approaching zero
        if (i > 0) {
            _Sat unsigned long _Accum div_result = val / (i * 2.0k);
            if (div_result < min_sat && div_result > 0.0k) {
                accums[i] = div_result;
            }
        }
    }
}

__attribute__((optimize("O3")))
void test_mixed_precision_ops(void) {
    // Different fixed-point types
    short _Fract sf1 = 0.5r;
    short _Fract sf2 = -0.25r;
    _Sat unsigned long _Accum ula1 = 100.0k;
    _Sat unsigned long _Accum ula2 = 200.0k;
    
    // Mixed operations with overflow checks
    short _Fract sf_result;
    int overflow;
    
    // Multiplication with overflow detection
    overflow = __builtin_mul_overflow((int)(sf1 * 1000), 
                                      (int)(sf2 * 1000), 
                                      (int*)&sf_result);
    
    // Addition with overflow detection
    _Sat unsigned long _Accum accum_result;
    overflow |= __builtin_add_overflow((long)(ula1), 
                                       (long)(ula2), 
                                       (long*)&accum_result);
    
    // Force range comparisons
    if (sf_result > 0.9r || sf_result < -0.9r) {
        volatile short _Fract temp = sf_result;
        (void)temp;
    }
    
    if (accum_result > 250.0k || accum_result < 50.0k) {
        volatile _Sat unsigned long _Accum temp = accum_result;
        (void)temp;
    }
}

// Struct containing fixed-point values
struct FixedPointStruct {
    unsigned short _Fract fract_member;
    _Sat unsigned long _Accum accum_member;
    short _Fract signed_fract;
};

__attribute__((optimize("O3")))
void test_struct_operations(struct FixedPointStruct *arr, int len) {
    // Initialize with boundary values
    for (int i = 0; i < len; i++) {
        arr[i].fract_member = (i % 2) ? 0.9999r : 0.0001r;
        arr[i].accum_member = (i % 3) ? 0x7FFFFFFFFFFFFFFFk : 0.0000000000000001k;
        arr[i].signed_fract = (i % 4) ? 0.5r : -0.5r;
    }
    
    // Operations on struct members
    for (int i = 0; i < len - 1; i++) {
        // Cross-struct comparisons forcing range analysis
        if (arr[i].fract_member > arr[i + 1].fract_member) {
            // Swap or modify based on comparison
            unsigned short _Fract temp = arr[i].fract_member;
            arr[i].fract_member = arr[i + 1].fract_member;
            arr[i + 1].fract_member = temp;
        }
        
        // Accumulator boundary checks
        if (arr[i].accum_member > 0x7FFFFFFFFFFFFFFFk ||
            (arr[i].accum_member == 0x7FFFFFFFFFFFFFFFk && 
             arr[i].fract_member > 0.5r)) {
            arr[i].accum_member = 0x7FFFFFFFFFFFFFFFk;
        }
        
        // Signed fraction comparisons
        if (arr[i].signed_fract < -0.9r || arr[i].signed_fract > 0.9r) {
            arr[i].signed_fract = (arr[i].signed_fract > 0) ? 0.9r : -0.9r;
        }
    }
}

__attribute__((optimize("O3")))
void test_array_boundary_checks(void) {
    // Array of different fixed-point types
    unsigned short _Fract fract_array[16];
    _Sat unsigned long _Accum accum_array[16];
    
    // Initialize with pattern approaching boundaries
    for (int i = 0; i < 16; i++) {
        fract_array[i] = (i / 16.0r);
        accum_array[i] = (i * 0x100000000000000k);
    }
    
    // Operations that force range comparisons
    for (int i = 0; i < 15; i++) {
        // Multiplication approaching limits
        fract_array[i] = fract_array[i] * fract_array[i + 1];
        accum_array[i] = accum_array[i] + accum_array[i + 1];
        
        // Explicit boundary checks
        if (fract_array[i] > 0.99r) {
            fract_array[i] = 0.99r;
        }
        if (accum_array[i] > 0x7FFFFFFFFFFFFFFFk) {
            accum_array[i] = 0x7FFFFFFFFFFFFFFFk;
        }
        if (accum_array[i] < 0.0000000000000001k && accum_array[i] > 0.0k) {
            accum_array[i] = 0.0000000000000001k;
        }
    }
}

int main(void) {
    // Prevent dead code elimination
    volatile int result_counter = 0;
    unsigned short _Fract results[32];
    
    // Test 1: Short fract range analysis
    test_short_fract_range(results, &result_counter);
    
    // Test 2: Saturated accum boundaries
    _Sat unsigned long _Accum accums[8];
    test_sat_accum_boundaries(accums, 8);
    
    // Test 3: Mixed precision operations
    test_mixed_precision_ops();
    
    // Test 4: Struct operations
    struct FixedPointStruct struct_arr[8];
    test_struct_operations(struct_arr, 8);
    
    // Test 5: Array boundary checks
    test_array_boundary_checks();
    
    // Aggregate results to prevent optimization
    unsigned short _Fract final_result = 0.0r;
    for (int i = 0; i < result_counter && i < 32; i++) {
        final_result = final_result + results[i];
    }
    
    // Add accumulator results
    _Sat unsigned long _Accum accum_sum = 0.0k;
    for (int i = 0; i < 8; i++) {
        accum_sum = accum_sum + accums[i];
    }
    
    // Print to prevent elimination
    printf("Final fract result: %d\n", (int)(final_result * 1000));
    printf("Accum sum: %ld\n", (long)(accum_sum));
    
    return 0;
}
