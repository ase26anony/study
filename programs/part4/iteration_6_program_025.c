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
    
    // 3. Symbolic arguments with constant relationships
    results[idx++] = pow(base, (double)n);    // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign directly to integer variables (triggers conversion check)
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(36.0);                      // 6
    int i3 = exp2(3.0);                       // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in integer array indexing
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int index1 = (int)pow(2.0, 2.0);          // 4
    int index2 = (int)sqrt(9.0);              // 3
    results[idx++] = array[index1] + array[index2];
    
    // 6. Compare to integer constants
    if (pow(2.0, 3.0) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(49.0) == 7.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0) / 2.0);               // sqrt(32) ≈ 5.657, not integer
    results[idx++] = cbrt(pow(2.0, 9.0));                  // 2^(9/3) = 8.0
    
    // 8. Single-argument functions
    results[idx++] = exp2(6.0);                           // 64.0
    results[idx++] = sqrt(81.0);                          // 9.0
    results[idx++] = cbrt(64.0);                          // 4.0
    results[idx++] = log2(1024.0);                        // 10.0
    
    // 9. Two-argument functions
    results[idx++] = pow(4.0, 1.5);                       // 8.0 (4^(3/2))
    results[idx++] = pow(9.0, 0.5);                       // 3.0 (sqrt(9))
    
    // 10. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is not constant, but the expression uses constants
        loop_sum += pow(2.0, (double)m);  // 2^3 = 8.0, repeated 3 times
    }
    results[idx++] = loop_sum;
    
    // 11. Trigonometric functions with special cases
    results[idx++] = sin(0.0);                            // 0.0
    results[idx++] = cos(0.0);                            // 1.0
    results[idx++] = sin(M_PI);                           // ~0.0 (not exactly integer)
    results[idx++] = cos(M_PI);                           // -1.0
    
    // 12. More mixed contexts
    int int_sum = 0;
    for (int k = 0; k < 4; k++) {
        double val = pow(2.0, (double)k);  // 1, 2, 4, 8
        if (val == (double)(1 << k)) {     // Compare to integer
            int_sum += (int)val;
        }
    }
    results[idx++] = (double)int_sum;
    
    // Ensure we don't overflow the array
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int j = 0; j < idx; j++) {
        checksum += results[j];
    }
    
    // Also use integer results
    int int_checksum = int_results[0];
    for (int j = 1; j < ARRAY_SIZE; j++) {
        int_checksum += int_results[j];
    }
    
    printf("Double checksum: %f\n", checksum);
    printf("Integer checksum: %d\n", int_checksum);
    
    return 0;
}
