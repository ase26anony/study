#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // Constant integer values for propagation
    const int n = 5;
    const int m = 3;
    const double pi = 3.141592653589793;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(16.0);              // 4.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log2(8.0);               // 3.0
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(25.0);    // 5.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments that are provably constant
    results[idx++] = pow(2.0, n);             // 32.0 (n=5)
    results[idx++] = sqrt((double)(n * n));   // 5.0
    results[idx++] = exp2(m);                 // 8.0 (m=3)
    
    // 4. Mixed integer context - direct assignment to integer
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(36.0);                      // 6
    int i3 = __builtin_exp2(3.0);             // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Nested calls with integer results
    results[idx++] = pow(sqrt(64.0), log2(8.0) / log2(2.0)); // 64.0
    results[idx++] = exp2(log2(32.0));                       // 32.0
    results[idx++] = sqrt(pow(6.0, 2.0));                    // 6.0
    
    // 6. Conditional constant propagation in loops
    for (int iter = 0; iter < 3; iter++) {
        // Loop-invariant constant arguments
        results[idx++] = pow(2.0, iter + 1);  // 2.0, 4.0, 8.0
        results[idx++] = sqrt((double)((iter + 1) * (iter + 1) * 4)); // 2.0, 4.0, 6.0
    }
    
    // 7. Single-argument functions
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    results[idx++] = __builtin_log(1.0);      // 0.0
    results[idx++] = __builtin_exp(0.0);      // 1.0
    
    // 8. Use in array indexing context
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)pow(2.0, 2.0);       // 4
    int_results[1] = array[array_idx];
    
    // 9. Comparisons with integer constants
    if (pow(3.0, 2.0) == 9) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    // 10. More complex expressions with integer-valued results
    results[idx++] = pow(2.0, log2(8.0));     // 8.0
    results[idx++] = cbrt(pow(3.0, 3.0));     // 3.0
    results[idx++] = exp2(log2(16.0) / 2.0);  // 4.0
    
    // 11. Two-argument atan2 with special cases
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    
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
    
    // Use results in control flow
    if ((int)sum % 2 == 0) {
        printf("Even checksum: %.2f\n", sum);
    } else {
        printf("Odd checksum: %.2f\n", sum);
    }
    
    printf("Integer sum: %d\n", int_sum);
    printf("Final array index: %d\n", array_idx);
    
    return 0;
}
