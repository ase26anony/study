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
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log2(8.0);               // 3.0
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments with constant relationships
    results[idx++] = pow(2.0, n);             // 32.0 (n=5)
    results[idx++] = sqrt((double)(n * n));   // 5.0
    
    // 4. Assign to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, m);                     // 8
    int i2 = sqrt(81.0);                      // 9
    int i3 = exp2(6.0);                       // 64
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in array indexing
    double array[10] = {0};
    array[(int)sqrt(16.0)] = 42.0;            // array[4] = 42.0
    results[idx++] = array[4];
    
    // 6. Compare to integer constants
    if (pow(2.0, 4.0) == 16.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(64.0), log2(8.0));      // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0) / 2.0);          // sqrt(32) ≈ 5.657
    results[idx++] = cbrt(pow(3.0, 3.0));             // 3.0
    
    // 8. Loop with invariant constants for conditional constant propagation
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is constant within each iteration after unrolling
        loop_sum += pow(2.0, j);  // 1 + 2 + 4 = 7
    }
    results[idx++] = loop_sum;
    
    // 9. One-argument functions
    results[idx++] = exp2(6.0);                       // 64.0
    results[idx++] = sqrt(144.0);                     // 12.0
    results[idx++] = cbrt(125.0);                     // 5.0
    results[idx++] = log(exp(3.0));                   // 3.0
    
    // 10. Two-argument functions
    results[idx++] = pow(4.0, 1.5);                   // 8.0
    results[idx++] = atan2(0.0, 1.0);                 // 0.0
    
    // 11. Mixed integer/float contexts with more builtins
    int i4 = __builtin_pow(5.0, 2.0);                 // 25
    int i5 = __builtin_sqrt(49.0);                    // 7
    int_results[1] = i4 * i5;
    
    // 12. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                        // 0.0
    results[idx++] = cos(0.0);                        // 1.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    int int_sum = 0;
    for (int j = 0; j < idx; j++) {
        sum += results[j];
    }
    for (int j = 0; j < 2; j++) {
        int_sum += int_results[j];
    }
    
    // Use results in output
    printf("Checksum: double_sum = %f, int_sum = %d\n", sum, int_sum);
    printf("Array[4] = %f\n", array[4]);
    
    return 0;
}
