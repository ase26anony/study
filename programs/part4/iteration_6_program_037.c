#include <math.h>
#include <stdio.h>

int main() {
    // 1. Declare variables with constant values
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 3;
    const double exp_dbl = 3.0;
    
    int int_result = 0;
    double dbl_result = 0.0;
    int checksum = 0;
    
    // 2. Direct assignments with constant arguments
    // Using standard library names
    int i1 = pow(2.0, 3.0);           // 8
    int i2 = sqrt(9.0);               // 3
    int i3 = exp2(4.0);               // 16
    int i4 = cbrt(27.0);              // 3
    int i5 = log2(8.0);               // 3
    
    // Using __builtin_ versions
    int i6 = __builtin_pow(3.0, 2.0); // 9
    int i7 = __builtin_sqrt(16.0);    // 4
    int i8 = __builtin_exp2(5.0);     // 32
    int i9 = __builtin_cbrt(64.0);    // 4
    int i10 = __builtin_log2(32.0);   // 5
    
    // 3. Symbolic arguments with constant relationships
    int i11 = pow(base, exp_dbl);     // 8.0
    int i12 = sqrt((double)(n * n));  // sqrt(25.0) = 5
    
    // 4. Use in array indexing
    int array[100] = {0};
    array[(int)pow(2.0, 2.0)] = 1;    // array[4]
    array[(int)__builtin_sqrt(36.0)] = 2; // array[6]
    
    // 5. Comparisons with integer constants
    if (pow(2.0, 3.0) == 8) {
        int_result += 1;
    }
    
    if (__builtin_sqrt(49.0) == 7) {
        int_result += 2;
    }
    
    // 6. Nested calls and combined expressions
    dbl_result = pow(sqrt(16.0), log(8.0) / log(2.0)); // 4^3 = 64
    int i13 = (int)dbl_result;
    
    // More complex nested expressions
    double val1 = exp2(log2(64.0) / 2.0); // sqrt(64) via exp2(log2/2)
    double val2 = cbrt(pow(2.0, 9.0));    // cbrt(512) = 8
    int i14 = (int)val1 + (int)val2;      // 8 + 8 = 16
    
    // 7. Loop with invariant constants (for conditional constant propagation)
    int loop_sum = 0;
    for (int iter = 0; iter < 3; iter++) {
        // These arguments are loop-invariant
        double power = pow(2.0, (double)iter);
        double root = sqrt((double)(iter * iter * 4));
        
        // Use in integer context
        loop_sum += (int)power + (int)root;
    }
    
    // 8. One-argument functions
    int i15 = exp2(6.0);      // 64
    int i16 = sqrt(81.0);     // 9
    int i17 = cbrt(125.0);    // 5
    int i18 = log2(256.0);    // 8
    
    // 9. Two-argument functions (besides pow)
    int i19 = (int)__builtin_pow(4.0, 1.5);  // 4^1.5 = 8
    // Note: atan2 doesn't typically produce integer results for integer inputs
    
    // 10. Trigonometric functions with special cases
    int i20 = (int)sin(0.0);      // 0
    int i21 = (int)cos(0.0);      // 1
    int i22 = (int)__builtin_sin(3.14159265358979323846); // ~0
    
    // 11. Calculate checksum (prevent dead code elimination)
    checksum = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
               i11 + i12 + i13 + i14 + i15 + i16 + i17 + i18 + i19 +
               i20 + i21 + i22 + int_result + loop_sum;
    
    // Use array elements
    checksum += array[4] + array[6];
    
    printf("Checksum: %d\n", checksum);
    
    // Additional test: mixed integer/float context with variable
    const int m = 4;
    double temp = pow(2.0, (double)m); // 16.0
    if ((int)temp == 16) {
        printf("Additional check passed\n");
    }
    
    return 0;
}
