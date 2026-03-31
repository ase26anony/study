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
    
    // 1. Direct constant arguments with integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = sqrt(16.0);              // 4.0
    results[idx++] = __builtin_sqrt(25.0);    // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 2. Symbolic arguments with constant relationships
    results[idx++] = pow(2.0, (double)n);     // 32.0
    results[idx++] = sqrt((double)(n * n));   // 5.0
    
    // 3. Assign directly to integer variables (triggers conversion check)
    int i1 = pow(2.0, 3.0);
    int i2 = __builtin_sqrt(81.0);
    int i3 = exp2(3.0);
    int_results[0] = i1 + i2 + i3;
    
    // 4. Use in array indexing
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int index = (int)sqrt(36.0);  // 6
    results[idx++] = array[index];
    
    // 5. Compare to integer constants
    if (pow(3.0, 2.0) == 9.0) {
        results[idx++] = 1.0;
    }
    if (__builtin_sqrt(100.0) == 10.0) {
        results[idx++] = 2.0;
    }
    
    // 6. Nested calls and combined expressions
    results[idx++] = pow(sqrt(64.0), log(8.0) / log(2.0));  // 8^3 = 512
    results[idx++] = exp2(log2(256.0) / 2.0);               // sqrt(256) = 16
    
    // 7. Functions with single arguments
    results[idx++] = cbrt(27.0);            // 3.0
    results[idx++] = __builtin_cbrt(64.0);  // 4.0
    results[idx++] = log(exp(2.0));         // 2.0
    results[idx++] = __builtin_log(exp(3.0)); // 3.0
    
    // 8. Trigonometric functions with special cases
    results[idx++] = sin(0.0);              // 0.0
    results[idx++] = cos(0.0);              // 1.0
    results[idx++] = __builtin_sin(pi);     // ~0.0
    results[idx++] = __builtin_cos(0.0);    // 1.0
    
    // 9. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is not constant, but 2.0 and 3.0 are
        loop_sum += pow(2.0, 3.0);  // Always adds 8.0
    }
    results[idx++] = loop_sum;
    
    // 10. Mixed integer/floating context with two-argument functions
    double x = atan2(0.0, 1.0);  // 0.0
    double y = __builtin_atan2(1.0, 0.0);  // π/2 (not integer)
    results[idx++] = x;
    results[idx++] = y;
    
    // 11. More complex expressions
    results[idx++] = pow(2.0, log(8.0) / log(2.0));  // 8.0
    results[idx++] = sqrt(pow(6.0, 2.0));            // 6.0
    
    // 12. Use results in integer context
    int int_sum = 0;
    for (int k = 0; k < idx && k < ARRAY_SIZE; k++) {
        int_sum += (int)results[k];
    }
    
    // Add the integer results
    int_sum += int_results[0];
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", int_sum);
    
    return 0;
}
