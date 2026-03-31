#include <math.h>
#include <stdio.h>

int main() {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 3;
    const double exp_double = 3.0;
    
    // Results storage
    double results[20] = {0};
    int int_results[10] = {0};
    int idx = 0;
    
    // 2. Direct constant arguments producing integer results
    // Two-argument calls (pow)
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = pow(4.0, 0.5);           // 2.0 (sqrt via pow)
    
    // One-argument calls
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = __builtin_sqrt(16.0);    // 4.0
    results[idx++] = exp2(3.0);               // 8.0
    results[idx++] = __builtin_exp2(4.0);     // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log2(8.0);               // 3.0
    results[idx++] = __builtin_log2(16.0);    // 4.0
    
    // 3. Symbolic arguments with constant relationships
    results[idx++] = pow(base, exp_int);      // 8.0
    results[idx++] = sqrt((double)(n * n));   // 5.0
    results[idx++] = exp2(exp_double);        // 8.0
    
    // 4. Mixed integer context usage
    // Assign to integer variables
    int i1 = pow(2.0, 3.0);                   // Should fold to 8
    int i2 = __builtin_sqrt(16.0);            // Should fold to 4
    int i3 = exp2(3.0);                       // Should fold to 8
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    // 5. Use in array indexing
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(25.0);          // Should fold to 5
    double val_from_array = array[array_idx];
    results[idx++] = val_from_array;
    
    // 6. Compare to integer constants
    int cmp_result = 0;
    if (pow(2.0, 3.0) == 8) {
        cmp_result = 1;
    }
    if (__builtin_sqrt(9.0) == 3) {
        cmp_result += 2;
    }
    int_results[3] = cmp_result;
    
    // 7. Nested and combined calls
    results[idx++] = pow(sqrt(16.0), log2(8.0));          // 4^3 = 64
    results[idx++] = exp2(log2(32.0) / 2.0);              // sqrt(32) via exp2/log2
    results[idx++] = __builtin_pow(__builtin_sqrt(81.0), 2.0); // 9^2 = 81
    
    // 8. Place calls in a loop with fixed iteration count
    double loop_result = 0.0;
    for (int j = 0; j < 3; j++) {
        // Loop-invariant computation that should still fold
        loop_result += pow(2.0, (double)j);  // 1 + 2 + 4 = 7
    }
    results[idx++] = loop_result;
    
    // Another loop with different function
    double loop_result2 = 1.0;
    for (int j = 1; j <= 3; j++) {
        loop_result2 *= sqrt((double)(j * j));  // 1 * 2 * 3 = 6
    }
    results[idx++] = loop_result2;
    
    // 9. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                 // 0.0
    results[idx++] = __builtin_sin(0.0);       // 0.0
    results[idx++] = cos(0.0);                 // 1.0
    results[idx++] = __builtin_cos(0.0);       // 1.0
    
    // 10. Additional one-argument functions
    results[idx++] = log(1.0);                 // 0.0
    results[idx++] = __builtin_log(1.0);       // 0.0
    results[idx++] = log10(1.0);               // 0.0
    results[idx++] = __builtin_log10(1.0);     // 0.0
    
    // 11. Compute checksum for observable output
    double sum = 0.0;
    for (int j = 0; j < idx; j++) {
        sum += results[j];
    }
    
    int int_sum = 0;
    for (int j = 0; j < 4; j++) {
        int_sum += int_results[j];
    }
    
    // Final mixed usage
    int final_idx = (int)pow(2.0, sqrt(4.0));  // 2^2 = 4
    results[final_idx % idx] += 1.0;           // Modify based on folded computation
    
    printf("Floating-point sum: %f\n", sum);
    printf("Integer sum: %d\n", int_sum);
    printf("Final index used: %d\n", final_idx);
    
    return 0;
}
