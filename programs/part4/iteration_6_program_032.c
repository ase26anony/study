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
    results[idx++] = pow(2.0, 3.0);          // 8.0
    results[idx++] = sqrt(25.0);             // 5.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = log2(8.0);              // 3.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0 (builtin version)
    
    // 2. Using const integer variables (symbolic but constant)
    results[idx++] = pow(2.0, n);            // 32.0
    results[idx++] = sqrt((double)(n * n));  // 5.0
    results[idx++] = __builtin_exp2(m);      // 8.0
    
    // 3. Mixed integer and floating-point contexts
    int int_val = pow(2.0, 4.0);             // Direct assignment to int
    int_results[0] = int_val;
    
    // Use as array index
    int array_index = sqrt(16.0);            // 4
    if (array_index >= 0 && array_index < ARRAY_SIZE) {
        results[array_index] = 99.0;
    }
    
    // 4. Comparisons with integer constants
    if (pow(3.0, 2.0) == 9) {
        results[idx++] = 1.0;
    }
    
    if (sqrt(36.0) == 6) {
        results[idx++] = 2.0;
    }
    
    // 5. Nested calls and complex expressions
    results[idx++] = pow(sqrt(64.0), log(8.0) / log(2.0)); // 8^3 = 512
    results[idx++] = exp2(log2(32.0));                     // 32
    
    // 6. Functions with single argument
    results[idx++] = __builtin_sqrt(9.0);                  // 3.0
    results[idx++] = __builtin_cbrt(64.0);                 // 4.0
    results[idx++] = __builtin_log(exp(1.0));              // 1.0
    results[idx++] = __builtin_sin(0.0);                   // 0.0
    results[idx++] = __builtin_cos(0.0);                   // 1.0
    
    // 7. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        // Arguments are constant within loop
        loop_sum += pow(2.0, i) + sqrt((double)(i * i + 1));
    }
    results[idx++] = loop_sum;
    
    // 8. More builtin variants
    results[idx++] = __builtin_pow(4.0, 0.5);              // 2.0 (sqrt via pow)
    results[idx++] = __builtin_powf(2.0f, 5.0f);           // 32.0 (float version)
    
    // 9. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                             // 0.0
    results[idx++] = cos(0.0);                             // 1.0
    results[idx++] = __builtin_sin(pi);                    // ~0.0 (should fold)
    results[idx++] = __builtin_cos(2.0 * pi);              // 1.0
    
    // 10. Logarithm base 10 of power of 10
    results[idx++] = log10(1000.0);                        // 3.0
    
    // Ensure we don't exceed array bounds
    if (idx >= ARRAY_SIZE) idx = ARRAY_SIZE - 1;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i <= idx; i++) {
        checksum += results[i];
        checksum += int_results[i];
    }
    
    // Use checksum in output
    printf("Checksum: %f\n", checksum);
    printf("Integer value from pow: %d\n", int_val);
    printf("Array index used: %d\n", array_index);
    
    return 0;
}
