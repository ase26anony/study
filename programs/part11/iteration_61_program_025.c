#include <stdio.h>
#include <math.h>

int main() {
    // Declare constant variables for use as arguments
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_rint(4.7); // Built-in version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.8);
    double f2 = __builtin_floor(4.8);
    double c1 = ceil(3.2);
    double c2 = __builtin_ceil(3.2);
    double rnd1 = round(6.3);
    double rnd2 = __builtin_round(6.3);
    double n1 = nearbyint(7.1);
    double n2 = __builtin_nearbyint(7.1);
    
    // 2. One-argument functions with const variables
    // These test constant propagation
    double r3 = rint(a);
    double t3 = trunc(a);
    double f3 = floor(a);
    double c3 = ceil(a);
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);           // 2^3 = 8 (integer)
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(c, d);               // 2^5 = 32 (integer)
    double fm1 = fmod(9.0, 3.0);         // 9 % 3 = 0 (integer)
    double fm2 = __builtin_fmod(9.0, 3.0);
    double fm3 = fmod(10.0, b);          // 10 % 3 = 1 (integer)
    double rem1 = remainder(10.0, 3.0);
    double rem2 = __builtin_remainder(10.0, 3.0);
    
    // 4. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);      // Should be 3
    int i2 = trunc(8.99);     // Should be 8
    int i3 = floor(7.2);      // Should be 7
    int i4 = ceil(6.1);       // Should be 7
    int i5 = (int)pow(3.0, 2.0); // 3^2 = 9
    
    // Comparisons with integers
    if (trunc(4.8) == 4) {
        result += 1;
    }
    if (floor(5.9) == 5) {
        result += 2;
    }
    if (ceil(3.1) == 4) {
        result += 4;
    }
    if (rint(2.5) == 2 || rint(2.5) == 3) { // Depends on rounding mode
        result += 8;
    }
    
    // Array indexing
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;  // arr[2] = 5
    arr[(int)trunc(3.2)] = 7;  // arr[3] = 7
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;      // 8 + 1 = 9
    double d2 = fmod(15.0, 4.0) * 2;    // 3 * 2 = 6
    double d3 = remainder(20.0, 6.0) - 2; // 2 - 2 = 0
    
    // 5. Use results in arithmetic to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)t1 + (int)t2;
    result += (int)f1 + (int)f2 + (int)c1 + (int)c2;
    result += (int)rnd1 + (int)rnd2 + (int)n1 + (int)n2;
    result += (int)p1 + (int)p2 + (int)p3;
    result += (int)fm1 + (int)fm2 + (int)fm3;
    result += (int)rem1 + (int)rem2;
    result += i1 + i2 + i3 + i4 + i5;
    result += arr[2] + arr[3];
    result += (int)d1 + (int)d2 + (int)d3;
    
    printf("Result: %d\n", result);
    
    // Additional complex expression to encourage analysis
    // This uses nested calls and mixed operations
    double complex_expr = floor(pow(2.0, 3.0)) + ceil(fmod(17.0, 5.0)) * trunc(4.7);
    printf("Complex: %f\n", complex_expr);
    
    return 0;
}
