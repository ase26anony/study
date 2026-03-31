#include <math.h>
#include <stdio.h>

int main() {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    const int iterations = 3;
    
    // Results storage
    double fp_results[20] = {0};
    int int_results[20] = {0};
    int idx = 0;
    
    // 2. Direct constant argument calls (will be folded at compile-time)
    // Two-argument functions (pow, atan2)
    fp_results[idx++] = pow(2.0, 3.0);           // 8.0 - integer result
    fp_results[idx++] = __builtin_pow(3.0, 2.0); // 9.0 - integer result
    fp_results[idx++] = pow(4.0, 0.5);           // 2.0 - integer result (sqrt)
    
    // One-argument functions
    fp_results[idx++] = sqrt(25.0);              // 5.0 - integer result
    fp_results[idx++] = __builtin_sqrt(36.0);    // 6.0 - integer result
    fp_results[idx++] = exp2(4.0);               // 16.0 - integer result
    fp_results[idx++] = __builtin_exp2(5.0);     // 32.0 - integer result
    fp_results[idx++] = cbrt(27.0);              // 3.0 - integer result
    fp_results[idx++] = log2(8.0);               // 3.0 - integer result
    fp_results[idx++] = __builtin_log2(16.0);    // 4.0 - integer result
    
    // 3. Symbolic arguments with constant relationships
    // Using const int n = 5
    fp_results[idx++] = pow(base, n);            // 2^5 = 32.0 - integer result
    fp_results[idx++] = sqrt(n * n);             // sqrt(25) = 5.0 - integer result
    
    // 4. Mixed integer and floating-point contexts
    // Assign to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, 3.0);                      // Should fold to 8
    int i2 = sqrt(49.0);                         // Should fold to 7
    int i3 = __builtin_exp2(3.0);                // Should fold to 8
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    // 5. Use in array indexing
    double test_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(16.0);             // Should fold to 4
    double val_from_array = test_array[array_idx];
    fp_results[idx++] = val_from_array;
    
    // 6. Compare to integer (triggers folding in conditional)
    if (pow(2.0, 3.0) == 8) {
        fp_results[idx++] = 100.0;  // Marker value
    }
    
    if (__builtin_sqrt(81.0) == 9) {
        fp_results[idx++] = 200.0;  // Marker value
    }
    
    // 7. Nested calls and combined expressions
    fp_results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  // 4^3 = 64
    fp_results[idx++] = exp2(log2(32.0) / 2.0);                // sqrt(32) ≈ 5.657
    
    // 8. Conditional constant propagation in loops
    double loop_result = 1.0;
    for (int j = 0; j < iterations; j++) {
        // Loop-invariant constant argument
        loop_result *= pow(2.0, 1.0);  // Always 2.0, but compiler may not know
    }
    fp_results[idx++] = loop_result;
    
    // Another loop with constant iteration count
    double power_result = 1.0;
    const int fixed_loop = 4;
    for (int k = 0; k < fixed_loop; k++) {
        power_result = pow(power_result, 2.0);
    }
    fp_results[idx++] = power_result;
    
    // 9. More built-in variants
    fp_results[idx++] = __builtin_pow(10.0, 2.0);   // 100.0
    fp_results[idx++] = __builtin_cbrt(64.0);       // 4.0
    fp_results[idx++] = __builtin_log(exp(1.0));    // 1.0
    
    // 10. Trigonometric functions with special cases
    fp_results[idx++] = sin(0.0);                   // 0.0 - integer result
    fp_results[idx++] = cos(0.0);                   // 1.0 - integer result
    fp_results[idx++] = __builtin_sin(M_PI);        // ~0.0 (should be exactly 0 with -ffast-math)
    
    // 11. Use results in integer context
    int sum_int = 0;
    for (int m = 0; m < 3; m++) {
        sum_int += (int)pow(2.0, m);  // 1 + 2 + 4 = 7
    }
    int_results[3] = sum_int;
    
    // 12. Final checksum calculation
    double checksum_fp = 0.0;
    for (int c = 0; c < idx; c++) {
        checksum_fp += fp_results[c];
    }
    
    int checksum_int = 0;
    for (int c = 0; c < 4; c++) {
        checksum_int += int_results[c];
    }
    
    // Print results to prevent dead code elimination
    printf("Floating-point checksum: %f\n", checksum_fp);
    printf("Integer checksum: %d\n", checksum_int);
    printf("Array index test: %f\n", val_from_array);
    
    return 0;
}
