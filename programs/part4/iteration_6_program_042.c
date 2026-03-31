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
    results[idx++] = log(256.0) / log(2.0);   // 8.0 (log2 via change of base)
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments that are constant after propagation
    results[idx++] = pow(base, n);            // 2^5 = 32.0
    results[idx++] = sqrt((double)(n * n));   // sqrt(25) = 5.0
    
    // 4. Assign directly to integer variables (triggers integer-valued check)
    int i1 = pow(4.0, 2.0);                   // 16
    int i2 = sqrt(81.0);                      // 9
    int i3 = exp2(3.0);                       // 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in array indexing context
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int index1 = (int)pow(2.0, 2.0);          // 4
    int index2 = (int)sqrt(9.0);              // 3
    results[idx++] = array[index1] + array[index2];
    
    // 6. Compare to integer constants
    if (pow(2.0, 3.0) == 8.0) {
        results[idx++] = 1.0;
    }
    if (sqrt(100.0) == 10.0) {
        results[idx++] = 2.0;
    }
    
    // 7. Nested calls and combined expressions
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    results[idx++] = exp2(log2(pow(2.0, 4.0)));            // 16.0
    results[idx++] = cbrt(pow(3.0, 3.0));                  // 3.0
    
    // 8. Single-argument functions
    results[idx++] = exp2(6.0);               // 64.0
    results[idx++] = sqrt(144.0);             // 12.0
    results[idx++] = cbrt(64.0);              // 4.0
    results[idx++] = log(1.0);                // 0.0
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    
    // 9. Conditional constant propagation in loop
    double loop_result = 0.0;
    for (int j = 0; j < 3; j++) {
        // j is constant within each iteration for folding
        loop_result += pow(2.0, j);           // 1 + 2 + 4 = 7.0
    }
    results[idx++] = loop_result;
    
    // 10. Two-argument functions beyond pow
    results[idx++] = atan2(0.0, 1.0);         // 0.0
    results[idx++] = fmod(15.0, 4.0);         // 3.0
    
    // 11. More complex integer-valued expressions
    results[idx++] = pow(10.0, log10(1000.0)); // 1000.0
    results[idx++] = sqrt(pow(6.0, 2.0));      // 6.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // 12. Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int k = 0; k < idx; k++) {
        checksum += results[k];
    }
    checksum += int_results[0];
    
    // Print checksum as integer (truncates, but that's fine for our purpose)
    printf("Checksum: %d\n", (int)checksum);
    
    return (int)checksum % 100;
}
