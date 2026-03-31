#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    // 1. Constant arguments producing integer results
    const double base = 2.0;
    const double exponent = 3.0;
    
    // Direct constant calls - should fold to integer values
    double r1 = pow(2.0, 3.0);           // 8.0
    double r2 = __builtin_pow(2.0, 3.0); // 8.0
    double r3 = sqrt(9.0);               // 3.0
    double r4 = __builtin_sqrt(9.0);     // 3.0
    double r5 = exp2(4.0);               // 16.0
    double r6 = __builtin_exp2(4.0);     // 16.0
    double r7 = cbrt(27.0);              // 3.0
    double r8 = __builtin_cbrt(27.0);    // 3.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const int m = 2;
    double r9 = pow(base, n);            // 32.0
    double r10 = __builtin_pow(base, n); // 32.0
    
    // 3. Mixed integer and floating-point contexts
    // Assign to integer variables - triggers integer-valued check
    int i1 = pow(3.0, 2.0);              // 9
    int i2 = __builtin_pow(3.0, 2.0);    // 9
    int i3 = sqrt(25.0);                 // 5
    int i4 = __builtin_sqrt(25.0);       // 5
    
    // Use in array indexing
    array[(int)sqrt(16.0)] = 1;                     // array[4]
    array[(int)__builtin_sqrt(16.0)] = 2;           // array[4]
    array[(int)pow(2.0, 3.0)] = 3;                  // array[8]
    array[(int)__builtin_pow(2.0, 3.0)] = 4;        // array[8]
    
    // Compare to integer constants
    int cmp1 = (pow(2.0, 4.0) == 16.0);
    int cmp2 = (__builtin_pow(2.0, 4.0) == 16.0);
    int cmp3 = (sqrt(36.0) == 6.0);
    int cmp4 = (__builtin_sqrt(36.0) == 6.0);
    
    // 4. Multiple and nested calls
    double nested1 = pow(sqrt(64.0), log(8.0) / log(2.0));      // 8^3 = 512
    double nested2 = __builtin_pow(__builtin_sqrt(64.0), 
                                   __builtin_log(8.0) / __builtin_log(2.0));
    
    // Single argument functions
    double s1 = exp2(5.0);              // 32.0
    double s2 = __builtin_exp2(5.0);    // 32.0
    double s3 = log(1.0);               // 0.0
    double s4 = __builtin_log(1.0);     // 0.0
    double s5 = sin(0.0);               // 0.0
    double s6 = __builtin_sin(0.0);     // 0.0
    double s7 = cos(0.0);               // 1.0
    double s8 = __builtin_cos(0.0);     // 1.0
    
    // 5. Loop with constant propagation
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; ++iter) {
        // Loop-invariant constant arguments
        double loop_val = pow(2.0, iter + 1.0);  // 2^1, 2^2, 2^3 = 2, 4, 8
        double loop_val2 = __builtin_pow(2.0, iter + 1.0);
        loop_sum += loop_val + loop_val2;
        
        // Use in integer context inside loop
        int idx = (int)sqrt((double)((iter + 1) * (iter + 1) * 4));  // 2, 4, 6
        array[idx % ARRAY_SIZE] += iter;
    }
    
    // 6. Additional integer-valued calls
    double a1 = pow(4.0, 0.5);          // 2.0 (sqrt via pow)
    double a2 = __builtin_pow(4.0, 0.5); // 2.0
    double a3 = log2(8.0);              // 3.0
    double a4 = __builtin_log2(8.0);    // 3.0
    double a5 = log10(100.0);           // 2.0
    double a6 = __builtin_log10(100.0); // 2.0
    
    // 7. Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    checksum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    checksum += r9 + r10 + nested1 + nested2;
    checksum += s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8;
    checksum += a1 + a2 + a3 + a4 + a5 + a6;
    checksum += loop_sum;
    checksum += i1 + i2 + i3 + i4;
    checksum += cmp1 + cmp2 + cmp3 + cmp4;
    
    // Add array values to checksum
    for (int j = 0; j < ARRAY_SIZE; ++j) {
        checksum += array[j];
    }
    
    // Print integer checksum to ensure all computations are used
    printf("Checksum: %d\n", (int)checksum);
    
    return 0;
}
