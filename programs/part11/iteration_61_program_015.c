#include <stdio.h>
#include <math.h>

int main() {
    // Declare const variables with constant initializers
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);
    double t1 = trunc(5.9);
    double f1 = floor(7.2);
    double c1 = ceil(3.1);
    double rd1 = round(6.5);
    
    // Use __builtin_ versions explicitly
    double r2 = __builtin_rint(8.3);
    double t2 = __builtin_trunc(9.8);
    double f2 = __builtin_floor(a);  // Using const variable
    
    // 2. Two-argument integer-valued functions with constants
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 2^3 = 8 (integer)
    double p2 = pow(c, 5.0);        // 2^5 = 32 (integer)
    double fm1 = fmod(9.0, 3.0);    // 9 % 3 = 0 (integer)
    double fm2 = fmod(10.0, b);     // 10 % 3 = 1 (integer)
    double rem1 = remainder(15.0, 5.0); // 15 rem 5 = 0 (integer)
    
    // __builtin_ versions of two-argument functions
    double p3 = __builtin_pow(3.0, 2.0);  // 3^2 = 9 (integer)
    double fm3 = __builtin_fmod(12.0, 4.0); // 12 % 4 = 0 (integer)
    
    // 3. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);
    int i2 = trunc(7.89);
    int i3 = floor(4.2);
    
    // Comparisons with integers
    if (trunc(5.6) == 5) {
        result += 1;
    }
    
    if (floor(8.9) == 8) {
        result += 2;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;      // arr[2] = 5
    arr[(int)trunc(3.2)] = 7;      // arr[3] = 7
    
    // Binary operations with integers
    double d1 = pow(2.0, 4.0) + 1;      // 16 + 1 = 17
    double d2 = fmod(20.0, 5.0) * 2;    // 0 * 2 = 0
    
    // 4. Mix of function variants and constant types
    
    // Using integer constants (implicit conversion to double)
    double e1 = floor(4);
    double e2 = ceil(6);
    double e3 = pow(2, 5);          // 2^5 = 32
    
    // Using constexpr-like variables (C99 const)
    const double pi = 3.14159;
    double e4 = floor(pi);          // floor(3.14159) = 3
    
    // Complex expressions with multiple integer-valued calls
    double complex_expr = pow(2.0, 3.0) + floor(4.7) - trunc(3.8);
    
    // 5. Additional two-argument functions to ensure coverage
    double p4 = pow(4.0, 0.5);      // sqrt(4) = 2 (integer)
    double fm4 = fmod(16.0, d);     // 16 % 4 = 0 (integer)
    
    // 6. Use results to compute a checksum (prevent dead code elimination)
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    result += (int)p3 + (int)fm3;
    result += i1 + i2 + i3;
    result += (int)d1 + (int)d2;
    result += (int)e1 + (int)e2 + (int)e3 + (int)e4;
    result += (int)complex_expr;
    result += (int)p4 + (int)fm4;
    
    // Add array elements
    for (int i = 0; i < 10; i++) {
        result += arr[i];
    }
    
    printf("Result: %d\n", result);
    
    return 0;
}
