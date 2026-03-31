#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    // 1. Constant arguments producing integer results
    const double base = 2.0;
    const double exponent = 3.0;
    
    // Direct calls with constant arguments
    double r1 = pow(2.0, 3.0);           // 8.0
    double r2 = sqrt(25.0);              // 5.0
    double r3 = exp2(4.0);               // 16.0
    double r4 = cbrt(27.0);              // 3.0
    double r5 = log2(8.0);               // 3.0
    
    // __builtin_ versions
    double r6 = __builtin_pow(3.0, 2.0); // 9.0
    double r7 = __builtin_sqrt(36.0);    // 6.0
    double r8 = __builtin_exp2(5.0);     // 32.0
    
    // 2. Symbolic arguments with constant relationships
    const int n = 5;
    const int m = 2;
    
    double r9 = pow(base, n);            // 32.0
    double r10 = sqrt(n * n);            // 5.0
    double r11 = exp2(m + 1);            // 8.0
    
    // 3. Assign to integer variables (triggers integer-valued check)
    int i1 = pow(2.0, 3.0);              // Should be 8
    int i2 = sqrt(49.0);                 // Should be 7
    int i3 = exp2(3.0);                  // Should be 8
    int i4 = __builtin_pow(4.0, 1.5);    // Should be 8 (4^(3/2) = 8)
    
    // 4. Use in array indexing
    array[(int)sqrt(16.0)] = 1;          // array[4] = 1
    array[(int)pow(2.0, 2.0)] = 2;       // array[4] = 2
    array[(int)__builtin_exp2(3.0)] = 3; // array[8] = 3
    
    // 5. Compare to integer constants
    int cmp1 = (pow(3.0, 2.0) == 9.0);
    int cmp2 = (sqrt(100.0) == 10.0);
    int cmp3 = (__builtin_exp2(4.0) == 16.0);
    
    // 6. Place calls in a loop with fixed iteration count
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        // Loop-invariant constants
        const double loop_base = 2.0;
        const int loop_exp = i + 1;
        
        // These should be foldable despite being in loop
        loop_sum += pow(loop_base, loop_exp);  // 2^1 + 2^2 + 2^3 = 2 + 4 + 8
        loop_sum += sqrt(loop_exp * loop_exp); // sqrt(1) + sqrt(4) + sqrt(9)
    }
    
    // 7. Nested calls and complex expressions
    double complex1 = pow(sqrt(16.0), log(8.0) / log(2.0));  // 4^3 = 64
    double complex2 = exp2(log2(32.0) / 2.0);                // sqrt(32) ≈ 5.65685
    double complex3 = cbrt(pow(2.0, 9.0));                   // cube root of 512 = 8
    
    // 8. Single-argument functions
    double s1 = sin(0.0);      // 0.0 (integer result)
    double s2 = cos(0.0);      // 1.0 (integer result)
    double s3 = __builtin_log(1.0);  // 0.0 (integer result)
    double s4 = __builtin_log10(100.0); // 2.0 (integer result)
    
    // 9. Two-argument functions
    double t1 = atan2(0.0, 1.0);    // 0.0 (integer result)
    double t2 = pow(1.0, 100.0);    // 1.0 (integer result)
    double t3 = __builtin_pow(0.0, 2.0);  // 0.0 (integer result)
    
    // 10. Mixed integer context usage
    int mixed1 = (int)(pow(2.0, 4.0) + sqrt(9.0));  // 16 + 3 = 19
    int mixed2 = (int)(exp2(3.0) * cbrt(8.0));      // 8 * 2 = 16
    
    // Prevent dead code elimination by computing checksum
    int checksum = 0;
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
    checksum += (int)r6 + (int)r7 + (int)r8 + (int)r9 + (int)r10 + (int)r11;
    checksum += i1 + i2 + i3 + i4;
    checksum += cmp1 + cmp2 + cmp3;
    checksum += (int)loop_sum;
    checksum += (int)complex1 + (int)complex2 + (int)complex3;
    checksum += (int)s1 + (int)s2 + (int)s3 + (int)s4;
    checksum += (int)t1 + (int)t2 + (int)t3;
    checksum += mixed1 + mixed2;
    
    // Also sum array elements
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    // Expected checksum calculation:
    // r1-r11: 8+5+16+3+3+9+6+32+32+5+8 = 127
    // i1-i4: 8+7+8+8 = 31
    // cmp1-cmp3: 1+1+1 = 3
    // loop_sum: (2+4+8)+(1+2+3) = 14+6 = 20
    // complex1-complex3: 64+5+8 = 77
    // s1-s4: 0+1+0+2 = 3
    // t1-t3: 0+1+0 = 1
    // mixed1-mixed2: 19+16 = 35
    // array: array[4]=2, array[8]=3, others=0 → 5
    // Total: 127+31+3+20+77+3+1+35+5 = 302
    
    return 0;
}
