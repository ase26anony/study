/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    // Constants for use in function calls
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    
    /* 1. One-argument integer-valued functions with literals */
    // These should trigger arg0 extraction
    double r1 = rint(4.7);          // Standard library function
    double r2 = __builtin_trunc(5.9);  // Built-in version
    double r3 = floor(4.0);         // Constant argument
    double r4 = ceil(3.2);          // Another constant
    double r5 = round(6.5);         // Round to nearest
    
    // Use in integer context to prompt analysis
    int i1 = (int)r1;
    checksum += i1;
    
    /* 2. One-argument functions with const variables */
    // These mix variable usage with constant folding
    double r6 = rint(a);            // rint(10.5) = 10.0 or 11.0 depending on rounding
    double r7 = __builtin_trunc(a); // trunc(10.5) = 10.0
    double r8 = floor(a);           // floor(10.5) = 10.0
    double r9 = ceil(a);            // ceil(10.5) = 11.0
    double r10 = nearbyint(a);      // nearbyint(10.5) = 10.0 or 11.0
    
    // Use in comparison with integer
    if (r7 == 10.0) {
        checksum += 10;
    }
    
    /* 3. Two-argument functions with constant literals */
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 2^3 = 8 (integer)
    double p2 = __builtin_pow(3.0, 2.0); // 3^2 = 9 (integer)
    double f1 = fmod(9.0, 3.0);     // 9 % 3 = 0 (integer)
    double f2 = remainder(10.0, 2.0); // 10 % 2 = 0 (integer)
    
    // Use as array index (prompts integer-valued analysis)
    int arr[10] = {0};
    arr[(int)p1] = 5;               // arr[8] = 5
    checksum += arr[8];
    
    /* 4. Two-argument functions with const variables */
    double p3 = pow(c, d);          // 2^4 = 16 (integer)
    double p4 = __builtin_pow(b, c); // 3^2 = 9 (integer)
    double f3 = fmod(e, b);         // 9 % 3 = 0 (integer)
    double f4 = remainder(e, b);    // 9 % 3 = 0 (integer)
    
    // Use in arithmetic with integers
    double calc = p3 + 4;           // 16 + 4 = 20
    checksum += (int)calc;
    
    /* 5. Mixed usage in expressions */
    // Complex expressions that may require analysis
    double expr1 = pow(2.0, 3.0) + rint(1.7);
    double expr2 = fmod(15.0, 4.0) * trunc(3.8);
    double expr3 = remainder(20.0, 5.0) - ceil(2.3);
    
    // Use in conditional
    if (trunc(expr1) == 9.0) {
        checksum += 9;
    }
    
    /* 6. Direct integer assignments (implicit conversion) */
    int i2 = pow(2.0, 4.0);         // 2^4 = 16, implicit double->int conversion
    int i3 = trunc(7.8);            // trunc(7.8) = 7
    int i4 = floor(6.1);            // floor(6.1) = 6
    int i5 = ceil(5.2);             // ceil(5.2) = 6
    int i6 = round(3.6);            // round(3.6) = 4
    
    checksum += i2 + i3 + i4 + i5 + i6;
    
    /* 7. Function calls in switch context */
    double val = 3.7;
    switch ((int)round(val)) {      // round(3.7) = 4
        case 4:
            checksum += 4;
            break;
        default:
            checksum += 1;
    }
    
    /* 8. Ensure all results are used to prevent optimization */
    double results[] = {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,
                       p1, p2, f1, f2, p3, p4, f3, f4,
                       expr1, expr2, expr3};
    
    // Add a small portion of each to checksum
    for (int j = 0; j < sizeof(results)/sizeof(results[0]); j++) {
        checksum += (int)(results[j] * 0.01);
    }
    
    printf("Checksum: %d\n", checksum);
    
    // Additional test: built-in with integer constant arguments
    // These might be folded even more aggressively
    double test1 = __builtin_pow(2, 5);     // Integer constants
    double test2 = __builtin_fmod(14, 7);   // Integer constants
    printf("Additional tests: %.0f, %.0f\n", test1, test2);
    
    return 0;
}
