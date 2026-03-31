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
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments that become constant
    const int n = 3;
    const double base = 2.0;
    results[idx++] = pow(base, (double)n);    // 8.0
    results[idx++] = exp2((double)n);         // 8.0
    
    // 4. Mixed integer/float contexts
    int int_var = pow(4.0, 1.5);              // 8.0 -> 8
    int_results[0] = sqrt(49.0);              // 7.0 -> 7
    int_results[1] = (int)pow(5.0, 2.0);      // 25.0 -> 25
    
    // 5. Nested calls
    results[idx++] = pow(sqrt(16.0), log2(8.0));  // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0) / 2.0);      // sqrt(32) ≈ 5.657, not integer
    
    // 6. Single-argument functions
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    results[idx++] = __builtin_sin(0.0);      // 0.0
    results[idx++] = __builtin_cos(0.0);      // 1.0
    
    // 7. Loop with invariant constants (fixed iteration)
    double loop_result = 1.0;
    for (int i = 0; i < 3; i++) {
        // pow(2,0)=1, pow(2,1)=2, pow(2,2)=4
        loop_result *= pow(2.0, (double)i);
    }
    results[idx++] = loop_result;             // 1*2*4 = 8.0
    
    // 8. Conditional constant propagation
    int m = 2;
    if (m > 1) {
        m = 4;  // Becomes constant 4
    }
    results[idx++] = pow(2.0, (double)m);     // 16.0
    
    // 9. Use in array indexing (triggers integer-valued check)
    int array[10] = {0,1,2,3,4,5,6,7,8,9};
    int array_idx = sqrt(9.0);                // 3.0 -> 3
    int_results[2] = array[array_idx];        // 3
    
    // 10. Comparisons with integers
    int count = 0;
    if (pow(3.0, 2.0) == 9) count++;         // true
    if (sqrt(100.0) == 10) count++;          // true
    if (exp2(3.0) == 8) count++;             // true
    int_results[3] = count;                   // 3
    
    // 11. More complex expressions
    results[idx++] = pow(2.0, log2(8.0));     // 8.0
    results[idx++] = cbrt(pow(2.0, 9.0));     // 8.0
    
    // Ensure idx doesn't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double sum = 0.0;
    int int_sum = 0;
    
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    
    for (int i = 0; i < 4; i++) {
        int_sum += int_results[i];
    }
    
    // Use results in output
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum + int_var);
    
    // Use in control flow
    if ((int)sum % 2 == 0) {
        printf("Even total\n");
    } else {
        printf("Odd total\n");
    }
    
    return 0;
}
