#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    // 1. Constant arguments producing integer results
    const double two = 2.0;
    const double three = 3.0;
    const double four = 4.0;
    const double nine = 9.0;
    const double sixteen = 16.0;
    const double twenty_five = 25.0;
    
    // 2. Direct assignments to integer variables
    int i1 = pow(2.0, 3.0);           // 8
    int i2 = __builtin_pow(3.0, 2.0); // 9
    int i3 = sqrt(9.0);               // 3
    int i4 = __builtin_sqrt(16.0);    // 4
    int i5 = exp2(4.0);               // 16
    int i6 = __builtin_exp2(3.0);     // 8
    int i7 = cbrt(27.0);              // 3
    int i8 = __builtin_cbrt(8.0);     // 2
    
    // 3. Symbolic arguments with constant relationships
    const int n = 5;
    const int m = 3;
    double d1 = pow(two, n);          // 32.0
    double d2 = __builtin_pow(three, m); // 27.0
    
    // 4. Use in integer array indexing
    array[(int)sqrt(twenty_five)] = 1;           // array[5]
    array[(int)__builtin_pow(2.0, 3.0)] = 2;     // array[8]
    array[(int)exp2(2.0)] = 3;                   // array[4]
    
    // 5. Compare to integer constants
    int cmp_results = 0;
    if (pow(2.0, 4.0) == 16) cmp_results |= 1;
    if (__builtin_sqrt(36.0) == 6) cmp_results |= 2;
    if (exp2(5.0) == 32) cmp_results |= 4;
    if (__builtin_cbrt(64.0) == 4) cmp_results |= 8;
    
    // 6. Nested calls and complex expressions
    double complex1 = pow(sqrt(sixteen), log(8.0) / log(2.0)); // 4^3 = 64
    double complex2 = __builtin_pow(__builtin_sqrt(81.0), 1.5); // 9^1.5 = 27
    
    // 7. Loop with invariant constants
    int loop_sum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        // These arguments are loop-invariant constants
        double loop_val = pow(2.0, iter + 2.0); // 4, 8, 16
        loop_sum += (int)loop_val;
        
        // Using __builtin_ version inside loop
        double builtin_val = __builtin_pow(3.0, iter + 1.0); // 3, 9, 27
        loop_sum += (int)builtin_val;
    }
    
    // 8. One-argument functions
    int j1 = (int)log(1.0);           // 0
    int j2 = (int)__builtin_log(1.0); // 0
    int j3 = (int)sin(0.0);           // 0
    int j4 = (int)__builtin_sin(0.0); // 0
    int j5 = (int)cos(0.0);           // 1
    int j6 = (int)__builtin_cos(0.0); // 1
    
    // 9. Two-argument functions (atan2)
    int k1 = (int)atan2(0.0, 1.0);           // 0
    int k2 = (int)__builtin_atan2(0.0, 1.0); // 0
    
    // 10. Mixed integer and floating-point contexts
    double mixed = pow(2.0, 3.0) + sqrt(16.0) * exp2(2.0); // 8 + 4*4 = 24
    int int_mixed = (int)mixed;
    
    // 11. Compute checksum to prevent dead code elimination
    int checksum = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8
                 + (int)d1 + (int)d2
                 + array[5] + array[8] + array[4]
                 + cmp_results
                 + (int)complex1 + (int)complex2
                 + loop_sum
                 + j1 + j2 + j3 + j4 + j5 + j6
                 + k1 + k2
                 + int_mixed;
    
    // Use results to prevent optimization
    printf("Checksum: %d\n", checksum);
    
    // Additional print to use array values
    for (int idx = 0; idx < ARRAY_SIZE; ++idx) {
        if (array[idx] != 0) {
            printf("array[%d] = %d\n", idx, array[idx]);
        }
    }
    
    return 0;
}
