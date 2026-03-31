#include <stdio.h>
#include <math.h>

int main(void) {
    // Declare const variables with constant initializers
    const double a = 10.5;
    const double b = 3.14;
    const double c = 9.0;
    const double d = 3.0;
    const double e = 2.0;
    const double f = 5.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_rint(4.7); // Built-in version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.2);
    double f2 = __builtin_floor(4.2);
    double c1 = ceil(4.2);
    double c2 = __builtin_ceil(4.2);
    double r3 = round(3.6);
    double r4 = __builtin_round(3.6);
    double n1 = nearbyint(2.3);
    double n2 = __builtin_nearbyint(2.3);
    
    // 2. One-argument functions with const variables
    // These test constant propagation
    double r5 = rint(a);
    double t3 = trunc(b);
    double f3 = floor(a);
    double c3 = ceil(b);
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);           // 2^3 = 8 (integer)
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(e, f);               // 2^5 = 32 (integer)
    double fm1 = fmod(9.0, 3.0);         // 9 % 3 = 0 (integer)
    double fm2 = __builtin_fmod(9.0, 3.0);
    double fm3 = fmod(c, d);             // 9 % 3 = 0
    double rem1 = remainder(10.0, 2.0);  // 10 % 2 = 0
    double rem2 = __builtin_remainder(10.0, 2.0);
    
    // 4. Mixed integer-valued functions
    // pow with integer result from non-integer base
    double p4 = pow(4.0, 0.5);           // sqrt(4) = 2 (integer)
    double p5 = pow(27.0, 1.0/3.0);      // cube root of 27 = 3 (integer)
    
    // 5. Use results in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);                 // Should be 3
    int i2 = trunc(7.89);                // Should be 7
    int i3 = floor(8.99);                // Should be 8
    int i4 = ceil(6.01);                 // Should be 7
    int i5 = (int)pow(3.0, 2.0);         // 3^2 = 9
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    if (floor(4.2) == 4) {
        checksum += 2;
    }
    if (ceil(4.2) == 5) {
        checksum += 4;
    }
    if (rint(3.5) == 4) {               // Round half away from zero
        checksum += 8;
    }
    if (fmod(9.0, 3.0) == 0.0) {
        checksum += 16;
    }
    if (pow(2.0, 3.0) == 8.0) {
        checksum += 32;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 1;           // arr[2] = 1
    arr[(int)trunc(3.2)] = 2;           // arr[3] = 2
    arr[(int)rint(4.6)] = 3;            // arr[5] = 3
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;      // 8 + 1 = 9
    double d2 = fmod(10.0, 3.0) * 2;    // 1 * 2 = 2
    double d3 = trunc(6.7) - 3;         // 6 - 3 = 3
    
    // Complex expressions combining multiple integer-valued functions
    double complex1 = floor(4.7) + ceil(3.2) - trunc(2.9);
    double complex2 = pow(2.0, floor(3.8)) + fmod(10.0, ceil(2.1));
    
    // Use results to prevent dead code elimination
    checksum += (int)r1 + (int)r2 + (int)t1 + (int)t2;
    checksum += (int)f1 + (int)f2 + (int)c1 + (int)c2;
    checksum += (int)r3 + (int)r4 + (int)n1 + (int)n2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2;
    checksum += i1 + i2 + i3 + i4 + i5;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)complex1 + (int)complex2;
    
    // Add array values
    for (int i = 0; i < 10; i++) {
        checksum += arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    // Additional calls in return expression context
    return (int)(rint(checksum / 10.0) + trunc(checksum / 20.0));
}
