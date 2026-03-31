#include <stdio.h>
#include <math.h>

int main() {
    // Declare constant variables for use in function calls
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);          // Standard library version
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.8);
    double f2 = __builtin_floor(4.8);
    double c1 = ceil(3.2);
    double c2 = __builtin_ceil(3.2);
    double rd1 = round(6.3);
    double rd2 = __builtin_round(6.3);
    double n1 = nearbyint(7.1);
    double n2 = __builtin_nearbyint(7.1);
    
    // Use in integer context to prompt integer-valued analysis
    int i1 = rint(3.14);           // Implicit conversion
    int i2 = (int)trunc(8.99);     // Explicit cast
    checksum += i1 + i2;
    
    // 2. One-argument functions with const variables
    // These should also trigger arg0 extraction
    double r3 = rint(a);
    double t3 = trunc(a);
    double f3 = floor(a);
    double c3 = ceil(a);
    
    // 3. Two-argument integer-valued functions with literals
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 8.0 - integer result
    double p2 = __builtin_pow(2.0, 5.0); // 32.0 - integer result
    double fm1 = fmod(9.0, 3.0);    // 0.0 - integer result
    double fm2 = __builtin_fmod(12.0, 4.0); // 0.0 - integer result
    double rem1 = remainder(10.0, 2.0); // 0.0 - integer result
    double rem2 = __builtin_remainder(15.0, 5.0); // 0.0 - integer result
    
    // Use in comparisons with integers
    if (trunc(p1) == 8) {
        checksum += 1;
    }
    
    // 4. Two-argument functions with const variables
    double p3 = pow(c, d);          // 2^5 = 32
    double fm3 = fmod(a, b);        // 10.5 % 3.0 = 1.5 (not integer)
    double rem3 = remainder(20.0, b); // 20.0 % 3.0 = 2.0
    
    // 5. Mixed constant and variable arguments
    double p4 = pow(3.0, 2);        // Integer constant as second arg
    double p5 = pow(2, 3.0);        // Integer constant as first arg
    
    // 6. Use in array indexing context
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       // floor(2.8) = 2.0
    arr[(int)ceil(1.1)] = 3;        // ceil(1.1) = 2.0
    checksum += arr[2] + arr[1];
    
    // 7. Use in binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;  // 8.0 + 1 = 9.0
    double d2 = fmod(9.0, 3.0) * 5; // 0.0 * 5 = 0.0
    checksum += (int)d1 + (int)d2;
    
    // 8. Use in switch-like context (via if-else chain)
    double val = round(4.5);        // 5.0
    if ((int)val == 5) {
        checksum += 10;
    } else if ((int)val == 4) {
        checksum += 20;
    }
    
    // 9. Additional calls with implicit integer to double conversion
    double e1 = floor(4);           // Integer constant
    double e2 = pow(2, 5);          // Both integer constants
    double e3 = fmod(9, 3);         // Both integer constants
    
    // 10. Complex expressions involving multiple integer-valued functions
    double complex_expr = pow(floor(4.7), trunc(3.2)) + rint(2.8);
    checksum += (int)complex_expr;
    
    // Aggregate results to prevent dead code elimination
    checksum += (int)(r1 + r2 + t1 + t2 + f1 + f2 + c1 + c2 + rd1 + rd2 + n1 + n2);
    checksum += (int)(p1 + p2 + fm1 + fm2 + rem1 + rem2 + p3 + fm3 + rem3 + p4 + p5);
    checksum += (int)(r3 + t3 + f3 + c3 + e1 + e2 + e3);
    
    printf("Checksum: %d\n", checksum);
    
    // Return value based on checksum to ensure all code paths matter
    return checksum > 100 ? 0 : 1;
}
