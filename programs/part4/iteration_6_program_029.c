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
    
    // 1. Direct constant arguments with integer results
    results[idx++] = pow(2.0, 3.0);           // 8.0
    results[idx++] = sqrt(25.0);              // 5.0
    results[idx++] = exp2(4.0);               // 16.0
    results[idx++] = cbrt(27.0);              // 3.0
    results[idx++] = log(1.0);                // 0.0
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(16.0);    // 4.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments with constant relationships
    results[idx++] = pow(base, n);            // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign directly to integer variables (triggers conversion check)
    int i1 = pow(2.0, 4.0);                   // 16
    int i2 = sqrt(36.0);                      // 6
    int i3 = exp2(3.0);                       // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in array indexing
    double array[10] = {0};
    array[(int)sqrt(9.0)] = 3.14;             // array[3]
    array[(int)pow(2.0, 2.0)] = 2.71;         // array[4]
    results[idx++] = array[3] + array[4];
    
    // 6. Compare to integer constants
    if (pow(2.0, 3.0) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(49.0) == 7.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(32.0));                     // 32.0
    results[idx++] = cbrt(pow(3.0, 3.0));                  // 3.0
    
    // 8. Functions with single argument
    results[idx++] = exp2(m);                              // 8.0
    results[idx++] = sqrt((double)(m * m * m * m));        // sqrt(81) = 9.0
    results[idx++] = cbrt(64.0);                           // 4.0
    
    // 9. Loop with invariant arguments (constant propagation)
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is not constant, but 2.0 and 3.0 are
        loop_sum += pow(2.0, 3.0);  // Always adds 8.0
    }
    results[idx++] = loop_sum;
    
    // 10. Mixed integer/float context with atan2 (two arguments)
    results[idx++] = atan2(0.0, 1.0);  // 0.0 (integer result)
    
    // 11. More complex integer-valued real results
    results[idx++] = sin(0.0);         // 0.0
    results[idx++] = cos(0.0);         // 1.0
    results[idx++] = log10(100.0);     // 2.0
    results[idx++] = log2(1024.0);     // 10.0
    
    // 12. Using the results in control flow
    int count = 0;
    for (int k = 0; k < idx; k++) {
        // Check if result is integer-valued
        if (results[k] == (double)((int)results[k])) {
            count++;
        }
    }
    int_results[1] = count;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    int int_checksum = 0;
    
    for (int k = 0; k < idx; k++) {
        checksum += results[k];
    }
    for (int k = 0; k < 2; k++) {
        int_checksum += int_results[k];
    }
    
    // Print results to ensure execution
    printf("Checksum: %f\n", checksum);
    printf("Integer checksum: %d\n", int_checksum);
    printf("Integer-valued results count: %d\n", int_results[1]);
    
    return 0;
}
