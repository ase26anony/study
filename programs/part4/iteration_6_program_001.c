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
    const double base = 2.0;
    
    // 1. Direct constant arguments producing integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log2(8.0);               // 3.0
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0 (explicit builtin)
    
    // 2. Symbolic arguments with constant relationships
    results[idx++] = pow(base, (double)n);    // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    results[idx++] = exp2((double)m);         // 2^3 = 8.0
    
    // 3. Assign directly to integer variables (triggers conversion check)
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(36.0);                      // 6
    int i3 = __builtin_exp2(5.0);             // 32
    int_results[0] = i1 + i2 + i3;
    
    // 4. Use in array indexing context
    double array[10] = {0};
    array[(int)sqrt(9.0)] = 3.14;             // array[3]
    array[(int)pow(2.0, 2.0)] = 2.71;         // array[4]
    results[idx++] = array[3] + array[4];
    
    // 5. Compare to integer constants
    if (pow(2.0, 3.0) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(16.0) == 4.0) {
        results[idx++] = 2.0;
    }
    
    // 6. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0) / 2.0);               // sqrt(32) ≈ 5.657, not integer
    results[idx++] = cbrt(pow(2.0, 9.0));                  // 2^3 = 8.0
    
    // 7. One-argument functions
    results[idx++] = __builtin_sqrt(81.0);     // 9.0
    results[idx++] = __builtin_cbrt(64.0);     // 4.0
    results[idx++] = __builtin_log2(1024.0);   // 10.0
    results[idx++] = __builtin_exp2(6.0);      // 64.0
    
    // 8. Two-argument functions (besides pow)
    results[idx++] = fmod(15.0, 4.0);          // 3.0
    results[idx++] = remainder(17.0, 5.0);     // 2.0
    
    // 9. Loop with invariant arguments (constant propagation)
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is not constant, but 2.0 and 3.0 are
        loop_sum += pow(2.0, 3.0);  // Always adds 8.0
    }
    results[idx++] = loop_sum;
    
    // 10. Trigonometric functions that can produce integer results
    results[idx++] = sin(0.0);                  // 0.0
    results[idx++] = cos(0.0);                  // 1.0
    results[idx++] = __builtin_sin(M_PI);       // ~0.0 (may not fold exactly)
    results[idx++] = __builtin_cos(0.0);        // 1.0
    
    // 11. Mixed integer/floating context with conditional
    double x = 0.0;
    for (int k = 0; k < 4; k++) {
        if (k % 2 == 0) {
            x += pow(3.0, 2.0);  // 9.0
        } else {
            x += sqrt(4.0);      // 2.0
        }
    }
    results[idx++] = x;
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    sum += int_results[0];
    
    // Print checksum (cast to int for deterministic output across platforms)
    printf("Checksum: %d\n", (int)sum);
    
    return 0;
}
