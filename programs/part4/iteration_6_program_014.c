#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // Constant integer values for symbolic propagation
    const int n = 5;
    const int m = 3;
    const double base = 2.0;
    
    // 1. Direct constant arguments with integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log(exp(2.0));           // 2.0
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(16.0);    // 4.0
    results[idx++] = __builtin_exp2(3.0);     // 8.0
    
    // 3. Symbolic arguments that are provably constant
    results[idx++] = pow(base, (double)n);    // 32.0
    results[idx++] = sqrt((double)(n * n));   // 5.0
    results[idx++] = exp2((double)m);         // 8.0
    
    // 4. Mixed integer and floating-point contexts
    int i1 = pow(2.0, 3.0);                   // Should fold to 8
    int i2 = sqrt(49.0);                      // Should fold to 7
    int i3 = exp2(2.0);                       // Should fold to 4
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    // 5. Use in array indexing
    double array[10] = {0};
    array[(int)sqrt(9.0)] = 3.14;             // array[3]
    array[(int)pow(2.0, 2.0)] = 2.71;         // array[4]
    
    // 6. Comparisons with integers
    if (pow(2.0, 3.0) == 8) {
        results[idx++] = 1.0;
    }
    
    if (sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    results[idx++] = exp2(log2(pow(2.0, 3.0)));            // Should be 8
    
    // 8. Functions with single argument
    results[idx++] = exp2(5.0);               // 32.0
    results[idx++] = sqrt(36.0);              // 6.0
    results[idx++] = cbrt(64.0);              // 4.0
    results[idx++] = log(exp(3.0));           // 3.0
    
    // 9. Functions with two arguments
    results[idx++] = pow(4.0, 0.5);           // 2.0 (sqrt via pow)
    results[idx++] = atan2(0.0, 1.0);         // 0.0 (integer result)
    
    // 10. Loop with invariant constants (fixed iteration count)
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        // These arguments are loop-invariant and should fold
        loop_sum += pow(2.0, (double)iter);
    }
    results[idx++] = loop_sum;                // 1 + 2 + 4 = 7
    
    // 11. More complex expressions that should yield integer results
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    results[idx++] = tan(0.0);                // 0.0
    
    // 12. Logarithm of powers
    results[idx++] = log2(pow(2.0, 5.0));     // 5.0
    results[idx++] = log10(pow(10.0, 3.0));   // 3.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int j = 0; j < idx; j++) {
        checksum += results[j];
    }
    
    // Add integer results to checksum
    for (int j = 0; j < 3; j++) {
        checksum += int_results[j];
    }
    
    // Add array elements
    checksum += array[3] + array[4];
    
    // Print checksum to ensure all computations are used
    printf("Checksum: %f\n", checksum);
    printf("As integer: %d\n", (int)checksum);
    
    return 0;
}
