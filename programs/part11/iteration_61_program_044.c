#include <stdio.h>
#include <math.h>

int main() {
    // Declare constant variables for use as arguments
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double t1 = trunc(5.9);          // trunc(5.9) = 5.0
    double f1 = floor(8.2);          // floor(8.2) = 8.0
    double c1 = ceil(3.1);           // ceil(3.1) = 4.0
    double r2 = round(6.5);          // round(6.5) = 7.0
    
    // Use __builtin_ versions explicitly
    double n1 = __builtin_nearbyint(2.3);  // nearbyint(2.3) = 2.0
    
    // 2. One-argument functions with const variables
    double r3 = rint(a);             // rint(10.5) = 10.0 or 11.0 (depends on rounding)
    double t2 = trunc(a);            // trunc(10.5) = 10.0
    double f2 = floor(a);            // floor(10.5) = 10.0
    double c2 = ceil(a);             // ceil(10.5) = 11.0
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);       // 2^3 = 8 (integer)
    double p2 = pow(c, d);           // 2^4 = 16 (integer)
    double fm1 = fmod(9.0, 3.0);     // 9 % 3 = 0 (integer)
    double fm2 = fmod(10.0, b);      // 10 % 3 = 1.0
    double rem1 = remainder(15.0, 5.0); // 15 rem 5 = 0 (integer)
    
    // Use __builtin_ versions for two-argument functions
    double p3 = __builtin_pow(3.0, 2.0);  // 3^2 = 9 (integer)
    double fm3 = __builtin_fmod(12.0, 4.0); // 12 % 4 = 0 (integer)
    
    // 4. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);             // Should be 3
    int i2 = trunc(7.89);            // Should be 7
    int i3 = floor(4.0);             // Should be 4
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    
    if (floor(8.2) == 8) {
        checksum += 2;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;        // arr[2] = 5
    checksum += arr[2];
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;   // 8 + 1 = 9
    checksum += (int)d1;
    
    double d2 = fmod(9.0, 3.0) * 2;  // 0 * 2 = 0
    checksum += (int)d2;
    
    // 5. Complex expressions mixing different functions
    double complex_expr = floor(rint(trunc(ceil(4.3))));
    checksum += (int)complex_expr;
    
    // 6. Use results in switch-like logic (without actual switch to avoid case label issues)
    int val = (int)round(3.6);       // Should be 4
    if (val == 4) {
        checksum += 10;
    }
    
    // 7. More two-argument function tests
    // Test with integer constants that convert to double
    double p4 = pow(2, 5);           // 2^5 = 32 (integer)
    double fm4 = fmod(20, 6);        // 20 % 6 = 2 (integer)
    
    checksum += (int)p4;
    checksum += (int)fm4;
    
    // 8. Mixed constant and non-constant arguments
    // (still constant due to const variables)
    double p5 = pow(c, 3.0);         // 2^3 = 8
    double fm5 = fmod(15.0, b);      // 15 % 3 = 0
    
    checksum += (int)p5;
    checksum += (int)fm5;
    
    // 9. Use all computed values to avoid dead code elimination
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)n1 + (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)p3 + (int)fm3 + i1 + i2 + i3;
    
    // Print the checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
