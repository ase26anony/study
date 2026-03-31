#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    // 1. Declare mix of integer and floating-point variables
    const int n = 5;
    const double base = 2.0;
    double fp_results[10];
    int int_results[10];
    int idx = 0;
    
    // 2. Computations with constant arguments producing integer results
    
    // pow with integer result (two arguments)
    double r1 = pow(3.0, 2.0);  // 9.0
    int i1 = (int)r1;  // Triggers integer-valued check
    array[i1 % ARRAY_SIZE] = i1;
    
    // __builtin_pow version
    double r2 = __builtin_pow(2.0, 4.0);  // 16.0
    int i2 = (int)r2;
    array[i2 % ARRAY_SIZE] += i2;
    
    // sqrt of perfect square (one argument)
    double r3 = sqrt(25.0);  // 5.0
    int i3 = (int)r3;
    array[i3 % ARRAY_SIZE] = i3;
    
    // __builtin_sqrt version
    double r4 = __builtin_sqrt(36.0);  // 6.0
    int i4 = (int)r4;
    array[i4 % ARRAY_SIZE] += i4;
    
    // exp2 with integer exponent (one argument)
    double r5 = exp2(3.0);  // 8.0
    int i5 = (int)r5;
    array[i5 % ARRAY_SIZE] = i5;
    
    // __builtin_exp2 version
    double r6 = __builtin_exp2(4.0);  // 16.0
    int i6 = (int)r6;
    array[i6 % ARRAY_SIZE] += i6;
    
    // log2 of power of 2 (one argument)
    double r7 = log2(8.0);  // 3.0
    int i7 = (int)r7;
    array[i7 % ARRAY_SIZE] = i7;
    
    // cbrt of perfect cube (one argument)
    double r8 = cbrt(27.0);  // 3.0
    int i8 = (int)r8;
    array[i8 % ARRAY_SIZE] += i8;
    
    // sin/cos with special arguments that produce integer-like results
    double r9 = sin(0.0);  // 0.0
    int i9 = (int)r9;
    array[i9 % ARRAY_SIZE] = i9;
    
    // 3. Loop with fixed iterations and invariant arguments
    double loop_sum = 0.0;
    for (int iter = 0; iter < 3; ++iter) {
        // pow with loop-invariant constant
        double val = pow(base, (double)iter);  // 1.0, 2.0, 4.0
        loop_sum += val;
        
        // Use in integer context inside loop
        int idx_val = (int)__builtin_sqrt(val * val);  // Should fold to iter
        if (idx_val < ARRAY_SIZE) {
            array[idx_val] += iter;
        }
    }
    
    // 4. Nested calls and combined expressions
    double nested = pow(sqrt(16.0), log(8.0) / log(2.0));  // 4^3 = 64
    int i_nested = (int)nested;
    array[i_nested % ARRAY_SIZE] = i_nested;
    
    // Mixed one and two argument calls in expression
    double combined = __builtin_pow(__builtin_sqrt(81.0), 2.0) + 
                     __builtin_exp2(__builtin_log2(4.0));  // 9^2 + 4 = 85
    int i_combined = (int)combined;
    array[i_combined % ARRAY_SIZE] += i_combined;
    
    // 5. Comparisons with integer constants
    if (pow(2.0, 3.0) == 8) {
        array[0] += 1;
    }
    
    if (__builtin_sqrt(100.0) == 10.0) {
        array[1] += 2;
    }
    
    // 6. Array indexing with math function results
    array[(int)pow(2.0, 2.0)] = 99;  // array[4] = 99
    array[(int)__builtin_sqrt(9.0)] = 100;  // array[3] = 100
    
    // 7. Compute checksum
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        checksum += array[i];
    }
    
    // Add floating results converted to int
    checksum += (int)r1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
    checksum += (int)loop_sum + i_nested + i_combined;
    
    printf("Checksum: %d\n", checksum);
    
    // Prevent dead code elimination
    volatile int dummy = checksum;
    
    return 0;
}
