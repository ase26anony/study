/* Test program to cover integer_valued_real_p built-in function analysis
 * Lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    // Constant variables to use as arguments
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // __builtin_rint
    double t1 = trunc(5.9);          // __builtin_trunc
    double f1 = floor(6.2);          // __builtin_floor
    double c1 = ceil(3.1);           // __builtin_ceil
    double r2 = round(7.5);          // __builtin_round
    
    // Use in integer context to prompt analysis
    int i1 = (int)rint(3.14);
    result += i1;
    
    // 2. One-argument functions with const variables
    // Mix of standard and __builtin_ versions
    double r3 = __builtin_rint(a);
    double t2 = __builtin_trunc(a);
    double f2 = floor(a);
    double c2 = ceil(a);
    
    // Assignment to int (implicit conversion analysis)
    int i2 = trunc(8.9);
    result += i2;
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);       // 8.0, integer result
    double p2 = __builtin_pow(c, 4.0); // 16.0, integer result
    double fm1 = fmod(9.0, 3.0);     // 0.0, integer result
    double fm2 = __builtin_fmod(e, b); // 0.0, integer result
    double rem1 = remainder(10.0, 5.0); // 0.0, integer result
    double rem2 = __builtin_remainder(14.0, 7.0); // 0.0
    
    // Use in comparisons with integers
    if (pow(3.0, 2.0) == 9) {
        result += 1;
    }
    
    // 4. Array indexing context (requires integer value analysis)
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    result += arr[2];
    
    // 5. Mixed expressions with integer operations
    double mixed = pow(2.0, 5.0) + 1;  // 32 + 1 = 33
    result += (int)mixed;
    
    // 6. nearbyint function (another integer-valued real function)
    double n1 = nearbyint(4.3);
    double n2 = __builtin_nearbyint(7.8);
    result += (int)n1 + (int)n2;
    
    // 7. More complex constant expressions
    const double x = 12.6;
    const double y = 2.0;
    double p3 = pow(x, 0.0);  // Always 1.0, integer
    double f3 = floor(pow(y, 3.0));  // floor(8.0) = 8.0
    
    // Use in conditional
    if (trunc(pow(2.0, 4.0)) == 16) {
        result += 10;
    }
    
    // 8. Functions that may be integer-valued for specific inputs
    double l1 = rint(100.0 * 0.01);  // rint(1.0) = 1.0
    double l2 = trunc(sin(M_PI/2) * 10.0);  // trunc(10.0) = 10.0
    
    // 9. Direct integer conversions
    int i3 = (int)__builtin_round(9.2);
    int i4 = (int)remainder(20.0, 5.0);
    result += i3 + i4;
    
    // 10. Binary operations prompting analysis
    double expr1 = rint(3.7) + 5;      // 4.0 + 5 = 9
    double expr2 = trunc(6.5) * 2;     // 6.0 * 2 = 12
    result += (int)expr1 + (int)expr2;
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional calls to ensure all paths are considered
    // These might be analyzed even if not used
    (void)__builtin_rint(1.1);
    (void)__builtin_floor(2.2);
    (void)__builtin_ceil(3.3);
    (void)__builtin_trunc(4.4);
    (void)__builtin_round(5.5);
    (void)__builtin_nearbyint(6.6);
    (void)__builtin_pow(2.0, 2.0);
    (void)__builtin_fmod(8.0, 2.0);
    (void)__builtin_remainder(15.0, 3.0);
    
    return 0;
}
