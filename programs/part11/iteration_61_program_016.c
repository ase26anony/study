#include <stdio.h>
#include <math.h>

int main(void) {
    // Use const variables to encourage constant folding
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_trunc(5.9); // Builtin version
    double r3 = floor(4.0);          // Integer constant as double
    double r4 = ceil(a);             // Using const variable
    double r5 = __builtin_round(3.14);
    double r6 = nearbyint(7.8);
    
    // 2. Two-argument integer-valued functions with constants
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);       // 2^3 = 8 (integer)
    double p2 = __builtin_pow(c, 4.0); // 2^4 = 16 (integer)
    double f1 = fmod(9.0, 3.0);      // 9 % 3 = 0 (integer)
    double f2 = __builtin_remainder(10.0, 2.0); // 10 % 2 = 0
    
    // 3. Use results in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);
    int i2 = trunc(5.9);
    int i3 = (int)floor(2.8);
    
    // Comparisons with integers
    if (trunc(4.7) == 4) {
        result += 1;
    }
    
    if (__builtin_ceil(3.2) == 4) {
        result += 2;
    }
    
    // Array indexing (requires integer result)
    int arr[10];
    arr[(int)floor(2.8)] = 42;
    
    // Binary operations with integers
    double calc1 = pow(3.0, 2.0) + 5;    // 9 + 5 = 14
    double calc2 = fmod(15.0, 5.0) * 2;  // 0 * 2 = 0
    
    // 4. Mix of function variants with different argument patterns
    // Some with both arguments constant, some with one constant
    
    // Both constant literals
    double m1 = __builtin_pow(5.0, 2.0);  // 25
    
    // First constant, second from variable
    const double base = 3.0;
    double m2 = pow(base, 2.0);          // 9
    
    // Using constexpr-like computation
    const double pi = 3.141592653589793;
    double m3 = floor(pi);               // 3
    
    // Complex expression that might be folded
    double complex_expr = pow(2.0, floor(3.7)) + ceil(4.3) - trunc(5.6);
    
    // 5. Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)r3 + (int)r4;
    result += (int)p1 + (int)p2 + (int)f1 + (int)f2;
    result += i1 + i2 + i3;
    result += arr[2];
    result += (int)calc1 + (int)calc2;
    result += (int)m1 + (int)m2 + (int)m3;
    result += (int)complex_expr;
    
    printf("Result: %d\n", result);
    
    return 0;
}
