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
    results[idx++] = pow(2.0, n);             // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(49.0);                      // 7
    int i3 = exp2(3.0);                       // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in integer array indexing
    double array[10] = {0};
    array[(int)sqrt(16.0)] = 42.0;            // array[4] = 42.0
    results[idx++] = array[4];
    
    // 6. Compare to integer constants
    if (pow(2.0, m) == 8.0) {                 // 2^3 == 8
        results[idx++] = 1.0;
    }
    if (sqrt(81.0) == 9.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0));                     // 32.0
    results[idx++] = cbrt(pow(2.0, 9.0));                  // 8.0
    
    // 8. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // Arguments are loop-invariant constants
        loop_sum += pow(2.0, j) + sqrt((double)(j * j + 4 * j + 4));
    }
    results[idx++] = loop_sum;  // Should fold to constant
    
    // 9. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                 // 0.0
    results[idx++] = cos(0.0);                 // 1.0
    results[idx++] = __builtin_sin(pi);        // ~0.0 (with fast-math)
    results[idx++] = __builtin_cos(0.0);       // 1.0
    
    // 10. One-argument functions
    results[idx++] = exp2(6.0);                // 64.0
    results[idx++] = __builtin_log(1.0);       // 0.0
    results[idx++] = __builtin_log10(100.0);   // 2.0
    
    // 11. Two-argument functions
    results[idx++] = pow(4.0, 0.5);            // 2.0 (sqrt via pow)
    results[idx++] = __builtin_pow(5.0, 0.0);  // 1.0
    
    // 12. Mixed integer/float context with control flow
    double val = pow(2.0, 6.0);                // 64.0
    int int_val = (int)val;
    if (int_val == 64) {
        results[idx++] = 3.0;
    }
    
    // 13. More complex expressions that should fold to integers
    results[idx++] = sqrt(pow(3.0, 4.0));      // sqrt(81) = 9.0
    results[idx++] = log(pow(2.718281828459045, 5.0)); // ln(e^5) = 5.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    for (int k = 0; k < idx; k++) {
        sum += results[k];
    }
    sum += int_results[0];
    
    // Print checksum (cast to int for deterministic output across platforms)
    printf("Checksum: %d\n", (int)sum);
    
    return 0;
}
