#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    // Initialize results array
    double results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);          // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = sqrt(25.0);             // 5.0
    results[idx++] = __builtin_sqrt(36.0);   // 6.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = __builtin_exp2(5.0);    // 32.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = __builtin_cbrt(64.0);   // 4.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, n);           // 32.0
    results[idx++] = __builtin_pow(base, (double)n); // 32.0
    
    // 3. Mixed integer and floating-point contexts
    int int_result;
    int_result = (int)pow(4.0, 1.5);         // 8
    results[idx++] = int_result;
    
    int_result = (int)__builtin_sqrt(81.0);  // 9
    results[idx++] = int_result;
    
    // 4. Use in array indexing (triggers integer-valued check)
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(64.0);         // 8
    results[idx++] = array[array_idx];
    
    array_idx = (int)__builtin_pow(2.0, 3.0); // 8
    results[idx++] = array[array_idx];
    
    // 5. Comparisons with integer constants
    double cmp_val = pow(3.0, 3.0);          // 27.0
    if (cmp_val == 27.0) {
        results[idx++] = 1.0;
    } else {
        results[idx++] = 0.0;
    }
    
    cmp_val = __builtin_exp2(6.0);           // 64.0
    if (cmp_val == 64.0) {
        results[idx++] = 2.0;
    } else {
        results[idx++] = 0.0;
    }
    
    // 6. Nested calls and complex expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    results[idx++] = __builtin_pow(__builtin_sqrt(9.0), 3.0); // 3^3 = 27
    
    // 7. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        // Arguments are constant within loop
        loop_sum += pow(2.0, i) + sqrt((double)(i * i * 4));
    }
    results[idx++] = loop_sum;
    
    // 8. One-argument functions
    results[idx++] = exp2(6.0);              // 64.0
    results[idx++] = __builtin_log(exp(1.0)); // 1.0
    results[idx++] = __builtin_log2(256.0);  // 8.0
    results[idx++] = __builtin_log10(100.0); // 2.0
    
    // 9. Trigonometric functions with special cases
    results[idx++] = sin(0.0);               // 0.0
    results[idx++] = __builtin_cos(0.0);     // 1.0
    results[idx++] = __builtin_tan(0.0);     // 0.0
    
    // 10. Two-argument functions
    results[idx++] = atan2(0.0, 1.0);        // 0.0
    results[idx++] = __builtin_fmod(10.0, 3.0); // 1.0
    
    // Calculate checksum
    double checksum = 0.0;
    for (int i = 0; i < idx && i < ARRAY_SIZE; i++) {
        checksum += results[i];
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %f\n", checksum);
    printf("Number of computations: %d\n", idx);
    
    return 0;
}
