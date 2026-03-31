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
    results[idx++] = pow(2.0, (double)n);     // 32.0
    results[idx++] = sqrt((double)(n * n));   // 5.0
    results[idx++] = exp2((double)m);         // 8.0
    
    // 4. Mixed integer context - direct assignment to integer
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(36.0);                      // 6
    int i3 = __builtin_exp2(3.0);             // 8
    
    // Store in integer array
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    // 5. Nested calls and combined expressions
    results[idx++] = pow(sqrt(64.0), log2(8.0));          // 4^3 = 64
    results[idx++] = exp2(log2(32.0) / 2.0);              // sqrt(32) ≈ 5.656..., not integer
    results[idx++] = cbrt(pow(2.0, 9.0));                 // 512^(1/3) = 8
    
    // 6. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                           // 0
    results[idx++] = cos(0.0);                           // 1
    results[idx++] = __builtin_sin(pi);                  // ~0
    
    // 7. Loop with invariant constants - should still fold
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        // These arguments are loop-invariant, should fold
        loop_sum += pow(2.0, (double)iter) * sqrt((double)(iter * iter + 1));
    }
    results[idx++] = loop_sum;
    
    // 8. Conditional constant propagation
    double cond_result;
    const int flag = 1;
    if (flag > 0) {
        cond_result = pow(3.0, 2.0);  // 9.0
    } else {
        cond_result = sqrt(2.0);      // not taken
    }
    results[idx++] = cond_result;
    
    // 9. Use in array indexing context
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(81.0);  // 9
    int array_val = array[array_idx];
    int_results[3] = array_val;
    
    // 10. Comparison with integer constants
    int cmp_count = 0;
    if (pow(2.0, 3.0) == 8.0) cmp_count++;
    if (sqrt(49.0) == 7) cmp_count++;
    if (__builtin_exp2(4.0) == 16) cmp_count++;
    int_results[4] = cmp_count;
    
    // 11. More complex expressions with integer results
    results[idx++] = pow(4.0, 1.5);                      // 4^(3/2) = 8
    results[idx++] = exp2(log2(64.0) - 1.0);             // 64/2 = 32
    
    // 12. One-argument functions
    results[idx++] = exp(0.0);                           // 1.0
    results[idx++] = log(1.0);                           // 0.0
    results[idx++] = __builtin_log10(100.0);             // 2.0
    
    // 13. Two-argument functions (testing call_expr_nargs > 1 branch)
    results[idx++] = pow(5.0, 2.0);                      // 25.0
    results[idx++] = __builtin_pow(2.0, 5.0);            // 32.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum of all results
    double sum = 0.0;
    for (int j = 0; j < idx; j++) {
        sum += results[j];
    }
    
    int int_sum = 0;
    for (int j = 0; j < 5; j++) {
        int_sum += int_results[j];
    }
    
    // Final mixed integer context
    int final_result = (int)sum + int_sum;
    
    // Use results to prevent dead code elimination
    printf("Checksum: %d\n", final_result);
    printf("Array value at sqrt(81): %d\n", array_val);
    printf("Comparisons passed: %d\n", cmp_count);
    
    return 0;
}
