#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);          // 8.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = sqrt(25.0);             // 5.0
    results[idx++] = __builtin_sqrt(16.0);    // 4.0
    results[idx++] = exp2(4.0);              // 16.0
    results[idx++] = __builtin_exp2(3.0);     // 8.0
    results[idx++] = cbrt(27.0);             // 3.0
    results[idx++] = __builtin_cbrt(8.0);     // 2.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, n);           // 32.0
    results[idx++] = __builtin_pow(base, (double)n); // 32.0
    
    // 3. Mixed integer and floating-point contexts
    int int_var;
    int_var = pow(4.0, 1.5);                 // 8.0 -> assigned to int
    int_results[0] = int_var;
    
    int_var = __builtin_sqrt(36.0);          // 6.0 -> assigned to int
    int_results[1] = int_var;
    
    // 4. Use in array indexing
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(81.0);         // 9
    results[idx++] = array[array_idx];
    
    array_idx = (int)__builtin_pow(2.0, 3.0); // 8
    results[idx++] = array[array_idx];
    
    // 5. Compare to integer
    if (pow(10.0, 2.0) == 100) {
        results[idx++] = 1.0;
    }
    
    if (__builtin_sqrt(49.0) == 7) {
        results[idx++] = 2.0;
    }
    
    // 6. Nested calls and combined expressions
    results[idx++] = pow(sqrt(64.0), log(8.0) / log(2.0)); // 8^3 = 512
    results[idx++] = __builtin_pow(__builtin_sqrt(100.0), 2.0); // 10^2 = 100
    
    // 7. Functions with single argument (testing call_expr_nargs > 0)
    results[idx++] = exp2(log2(32.0));       // 32.0
    results[idx++] = __builtin_exp2(__builtin_log2(64.0)); // 64.0
    
    // 8. Functions with two arguments (testing call_expr_nargs > 1)
    results[idx++] = pow(2.0, 5.0);          // 32.0
    results[idx++] = __builtin_pow(3.0, 3.0); // 27.0
    
    // 9. Place calls inside a loop with fixed iteration count
    double loop_result = 0.0;
    for (int i = 0; i < 3; i++) {
        // Loop-invariant constant arguments
        loop_result += pow(2.0, i);          // 1 + 2 + 4 = 7
    }
    results[idx++] = loop_result;
    
    for (int i = 0; i < 4; i++) {
        // Another loop with __builtin_ version
        loop_result += __builtin_sqrt((double)(i * i)); // sqrt(0,1,4,9) = 0+1+2+3=6
    }
    results[idx++] = loop_result;
    
    // 10. Trigonometric functions that can produce integer results
    results[idx++] = sin(0.0);               // 0.0
    results[idx++] = __builtin_sin(0.0);     // 0.0
    results[idx++] = cos(0.0);               // 1.0
    results[idx++] = __builtin_cos(0.0);     // 1.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // 11. Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    int int_checksum = 0;
    for (int i = 0; i < 2; i++) {
        int_checksum += int_results[i];
    }
    
    // Print results to ensure computations aren't optimized away
    printf("Checksum (double): %f\n", checksum);
    printf("Checksum (int): %d\n", int_checksum);
    printf("Array[9] = %f\n", array[9]);
    
    return 0;
}
