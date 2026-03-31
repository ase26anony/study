#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);          // 8.0
    results[idx++] = sqrt(25.0);             // 5.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = log(exp(5.0));          // 5.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, n);           // 32.0
    results[idx++] = exp2(n);                // 32.0
    
    // 3. Mixed integer and floating-point contexts
    int int_var = pow(4.0, 2.0);             // 16
    int_results[0] = sqrt(64.0);             // 8
    int_results[1] = __builtin_pow(2.0, 4.0); // 16
    
    // 4. Use in array indexing
    int array[10] = {0,1,2,3,4,5,6,7,8,9};
    int array_idx = sqrt(9.0);               // 3
    int array_val = array[array_idx];        // 3
    
    // 5. Comparisons with integers
    if (pow(2.0, 3.0) == 8) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    // 6. Loop with invariant constants (for conditional constant propagation)
    double loop_result = 0.0;
    for (int i = 0; i < 3; i++) {
        // Arguments are constant within loop
        loop_result += pow(2.0, i);          // 1 + 2 + 4 = 7
    }
    results[idx++] = loop_result;
    
    // 7. Nested calls
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    results[idx++] = exp2(log2(32.0));                     // 32
    
    // 8. Single-argument functions
    results[idx++] = exp2(5.0);              // 32
    results[idx++] = sqrt(81.0);             // 9
    results[idx++] = cbrt(64.0);             // 4
    results[idx++] = __builtin_exp2(6.0);    // 64
    results[idx++] = __builtin_sqrt(49.0);   // 7
    
    // 9. Two-argument functions
    results[idx++] = pow(5.0, 2.0);          // 25
    results[idx++] = __builtin_pow(6.0, 2.0); // 36
    
    // 10. Trigonometric functions with special cases
    results[idx++] = sin(0.0);               // 0
    results[idx++] = cos(0.0);               // 1
    results[idx++] = __builtin_sin(M_PI);    // ~0 (should fold to 0 with -ffast-math)
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    
    int int_sum = 0;
    for (int i = 0; i < 2; i++) {
        int_sum += int_results[i];
    }
    
    // Print checksum
    printf("Checksum: %f\n", sum + int_sum + array_val);
    
    return 0;
}
