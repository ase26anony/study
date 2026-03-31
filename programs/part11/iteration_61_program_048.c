#include <stdio.h>
#include <math.h>

int main() {
    // Declare constant variables for use in function calls
    const double a = 10.5;
    const double b = 3.14;
    const double c = 7.0;
    const double d = 2.5;
    const double e = 9.0;
    const double f = 3.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);          // Standard library version
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(8.2);
    double f2 = __builtin_floor(8.2);
    double c1 = ceil(6.1);
    double c2 = __builtin_ceil(6.1);
    double rd1 = round(3.5);
    double rd2 = __builtin_round(3.5);
    double n1 = nearbyint(2.3);
    double n2 = __builtin_nearbyint(2.3);
    
    // 2. One-argument functions with const variables
    // These should also trigger arg0 extraction
    double r3 = rint(a);
    double t3 = trunc(b);
    double f3 = floor(c);
    double c3 = ceil(d);
    
    // 3. Two-argument integer-valued functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 2^3 = 8 (integer)
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(3.0, 2.0);      // 3^2 = 9 (integer)
    double p4 = pow(e, 1.0);        // e^1 = e (not integer)
    
    double fm1 = fmod(9.0, 3.0);    // 9 % 3 = 0 (integer)
    double fm2 = __builtin_fmod(9.0, 3.0);
    double fm3 = fmod(10.0, 2.0);   // 10 % 2 = 0 (integer)
    double fm4 = fmod(e, f);        // 9 % 3 = 0 (integer)
    
    double rem1 = remainder(15.0, 5.0);  // 15 rem 5 = 0 (integer)
    double rem2 = __builtin_remainder(15.0, 5.0);
    double rem3 = remainder(20.0, 4.0);  // 20 rem 4 = 0 (integer)
    
    // 4. Use results in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);           // Should analyze rint's integer-valued property
    int i2 = trunc(7.89);
    int i3 = floor(4.2);
    int i4 = ceil(3.1);
    int i5 = (int)pow(2.0, 4.0);   // 2^4 = 16
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    
    if (floor(8.8) == 8) {
        checksum += 2;
    }
    
    if (ceil(3.2) == 4) {
        checksum += 4;
    }
    
    if (fmod(12.0, 4.0) == 0) {
        checksum += 8;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;      // floor(2.8) = 2
    arr[(int)ceil(1.1)] = 7;       // ceil(1.1) = 2
    arr[(int)rint(3.5)] = 9;       // rint(3.5) = 4 (with default rounding)
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;     // 8 + 1 = 9
    double d2 = fmod(15.0, 4.0) * 2;   // 3 * 2 = 6
    double d3 = trunc(6.7) - 3;        // 6 - 3 = 3
    
    // 5. Complex expressions mixing multiple integer-valued functions
    double complex1 = floor(pow(2.0, 3.0)) + ceil(trunc(4.2));
    double complex2 = rint(fmod(17.0, 5.0)) * trunc(3.14);
    
    // 6. Use integer constants that get converted to double
    double g1 = floor(4);          // integer constant 4
    double g2 = pow(2, 5);         // integer constants 2 and 5
    double g3 = fmod(9, 3);        // integer constants 9 and 3
    
    // 7. Compute a checksum to prevent dead code elimination
    checksum += (int)r1 + (int)r2 + (int)t1 + (int)t2;
    checksum += (int)f1 + (int)f2 + (int)c1 + (int)c2;
    checksum += (int)rd1 + (int)rd2 + (int)n1 + (int)n2;
    checksum += (int)p1 + (int)p2 + (int)p3;
    checksum += (int)fm1 + (int)fm2 + (int)fm3 + (int)fm4;
    checksum += (int)rem1 + (int)rem2 + (int)rem3;
    checksum += i1 + i2 + i3 + i4 + i5;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)complex1 + (int)complex2;
    checksum += (int)g1 + (int)g2 + (int)g3;
    
    // Add array values
    for (int j = 0; j < 10; j++) {
        checksum += arr[j];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
