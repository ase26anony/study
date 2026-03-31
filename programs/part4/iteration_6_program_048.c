#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log2(8.0);               // 3.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, n);            // 32.0
    results[idx++] = exp2(n);                 // 32.0
    
    // 3. Mixed integer and floating-point contexts
    int int_var = pow(4.0, 2.0);              // 16
    int_results[0] = sqrt(49.0);              // 7
    int_results[1] = __builtin_exp2(3.0);     // 8
    
    // Use in array indexing
    int array[10] = {0};
    array[(int)sqrt(64.0)] = 42;              // array[8] = 42
    
    // 4. Nested and combined calls
    results[idx++] = pow(sqrt(16.0), 3.0);    // pow(4.0, 3.0) = 64.0
    results[idx++] = exp2(log2(32.0));        // 32.0
    results[idx++] = sqrt(pow(6.0, 2.0));     // 6.0
    
    // 5. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    results[idx++] = __builtin_sin(M_PI);     // ~0.0 (should fold)
    results[idx++] = __builtin_cos(0.0);      // 1.0
    
    // 6. Loop with invariant constants
    double loop_result = 1.0;
    for (int i = 0; i < 3; i++) {
        // Loop-invariant computation that should fold
        loop_result *= pow(2.0, 2.0);         // Always 4.0
    }
    results[idx++] = loop_result;             // 64.0 (4^3)
    
    // 7. Conditional constant propagation
    double cond_result;
    const int flag = 1;
    if (flag) {
        cond_result = pow(5.0, 2.0);          // 25.0
    } else {
        cond_result = sqrt(100.0);            // 10.0
    }
    results[idx++] = cond_result;
    
    // 8. Comparisons with integers
    int cmp_result = 0;
    if (pow(2.0, 3.0) == 8.0) {
        cmp_result = 1;
    }
    if (sqrt(81.0) == 9) {
        cmp_result += 2;
    }
    int_results[2] = cmp_result;              // Should be 3
    
    // 9. More builtin variants
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    results[idx++] = __builtin_log(exp(1.0)); // 1.0
    results[idx++] = __builtin_cbrt(64.0);    // 4.0
    
    // 10. Two-argument functions
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    results[idx++] = __builtin_pow(2.0, 6.0); // 64.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    int int_sum = 0;
    
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    
    for (int i = 0; i < 3; i++) {
        int_sum += int_results[i];
    }
    
    // Use array element to prevent optimization
    int_sum += array[8];
    
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum);
    printf("Comparison result: %d\n", cmp_result);
    
    return 0;
}
