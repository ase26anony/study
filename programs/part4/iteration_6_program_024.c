#include <math.h>
#include <stdio.h>

int main() {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 4;
    const double perfect_square = 64.0;
    
    // Results storage
    double fp_results[20] = {0};
    int int_results[20] = {0};
    int idx = 0;
    
    // 2. Direct constant calls with integer results
    // These should be folded at compile time
    double r1 = pow(2.0, 3.0);          // 8.0
    double r2 = sqrt(25.0);             // 5.0
    double r3 = exp2(4.0);              // 16.0
    double r4 = __builtin_pow(3.0, 2.0); // 9.0
    double r5 = __builtin_sqrt(36.0);    // 6.0
    
    // Store results
    fp_results[idx++] = r1;
    fp_results[idx++] = r2;
    fp_results[idx++] = r3;
    fp_results[idx++] = r4;
    fp_results[idx++] = r5;
    
    // 3. Calls with symbolic but constant arguments
    double r6 = pow(base, n);           // 2^5 = 32.0
    double r7 = sqrt(perfect_square);   // 8.0
    double r8 = __builtin_exp2(exp_int); // 16.0
    double r9 = log2(8.0);              // 3.0
    double r10 = __builtin_log2(64.0);   // 6.0
    
    fp_results[idx++] = r6;
    fp_results[idx++] = r7;
    fp_results[idx++] = r8;
    fp_results[idx++] = r9;
    fp_results[idx++] = r10;
    
    // 4. Mixed integer and floating-point contexts
    // Direct assignment to integer (triggers conversion check)
    int i1 = pow(2.0, 3.0);             // Should be 8
    int i2 = sqrt(49.0);                // Should be 7
    int i3 = __builtin_exp2(3.0);       // Should be 8
    int i4 = log2(256.0);               // Should be 8
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    int_results[3] = i4;
    
    // 5. Use in array indexing
    int array[100] = {0};
    array[(int)sqrt(16.0)] = 42;        // array[4] = 42
    array[(int)pow(3.0, 2.0)] = 99;     // array[9] = 99
    array[(int)__builtin_sqrt(81.0)] = 77; // array[9] = 77
    
    // 6. Compare to integer constants
    int cmp_results[5] = {0};
    if (pow(2.0, 4.0) == 16.0) cmp_results[0] = 1;
    if (sqrt(100.0) == 10.0) cmp_results[1] = 1;
    if (__builtin_exp2(5.0) == 32.0) cmp_results[2] = 1;
    if (log2(128.0) == 7.0) cmp_results[3] = 1;
    
    // 7. Nested and combined calls
    double r11 = pow(sqrt(16.0), log2(8.0)); // 4^3 = 64.0
    double r12 = sqrt(pow(6.0, 2.0));        // 6.0
    double r13 = exp2(log2(32.0));           // 32.0
    double r14 = __builtin_pow(__builtin_sqrt(9.0), 3.0); // 3^3 = 27.0
    
    fp_results[idx++] = r11;
    fp_results[idx++] = r12;
    fp_results[idx++] = r13;
    fp_results[idx++] = r14;
    
    // 8. Loop with invariant constants (for conditional constant propagation)
    double loop_sum = 0.0;
    const int iterations = 3;
    for (int i = 0; i < iterations; i++) {
        // Arguments are constant within loop
        double loop_val = pow(2.0, (double)i); // 1, 2, 4
        loop_sum += sqrt(loop_val * loop_val); // 1, 2, 4
    }
    fp_results[idx++] = loop_sum; // 7.0
    
    // 9. More single-argument functions
    double r15 = cbrt(27.0);           // 3.0
    double r16 = __builtin_cbrt(8.0);   // 2.0
    double r17 = exp(0.0);             // 1.0
    double r18 = __builtin_exp(0.0);    // 1.0
    double r19 = sin(0.0);             // 0.0
    double r20 = cos(0.0);             // 1.0
    
    fp_results[idx++] = r15;
    fp_results[idx++] = r16;
    fp_results[idx++] = r17;
    fp_results[idx++] = r18;
    fp_results[idx++] = r19;
    fp_results[idx++] = r20;
    
    // 10. Two-argument functions
    double r21 = atan2(0.0, 1.0);      // 0.0
    double r22 = __builtin_atan2(0.0, 1.0); // 0.0
    double r23 = pow(1.0, 100.0);      // 1.0
    
    fp_results[idx++] = r21;
    fp_results[idx++] = r22;
    fp_results[idx++] = r23;
    
    // 11. Compute checksum for observable output
    double fp_sum = 0.0;
    for (int i = 0; i < idx; i++) {
        fp_sum += fp_results[i];
    }
    
    int int_sum = 0;
    for (int i = 0; i < 4; i++) {
        int_sum += int_results[i];
    }
    
    for (int i = 0; i < 4; i++) {
        int_sum += cmp_results[i];
    }
    
    int_sum += array[4] + array[9];
    
    printf("Floating-point sum: %f\n", fp_sum);
    printf("Integer sum: %d\n", int_sum);
    
    return 0;
}
