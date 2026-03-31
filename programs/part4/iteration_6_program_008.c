#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // Constant integer values for symbolic propagation
    const int n = 5;
    const int m = 3;
    const double base = 2.0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log(256.0) / log(2.0);   // 8.0 (log2 via change of base)
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments with constant relationships
    results[idx++] = pow(base, n);            // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign directly to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(49.0);                      // 7
    int i3 = exp2(3.0);                       // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in array indexing context
    double array[10] = {0};
    array[(int)sqrt(16.0)] = 3.14;            // array[4] = 3.14
    array[(int)pow(2.0, 2.0)] = 2.71;         // array[4] = 2.71 (overwrites)
    results[idx++] = array[4];
    
    // 6. Compare to integer constants
    if (pow(2.0, m) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and complex expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0));                     // 32.0
    results[idx++] = cbrt(pow(3.0, 3.0));                  // 3.0
    
    // 8. Functions with single argument (testing call_expr_nargs > 0 branch)
    results[idx++] = exp2(6.0);                            // 64.0
    results[idx++] = sqrt(81.0);                           // 9.0
    results[idx++] = cbrt(64.0);                           // 4.0
    results[idx++] = log(1.0);                             // 0.0
    
    // 9. Functions with two arguments (testing call_expr_nargs > 1 branch)
    results[idx++] = pow(4.0, 1.5);                        // 8.0 (4^(3/2))
    results[idx++] = fmod(15.0, 4.0);                      // 3.0
    
    // 10. Loop with invariant constants (for conditional constant propagation)
    double loop_result = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is constant within each iteration after unrolling
        loop_result += pow(2.0, j);  // 1 + 2 + 4 = 7
    }
    results[idx++] = loop_result;
    
    // 11. Mixed integer/float context with atan2 (two arguments)
    results[idx++] = atan2(0.0, 1.0);  // 0.0
    
    // 12. Trigonometric functions that can produce integer results
    results[idx++] = sin(0.0);         // 0.0
    results[idx++] = cos(0.0);         // 1.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    for (int j = 0; j < idx; j++) {
        sum += results[j];
    }
    
    // Use integer results too
    int int_sum = int_results[0];
    for (int j = 1; j < 10 && j < ARRAY_SIZE; j++) {
        int_sum += int_results[j];
    }
    
    // Print checksum (makes computations observable)
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum);
    
    // Additional integer context usage
    printf("Array[4] = %f\n", array[4]);
    printf("Direct integer assignments: %d, %d, %d\n", i1, i2, i3);
    
    return 0;
}
