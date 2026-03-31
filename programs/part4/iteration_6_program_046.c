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
    const double base = 2.0;
    
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
    
    // 3. Mixed integer/floating contexts
    int i1 = pow(2.0, 3.0);                   // Should fold to 8
    int i2 = sqrt(9.0);                       // Should fold to 3
    int_results[0] = i1 + i2;
    
    // 4. Using constant-propagated variables
    results[idx++] = pow(base, n);            // 2^5 = 32.0
    results[idx++] = sqrt(n * n);             // sqrt(25) = 5.0
    results[idx++] = exp2(m);                 // 2^3 = 8.0
    
    // 5. Nested calls
    results[idx++] = pow(sqrt(16.0), 2.0);    // 4^2 = 16.0
    results[idx++] = sqrt(pow(3.0, 2.0));     // sqrt(9) = 3.0
    results[idx++] = exp2(log2(8.0));         // 8.0
    
    // 6. Single-argument functions
    results[idx++] = __builtin_log(exp(1.0)); // log(e) = 1.0
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    
    // 7. Loop with invariant arguments (constant propagation)
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; iter++) {
        // These should fold despite being in loop (invariant arguments)
        loop_sum += pow(2.0, iter) + sqrt(iter * iter + 1.0);
    }
    results[idx++] = loop_sum;
    
    // 8. Conditional constant propagation
    int x = 4;
    if (x > 0) {
        results[idx++] = sqrt(x * x);         // sqrt(16) = 4.0
        results[idx++] = pow(2.0, x);         // 16.0
    }
    
    // 9. Array indexing with folded results
    int array[10] = {0,1,2,3,4,5,6,7,8,9};
    int array_idx = (int)sqrt(25.0);          // Should fold to 5
    int_results[1] = array[array_idx];
    
    // 10. Comparisons with integer constants
    int count = 0;
    if (pow(2.0, 3.0) == 8.0) count++;       // Should fold
    if (sqrt(9.0) == 3.0) count++;           // Should fold
    if (exp2(4.0) == 16.0) count++;          // Should fold
    int_results[2] = count;
    
    // 11. More complex expressions
    results[idx++] = pow(2.0, log2(8.0) / log2(2.0));  // 8.0
    results[idx++] = cbrt(pow(3.0, 3.0));              // 3.0
    
    // 12. Two-argument atan2 with integer result
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    for (int j = 0; j < idx; j++) {
        sum += results[j];
    }
    
    int int_sum = 0;
    for (int j = 0; j < 3; j++) {
        int_sum += int_results[j];
    }
    
    // Print checksum (observable output)
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum);
    
    // Use results in control flow
    if ((int)sum % 2 == 0) {
        printf("Even result\n");
    } else {
        printf("Odd result\n");
    }
    
    return 0;
}
