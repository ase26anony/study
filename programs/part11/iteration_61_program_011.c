/* Test program to cover integer_valued_real_p built-in function analysis
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int result = 0;
    
    // Constant variables to use as arguments
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const int d = 4;
    
    /* 1. One-argument integer-valued functions with literals */
    // Using __builtin_ prefixed versions
    double r1 = __builtin_rint(4.7);      // Should return 5.0
    double r2 = __builtin_trunc(5.9);     // Should return 5.0
    double r3 = __builtin_floor(7.2);     // Should return 7.0
    double r4 = __builtin_ceil(6.1);      // Should return 7.0
    double r5 = __builtin_round(3.5);     // Should return 4.0
    double r6 = __builtin_nearbyint(8.3); // Should return 8.0
    
    // Using standard library functions (will be recognized as built-ins with -fbuiltin)
    double r7 = rint(9.8);    // Should return 10.0
    double r8 = trunc(2.3);   // Should return 2.0
    double r9 = floor(a);     // Should return 10.0 (using const variable)
    
    /* 2. Two-argument integer-valued functions with constant arguments */
    // pow with integer result
    double p1 = __builtin_pow(2.0, 3.0);      // 2^3 = 8 (integer)
    double p2 = pow(c, 3.0);                  // 2^3 = 8 (integer)
    double p3 = __builtin_pow(3.0, 2);        // 3^2 = 9 (integer, with int arg)
    
    // fmod with remainder 0 (integer result)
    double f1 = __builtin_fmod(9.0, 3.0);     // 9 % 3 = 0 (integer)
    double f2 = fmod(12.0, 4.0);              // 12 % 4 = 0 (integer)
    double f3 = __builtin_fmod(20.0, b);      // 20 % 3.0 = 2.0 (using const variable)
    
    // remainder with integer result
    double rem1 = __builtin_remainder(15.0, 5.0);  // 15 rem 5 = 0 (integer)
    double rem2 = remainder(25.0, 5.0);            // 25 rem 5 = 0 (integer)
    
    /* 3. Embed calls in contexts that require integer-valued analysis */
    
    // Assignments to integer types (implicit conversion analysis)
    int i1 = __builtin_rint(3.14);      // Should become 3
    int i2 = (int)trunc(8.9);           // Should become 8
    int i3 = floor(4.0);                // Should become 4
    
    // Comparisons with integers
    if (__builtin_ceil(2.1) == 3) {
        result += 1;
    }
    
    if (round(5.5) == 6) {
        result += 2;
    }
    
    // Array indexing (requires integer result)
    int arr[10] = {0};
    arr[(int)__builtin_floor(2.8)] = 5;      // arr[2] = 5
    arr[(int)trunc(3.2)] = 10;               // arr[3] = 10
    
    // Binary operations with integers
    double calc1 = __builtin_pow(2.0, 4.0) + 1;      // 16 + 1 = 17
    double calc2 = __builtin_fmod(18.0, 6.0) * 2;    // 0 * 2 = 0
    
    // Complex expression requiring integer-valued analysis
    int complex = (int)(__builtin_rint(3.7) + __builtin_trunc(2.3)) / 2;
    
    /* 4. Use results in computations to prevent dead code elimination */
    result += (int)r1 + (int)r2 + (int)r3;
    result += (int)r4 + (int)r5 + (int)r6;
    result += (int)r7 + (int)r8 + (int)r9;
    
    result += (int)p1 + (int)p2 + (int)p3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)rem1 + (int)rem2;
    
    result += i1 + i2 + i3;
    result += arr[2] + arr[3];
    result += (int)calc1 + (int)calc2;
    result += complex;
    
    printf("Result: %d\n", result);
    
    // Additional calls in return expression context
    return (int)(__builtin_floor(100.5) + __builtin_ceil(99.5)) % 100;
}
