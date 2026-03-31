#include <math.h>
#include <stdio.h>

int main(void) {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    int result_sum = 0;
    double fp_sum = 0.0;
    
    // Array for indexing tests
    int arr[100] = {0};
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    // 2. Direct constant arguments producing integer results
    // These should trigger integer_valued_real_call_p during folding
    
    // pow with two constant arguments
    double d1 = pow(2.0, 3.0);          // 8.0
    double d2 = __builtin_pow(3.0, 2.0); // 9.0
    
    // sqrt of perfect squares
    double d3 = sqrt(25.0);             // 5.0
    double d4 = __builtin_sqrt(36.0);   // 6.0
    
    // exp2 with integer exponents
    double d5 = exp2(4.0);              // 16.0
    double d6 = __builtin_exp2(5.0);    // 32.0
    
    // log2 of powers of 2
    double d7 = log2(8.0);              // 3.0
    double d8 = __builtin_log2(64.0);   // 6.0
    
    // cbrt of perfect cubes
    double d9 = cbrt(27.0);             // 3.0
    double d10 = __builtin_cbrt(125.0); // 5.0
    
    // 3. Assign to integer variables (triggers conversion check)
    int i1 = pow(4.0, 2.0);             // 16
    int i2 = __builtin_sqrt(81.0);      // 9
    int i3 = exp2(3.0);                 // 8
    
    // 4. Use in array indexing
    int idx1 = (int)sqrt(49.0);         // 7
    int idx2 = (int)__builtin_pow(2.0, 4.0); // 16
    arr[idx1] += 1;
    arr[idx2] += 2;
    
    // 5. Compare to integer constants
    if (pow(2.0, 5.0) == 32) {
        arr[0] += 10;
    }
    if (__builtin_sqrt(100.0) == 10) {
        arr[1] += 20;
    }
    
    // 6. Symbolic arguments with constant relationships
    const int exp = 3;
    double d11 = pow(base, exp);        // 8.0
    double d12 = __builtin_pow(base, n); // 32.0
    
    // 7. Nested calls
    double d13 = pow(sqrt(16.0), log2(8.0)); // 4^3 = 64.0
    double d14 = __builtin_exp2(__builtin_log2(32.0)); // 32.0
    
    // 8. Place calls in a loop with invariant constants
    for (int iter = 0; iter < 3; iter++) {
        // Loop-invariant computations
        double loop_val = pow(2.0, iter + 2.0); // 4.0, 8.0, 16.0
        int loop_int = (int)__builtin_sqrt(loop_val * loop_val);
        
        // Use in control flow
        if (loop_val == 8.0) {
            arr[2] += loop_int;
        }
        
        fp_sum += loop_val;
        result_sum += loop_int;
    }
    
    // 9. Single-argument functions
    double d15 = exp2(6.0);            // 64.0
    double d16 = __builtin_log(exp(1.0)); // 1.0
    double d17 = sin(0.0);             // 0.0
    double d18 = __builtin_cos(0.0);   // 1.0
    
    // 10. Two-argument functions beyond pow
    double d19 = __builtin_atan2(0.0, 1.0); // 0.0
    
    // 11. Mixed expressions
    double d20 = pow(2.0, sqrt(9.0)) + exp2(log2(8.0)); // 8 + 8 = 16.0
    
    // 12. More integer assignments
    int i4 = cbrt(216.0);              // 6
    int i5 = __builtin_log2(128.0);    // 7
    
    // 13. Use results to prevent elimination
    result_sum += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result_sum += (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
    result_sum += i1 + i2 + i3 + i4 + i5;
    result_sum += (int)d11 + (int)d12 + (int)d13 + (int)d14;
    result_sum += (int)d15 + (int)d16 + (int)d17 + (int)d18;
    result_sum += (int)d19 + (int)d20;
    
    // Add array values
    for (int i = 0; i < 20; i++) {
        result_sum += arr[i];
    }
    
    // 14. Print checksum
    printf("Result checksum: %d\n", result_sum);
    printf("Floating-point sum: %f\n", fp_sum);
    
    return 0;
}
