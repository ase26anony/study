#include <stdio.h>
#include <math.h>

int main(void) {
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    // One-argument integer-valued functions with literals
    double r1 = rint(4.7);           // Should be 5.0
    double t1 = trunc(5.9);          // Should be 5.0
    double f1 = floor(6.2);          // Should be 6.0
    double c1 = ceil(3.1);           // Should be 4.0
    double rd1 = round(7.5);         // Should be 8.0
    
    // One-argument functions with const variables
    double r2 = __builtin_rint(a);   // Should be 11.0
    double t2 = __builtin_trunc(a);  // Should be 10.0
    double f2 = __builtin_floor(a);  // Should be 10.0
    double c2 = __builtin_ceil(a);   // Should be 11.0
    
    // Two-argument functions with literals
    double p1 = pow(2.0, 3.0);       // 2^3 = 8 (integer)
    double fm1 = fmod(9.0, 3.0);     // 9 % 3 = 0 (integer)
    double rem1 = remainder(10.0, 2.0); // Should be 0 (integer)
    
    // Two-argument functions with const variables
    double p2 = __builtin_pow(c, d); // 2^5 = 32 (integer)
    double fm2 = __builtin_fmod(12.0, b); // 12 % 3 = 0 (integer)
    double rem2 = __builtin_remainder(14.0, b); // 14 % 3 = 2 (integer)
    
    // Mixed: one argument with integer constant
    double r3 = rint(4);             // Integer constant
    double f3 = floor(7);            // Integer constant
    
    // Use results in integer contexts to prompt analysis
    int i1 = rint(3.14);             // Implicit conversion to int
    int i2 = trunc(8.9);
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        result += 1;
    }
    
    if (floor(4.2) == 4) {
        result += 2;
    }
    
    // Array indexing with integer-valued real functions
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;        // arr[2] = 5
    arr[(int)round(3.2)] = 7;        // arr[3] = 7
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;   // 8 + 1 = 9
    double d2 = fmod(15.0, 4.0) * 2; // 3 * 2 = 6
    
    // Complex expression mixing multiple integer-valued calls
    double complex_expr = pow(2.0, floor(3.7)) + trunc(4.9) - rint(2.3);
    
    // Use in conditional expressions
    int check = (pow(3.0, 2.0) == 9) ? 1 : 0;  // 3^2 = 9
    
    // Additional two-argument cases with mixed constant types
    double p3 = pow(4, 0.5);         // sqrt(4) = 2 (integer)
    double fm3 = fmod(20.5, 0.5);    // 20.5 % 0.5 = 0 (integer)
    
    // Use nearbyint (another integer-valued function)
    double n1 = nearbyint(6.7);      // Should be 7.0
    double n2 = __builtin_nearbyint(2.3); // Should be 2.0
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)fm1 + (int)rem1;
    result += (int)p2 + (int)fm2 + (int)rem2;
    result += (int)r3 + (int)f3;
    result += i1 + i2;
    result += arr[2] + arr[3];
    result += (int)d1 + (int)d2;
    result += (int)complex_expr;
    result += check;
    result += (int)p3 + (int)fm3;
    result += (int)n1 + (int)n2;
    
    printf("Result: %d\n", result);
    
    return 0;
}
