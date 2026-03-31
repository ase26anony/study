/* Test program to cover integer_valued_real_p built-in function analysis
 * Lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int result = 0;
    
    // Constants for use in function calls
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    const double f = 3.0;
    
    /* One-argument integer-valued real functions */
    // Using __builtin_ prefixed versions
    double r1 = __builtin_rint(4.7);           // Should be 5.0
    double r2 = __builtin_trunc(5.9);          // Should be 5.0
    double r3 = __builtin_floor(a);            // floor(10.5) = 10.0
    double r4 = __builtin_ceil(3.2);           // ceil(3.2) = 4.0
    double r5 = __builtin_round(6.5);          // round(6.5) = 7.0
    double r6 = __builtin_nearbyint(8.3);      // nearbyint(8.3) = 8.0
    
    // Using standard library functions (will be recognized as built-ins with -fbuiltin)
    double r7 = rint(2.3);                     // Should be 2.0
    double r8 = trunc(7.8);                    // Should be 7.0
    double r9 = floor(4.0);                    // floor(4.0) = 4.0
    double r10 = ceil(5.1);                    // ceil(5.1) = 6.0
    
    /* Two-argument integer-valued real functions */
    // pow with integer result
    double p1 = __builtin_pow(c, 3.0);         // 2^3 = 8.0 (integer)
    double p2 = pow(3.0, 2.0);                 // 3^2 = 9.0 (integer)
    double p3 = __builtin_pow(5.0, 0.0);       // 5^0 = 1.0 (integer)
    
    // fmod with exact division
    double fm1 = __builtin_fmod(e, f);         // fmod(9.0, 3.0) = 0.0 (integer)
    double fm2 = fmod(10.0, 5.0);              // fmod(10.0, 5.0) = 0.0 (integer)
    double fm3 = __builtin_fmod(12.0, 4.0);    // fmod(12.0, 4.0) = 0.0 (integer)
    
    // remainder with exact division
    double rem1 = remainder(8.0, 2.0);         // remainder(8.0, 2.0) = 0.0 (integer)
    double rem2 = __builtin_remainder(15.0, 5.0); // remainder(15.0, 5.0) = 0.0 (integer)
    
    /* Use results in contexts that require integer-valued analysis */
    
    // 1. Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);                       // Should be 3
    int i2 = trunc(9.99);                      // Should be 9
    int i3 = floor(6.7);                       // Should be 6
    int i4 = (int)__builtin_ceil(2.3);         // Should be 3
    
    // 2. Comparisons with integers
    if (trunc(4.8) == 4) {
        result += 1;
    }
    
    if (__builtin_floor(7.2) == 7) {
        result += 2;
    }
    
    // 3. Array indexing
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;                  // arr[2] = 5
    arr[(int)__builtin_round(3.2)] = 10;       // arr[3] = 10
    
    // 4. Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;             // 8.0 + 1 = 9.0
    double d2 = __builtin_fmod(12.0, 6.0) * 2; // 0.0 * 2 = 0.0
    double d3 = remainder(10.0, 5.0) - 0;      // 0.0 - 0 = 0.0
    
    // 5. Mixed expressions
    double mixed = rint(3.7) + trunc(2.3) + floor(4.9) + ceil(5.1);
    mixed += __builtin_pow(2.0, 4.0);          // Add 16.0
    mixed += fmod(20.0, 5.0);                  // Add 0.0
    
    // 6. Conditional expressions based on integer-valued results
    int choice = (int)round(mixed / 10.0);
    switch (choice) {
        case 3:
            result += 4;
            break;
        case 4:
            result += 8;
            break;
        default:
            result += 16;
    }
    
    // 7. Complex nested calls
    double nested = floor(pow(2.0, 3.0));      // floor(8.0) = 8.0
    nested = ceil(fmod(17.0, 4.0));            // ceil(1.0) = 1.0
    
    // 8. Use constexpr-like variables (C99 compound literal as const)
    const double pi = 3.141592653589793;
    double circle_area = floor(pi * pow(3.0, 2.0));  // floor(28.2743...) = 28.0
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)r3 + (int)r4;
    result += (int)r5 + (int)r6 + (int)r7 + (int)r8;
    result += (int)r9 + (int)r10;
    result += (int)p1 + (int)p2 + (int)p3;
    result += (int)fm1 + (int)fm2 + (int)fm3;
    result += (int)rem1 + (int)rem2;
    result += i1 + i2 + i3 + i4;
    result += arr[2] + arr[3];
    result += (int)d1 + (int)d2 + (int)d3;
    result += (int)mixed;
    result += (int)nested;
    result += (int)circle_area;
    
    printf("Result: %d\n", result);
    
    // Additional verification
    if (result != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
