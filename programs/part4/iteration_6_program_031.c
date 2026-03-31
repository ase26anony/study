#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);          // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = sqrt(16.0);             // 4.0
    results[idx++] = __builtin_sqrt(25.0);   // 5.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = __builtin_exp2(5.0);    // 32.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = __builtin_cbrt(64.0);   // 4.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 3;
    const double base = 2.0;
    results[idx++] = pow(base, (double)n);   // 8.0
    results[idx++] = __builtin_pow(base, n); // 8.0
    
    // 3. Mixed integer and floating-point contexts
    int int_var;
    int_var = pow(4.0, 1.5);                 // 8.0 -> assigned to int
    int_results[0] = int_var;
    
    int_var = __builtin_sqrt(36.0);          // 6.0 -> assigned to int
    int_results[1] = int_var;
    
    // 4. Use in array indexing
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(9.0);          // 3
    results[idx++] = array[array_idx];
    
    array_idx = (int)__builtin_pow(2.0, 2.0); // 4
    results[idx++] = array[array_idx];
    
    // 5. Compare to integer
    if (pow(2.0, 3.0) == 8) {
        results[idx++] = 1.0;
    }
    
    if (__builtin_sqrt(49.0) == 7) {
        results[idx++] = 2.0;
    }
    
    // 6. Nested calls
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    results[idx++] = __builtin_pow(__builtin_sqrt(81.0), 2.0); // 9^2 = 81
    
    // 7. Loop with invariant constants
    double sum = 0.0;
    for (int i = 0; i < 3; i++) {
        // Loop-invariant constant arguments
        sum += pow(2.0, (double)i);  // 1 + 2 + 4 = 7
    }
    results[idx++] = sum;
    
    // Another loop with __builtin_ version
    double prod = 1.0;
    for (int i = 1; i <= 3; i++) {
        prod *= __builtin_sqrt((double)(i * i)); // sqrt(1)*sqrt(4)*sqrt(9) = 1*2*3 = 6
    }
    results[idx++] = prod;
    
    // 8. More single-argument functions
    results[idx++] = log(exp(1.0));          // 1.0
    results[idx++] = __builtin_log(__builtin_exp(2.0)); // 2.0
    results[idx++] = sin(0.0);               // 0.0
    results[idx++] = __builtin_cos(0.0);     // 1.0
    
    // 9. Two-argument functions (to trigger arg1 branch)
    results[idx++] = atan2(0.0, 1.0);        // 0.0
    results[idx++] = __builtin_atan2(1.0, 0.0); // π/2, not integer
    
    // 10. Complex expression mixing everything
    double complex_val = pow(2.0, log(8.0) / log(2.0)) + sqrt(16.0) * exp2(1.0);
    results[idx++] = complex_val;  // 8 + 4*2 = 16
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    // Also checksum integer results
    int int_checksum = 0;
    for (int i = 0; i < 2; i++) {
        int_checksum += int_results[i];
    }
    
    printf("Floating-point checksum: %f\n", checksum);
    printf("Integer checksum: %d\n", int_checksum);
    
    // Use results in control flow to prevent optimization
    if (checksum > 100.0) {
        printf("Large checksum detected\n");
    }
    
    return 0;
}
