#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    const int iterations = 3;
    
    // Variables to store results
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int array_index = 0;
    
    // 2. Direct constant arguments with integer results
    // Using both standard and __builtin_ versions
    fp_results[array_index++] = pow(2.0, 3.0);           // 8.0
    fp_results[array_index++] = __builtin_pow(3.0, 2.0); // 9.0
    fp_results[array_index++] = sqrt(25.0);              // 5.0
    fp_results[array_index++] = __builtin_sqrt(16.0);    // 4.0
    fp_results[array_index++] = exp2(4.0);               // 16.0
    fp_results[array_index++] = __builtin_exp2(3.0);     // 8.0
    
    // 3. Symbolic arguments with constant relationships
    // These should be folded due to constant propagation
    fp_results[array_index++] = pow(base, (double)n);    // 2^5 = 32
    fp_results[array_index++] = sqrt((double)(n * n));   // sqrt(25) = 5
    
    // 4. Assign to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, 3.0);          // Should be 8
    int i2 = __builtin_sqrt(9.0);    // Should be 3
    int i3 = exp2(3.0);              // Should be 8
    int_results[0] = i1 + i2 + i3;
    
    // 5. Use in array indexing
    double arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idx1 = (int)pow(2.0, 2.0);      // 4
    int idx2 = (int)__builtin_sqrt(16.0); // 4
    int_results[1] = (int)arr[idx1] + (int)arr[idx2];
    
    // 6. Compare to integer constants
    int cmp_results = 0;
    if (pow(2.0, 3.0) == 8.0) cmp_results += 1;
    if (__builtin_sqrt(25.0) == 5.0) cmp_results += 2;
    if (exp2(4.0) == 16.0) cmp_results += 4;
    int_results[2] = cmp_results;
    
    // 7. Place calls inside a loop with fixed iterations
    // Loop-invariant constants should still allow folding
    double loop_sum = 0.0;
    for (int j = 0; j < iterations; j++) {
        // These arguments are loop-invariant constants
        loop_sum += pow(2.0, (double)j);      // 1 + 2 + 4 = 7
        loop_sum += __builtin_sqrt((double)(j * j + 1)); // sqrt(1) + sqrt(2) + sqrt(5)
    }
    fp_results[array_index++] = loop_sum;
    
    // 8. Nested calls and combined expressions
    fp_results[array_index++] = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    fp_results[array_index++] = __builtin_exp2(__builtin_log2(8.0));  // should be 8
    
    // 9. Functions with one argument (sqrt, exp2, log, sin, cos)
    fp_results[array_index++] = sqrt(36.0);          // 6.0
    fp_results[array_index++] = __builtin_cbrt(27.0); // 3.0
    fp_results[array_index++] = log(1.0);            // 0.0
    fp_results[array_index++] = __builtin_log10(100.0); // 2.0
    fp_results[array_index++] = sin(0.0);            // 0.0
    fp_results[array_index++] = __builtin_cos(0.0);  // 1.0
    
    // 10. Functions with two arguments (pow, atan2)
    fp_results[array_index++] = pow(4.0, 0.5);       // 2.0 (sqrt(4))
    fp_results[array_index++] = __builtin_atan2(0.0, 1.0); // 0.0
    
    // 11. More integer context usage
    int i4 = cbrt(64.0);            // Should be 4
    int i5 = __builtin_log2(16.0);  // Should be 4
    int_results[3] = i4 * i5;       // 16
    
    // Use result in array indexing
    int idx3 = (int)pow(3.0, 2.0);  // 9
    if (idx3 < ARRAY_SIZE) {
        int_results[4] = (int)fp_results[idx3 % array_index];
    }
    
    // 12. Compute checksum
    double fp_sum = 0.0;
    for (int k = 0; k < array_index; k++) {
        fp_sum += fp_results[k];
    }
    
    int int_sum = 0;
    for (int k = 0; k < 5; k++) {
        int_sum += int_results[k];
    }
    
    // Final checksum combining floating and integer results
    int checksum = (int)fp_sum + int_sum;
    
    // Print to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    printf("Array index used: %d\n", array_index);
    
    return 0;
}
