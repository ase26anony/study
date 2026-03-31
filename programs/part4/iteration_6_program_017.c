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
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(16.0);    // 4.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments that are provably constant
    results[idx++] = pow(base, (double)n);    // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign directly to integer variables (triggers conversion check)
    int i1 = pow(4.0, 1.5);                   // 4^1.5 = 8.0
    int i2 = sqrt(81.0);                      // 9.0
    int_results[0] = i1 + i2;
    
    // 5. Use in array indexing context
    int array[10] = {0};
    array[(int)sqrt(9.0)] = 42;               // array[3] = 42
    array[(int)pow(2.0, 2.0)] = 100;          // array[4] = 100
    
    // 6. Compare to integer constants
    if (pow(2.0, 3.0) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0) / 2.0);                // sqrt(32) ≈ 5.657, not integer
    results[idx++] = cbrt(pow(2.0, 9.0));                   // 512^(1/3) = 8.0
    
    // 8. Functions with single argument (testing call_expr_nargs > 0 branch)
    results[idx++] = exp2(6.0);               // 64.0
    results[idx++] = log(exp(1.0));           // 1.0
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    
    // 9. Functions with two arguments (testing call_expr_nargs > 1 branch)
    results[idx++] = pow(5.0, 0.0);           // 1.0
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    
    // 10. Loop with invariant constants (conditional constant propagation)
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        // These arguments are loop-invariant and should fold
        loop_sum += pow(2.0, (double)iter) + sqrt((double)(iter * iter + 1));
    }
    results[idx++] = loop_sum;
    
    // 11. Mixed integer/float context with control flow
    double val = 0.0;
    for (int k = 0; k < 4; k++) {
        double temp = pow(2.0, (double)k);
        if (temp == (int)temp) {  // Compare to integer
            val += temp;
        }
    }
    results[idx++] = val;
    
    // 12. More complex expressions that should fold to integers
    results[idx++] = pow(10.0, log10(1000.0));  // 10^3 = 1000.0
    results[idx++] = sqrt(pow(6.0, 2.0));       // sqrt(36) = 6.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int j = 0; j < idx; j++) {
        checksum += results[j];
    }
    checksum += int_results[0];
    checksum += array[3] + array[4];
    
    // Print checksum as integer (truncates, matching integer-valued check)
    printf("Result: %d\n", (int)checksum);
    
    return (int)checksum % 256;
}
