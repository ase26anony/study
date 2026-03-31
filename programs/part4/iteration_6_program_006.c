#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // Constant integer values for propagation
    const int n = 5;
    const int m = 3;
    const double pi = 3.141592653589793;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = sqrt(16.0);              // 4.0
    results[idx++] = __builtin_sqrt(25.0);    // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = __builtin_cbrt(64.0);    // 4.0
    
    // 2. Symbolic arguments with constant relationships
    results[idx++] = pow(2.0, n);             // 32.0 (n=5)
    results[idx++] = __builtin_pow(m, 3.0);   // 27.0 (m=3)
    
    // 3. Mixed integer and floating-point contexts
    int int_var = pow(2.0, m);                // 8.0 -> 8
    int_results[0] = int_var;
    
    // Use in array indexing
    int array_index = (int)sqrt(36.0);        // 6
    results[array_index] = 99.0;
    
    // Compare to integer
    if (pow(2.0, 4.0) == 16) {
        results[idx++] = 1.0;
    }
    
    // 4. Multiple and nested calls
    results[idx++] = pow(sqrt(81.0), log(8.0) / log(2.0));  // 9^3 = 729.0
    results[idx++] = exp2(log2(256.0) / 2.0);               // sqrt(256) = 16.0
    
    // One-argument functions
    results[idx++] = __builtin_log(exp(1.0));               // 1.0
    results[idx++] = sin(pi / 2.0);                         // 1.0
    results[idx++] = __builtin_cos(0.0);                    // 1.0
    
    // Two-argument functions
    results[idx++] = atan2(0.0, 1.0);                       // 0.0
    
    // 5. Conditional constant propagation in loops
    for (int i = 0; i < 3; i++) {
        // Loop-invariant constant arguments
        results[idx++] = pow(2.0, i);                       // 1.0, 2.0, 4.0
        int_results[i] = (int)__builtin_exp2(i);            // 1, 2, 4
    }
    
    // Complex expression combining multiple calls
    double complex_val = pow(__builtin_sqrt(144.0), 
                           __builtin_log(27.0) / __builtin_log(3.0));  // 12^3 = 1728.0
    results[idx++] = complex_val;
    
    // Use in control flow with integer comparison
    for (int i = 1; i <= 4; i++) {
        if (cbrt(i * i * i) == i) {
            results[idx++] = (double)i;
        }
    }
    
    // Ensure we don't exceed array bounds
    if (idx >= ARRAY_SIZE) idx = ARRAY_SIZE - 1;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    int int_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += results[i];
        int_sum += int_results[i];
    }
    
    // Use results in output
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum);
    printf("Array element at sqrt(36): %f\n", results[6]);
    
    return 0;
}
