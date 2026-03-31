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
    results[idx++] = sqrt(25.0);             // 5.0
    results[idx++] = __builtin_sqrt(16.0);   // 4.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = __builtin_exp2(3.0);    // 8.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = __builtin_cbrt(8.0);    // 2.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, (double)n);   // 32.0
    results[idx++] = __builtin_pow(10.0, (double)(n-3)); // 100.0
    
    // 3. Mixed integer and floating-point contexts
    int i1 = pow(4.0, 1.5);                  // 8.0 -> 8
    int i2 = __builtin_sqrt(81.0);           // 9.0 -> 9
    int_results[0] = i1;
    int_results[1] = i2;
    
    // 4. Use in integer array indexing
    double arr1[10] = {0};
    arr1[(int)sqrt(36.0)] = 3.14;            // arr1[6]
    arr1[(int)__builtin_pow(2.0, 2.0)] = 2.71; // arr1[4]
    
    // 5. Compare to integer
    if (pow(2.0, 4.0) == 16) {
        results[idx++] = 1.0;
    }
    if (__builtin_sqrt(49.0) == 7) {
        results[idx++] = 2.0;
    }
    
    // 6. Nested calls and combined expressions
    results[idx++] = pow(sqrt(64.0), log(8.0) / log(2.0)); // 8^3 = 512
    results[idx++] = __builtin_exp2(__builtin_log2(32.0)); // 32
    
    // 7. Functions with one argument
    results[idx++] = exp2(5.0);              // 32.0
    results[idx++] = __builtin_log(exp(1.0)); // 1.0
    results[idx++] = sin(0.0);               // 0.0
    results[idx++] = __builtin_cos(0.0);     // 1.0
    
    // 8. Functions with two arguments
    results[idx++] = pow(5.0, 2.0);          // 25.0
    results[idx++] = __builtin_atan2(0.0, 1.0); // 0.0
    
    // 9. Loop with invariant constants (triggers constant propagation)
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; ++iter) {
        // These arguments are loop-invariant and should fold
        loop_sum += pow(2.0, (double)iter) + sqrt((double)(iter * iter * 4));
    }
    results[idx++] = loop_sum;
    
    // 10. More complex integer-valued real calls
    const int m = 3;
    results[idx++] = pow(2.0, (double)(m + 2)); // 32.0
    results[idx++] = sqrt((double)(m * m * 9)); // 9.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int j = 0; j < idx; ++j) {
        checksum += results[j];
    }
    for (int j = 0; j < 2; ++j) {
        checksum += int_results[j];
    }
    
    // Use checksum in output
    printf("Checksum: %f\n", checksum);
    printf("Integer results: %d, %d\n", int_results[0], int_results[1]);
    
    return 0;
}
