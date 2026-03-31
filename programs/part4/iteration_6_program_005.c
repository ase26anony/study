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
    results[idx++] = sqrt((double)(n * n));   // 5.0
    results[idx++] = exp2(m);                 // 8.0 (m=3)
    
    // 3. Mixed integer and floating-point contexts
    int i1 = pow(2.0, 3.0);                   // Integer assignment
    int i2 = __builtin_sqrt(81.0);            // Integer assignment
    int_results[0] = i1 + i2;                 // 8 + 9 = 17
    
    // Use in array indexing
    double array[10] = {0};
    array[(int)sqrt(36.0)] = 3.14;            // index 6
    array[(int)__builtin_pow(2.0, 2.0)] = 2.71; // index 4
    
    // 4. Comparisons with integers
    if (pow(3.0, 2.0) == 9) {
        results[idx++] = 1.0;
    }
    if (__builtin_sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    // 5. Nested calls and complex expressions
    results[idx++] = pow(sqrt(64.0), log(8.0) / log(2.0)); // 8^3 = 512
    results[idx++] = exp2(log2(pow(2.0, 4.0)));            // 16.0
    
    // 6. Loop with invariant constants for conditional constant propagation
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        // These arguments are loop-invariant and should fold
        loop_sum += pow(2.0, iter) + sqrt((double)(iter * iter + 1));
    }
    results[idx++] = loop_sum;  // Should be ~10.732
    
    // 7. More one-argument functions
    results[idx++] = log(exp(2.0));           // 2.0
    results[idx++] = __builtin_log(exp(3.0)); // 3.0
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    
    // 8. Two-argument functions (testing call_expr_nargs > 1 branch)
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    results[idx++] = __builtin_atan2(1.0, 0.0); // π/2
    
    // 9. Using results in integer context
    int sum_int = 0;
    for (int j = 0; j < idx && j < ARRAY_SIZE; j++) {
        // Force integer-valued check by casting to int
        int_results[j] = (int)results[j];
        sum_int += int_results[j];
    }
    
    // 10. Prevent dead code elimination with observable output
    printf("Integer sum: %d\n", sum_int);
    printf("Array[6] = %.2f, Array[4] = %.2f\n", array[6], array[4]);
    printf("i1 = %d, i2 = %d\n", i1, i2);
    
    // Checksum to ensure all computations matter
    double checksum = 0.0;
    for (int j = 0; j < idx && j < ARRAY_SIZE; j++) {
        checksum += results[j];
    }
    printf("Checksum: %.6f\n", checksum);
    
    return 0;
}
