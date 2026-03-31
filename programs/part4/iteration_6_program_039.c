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
    results[idx++] = log(256.0) / log(2.0);   // 8.0 (log2 via change of base)
    
    // 2. Using __builtin_ versions explicitly
    results[idx++] = __builtin_pow(3.0, 2.0); // 9.0
    results[idx++] = __builtin_sqrt(36.0);    // 6.0
    results[idx++] = __builtin_exp2(5.0);     // 32.0
    
    // 3. Symbolic arguments with constant relationships
    const int n = 5;
    const double base = 2.0;
    results[idx++] = pow(base, (double)n);    // 32.0
    
    // 4. Mixed integer/float contexts
    int int_var;
    int_var = pow(4.0, 1.5);                  // 8.0 -> assigned to int
    int_results[0] = int_var;
    
    // 5. Use in array indexing
    double array[10] = {0};
    array[(int)sqrt(81.0)] = 3.14;            // array[9]
    
    // 6. Compare to integer
    if (pow(2.0, 4.0) == 16) {
        results[idx++] = 1.0;
    }
    
    // 7. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        // Arguments are constant within loop
        loop_sum += pow(2.0, (double)i);      // 1 + 2 + 4
    }
    results[idx++] = loop_sum;
    
    // 8. Nested calls
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64.0
    
    // 9. Single-argument functions
    results[idx++] = exp2(6.0);               // 64.0
    results[idx++] = sqrt(100.0);             // 10.0
    
    // 10. Two-argument functions
    results[idx++] = pow(5.0, 2.0);           // 25.0
    results[idx++] = atan2(0.0, 1.0);         // 0.0 (integer result)
    
    // 11. Trigonometric functions with integer results
    results[idx++] = sin(0.0);                // 0.0
    results[idx++] = cos(0.0);                // 1.0
    
    // 12. More complex expressions
    results[idx++] = pow(2.0, log(256.0) / log(2.0)) / 16.0; // 256/16 = 16.0
    
    // Ensure we don't exceed array bounds
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    // Use integer results too
    int int_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE && i < 5; i++) {
        int_checksum += int_results[i];
    }
    
    // Print checksums (observable output)
    printf("Floating-point checksum: %f\n", checksum);
    printf("Integer checksum: %d\n", int_checksum);
    
    return 0;
}
