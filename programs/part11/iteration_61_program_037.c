#include <stdio.h>
#include <math.h>

int main() {
    // Declare const variables with constant initializers
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.7;
    const double e = 9.0;
    const double f = 3.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // Using __builtin_ prefixed versions
    double r1 = __builtin_rint(4.7);      // Should be 5.0
    double t1 = __builtin_trunc(5.9);     // Should be 5.0
    double f1 = __builtin_floor(4.2);     // Should be 4.0
    double c1 = __builtin_ceil(3.1);      // Should be 4.0
    double r2 = __builtin_round(2.5);     // Should be 3.0
    
    // 2. One-argument functions with const variables
    double r3 = rint(a);                  // Should be 11.0
    double t2 = trunc(d);                 // Should be 4.0
    double f2 = floor(a);                 // Should be 10.0
    double c2 = ceil(d);                  // Should be 5.0
    
    // 3. Two-argument functions with constant pairs
    // Using __builtin_ prefixed versions
    double p1 = __builtin_pow(2.0, 3.0);  // 2^3 = 8 (integer)
    double p2 = __builtin_pow(c, 5.0);    // 2^5 = 32 (integer)
    double fm1 = __builtin_fmod(9.0, 3.0); // 9 % 3 = 0 (integer)
    double fm2 = __builtin_fmod(e, f);    // 9 % 3 = 0 (integer)
    double rem1 = __builtin_remainder(10.0, 2.0); // 10 % 2 = 0 (integer)
    
    // 4. Standard library calls (will be recognized as builtins with -fbuiltin)
    double p3 = pow(3.0, 2.0);            // 3^2 = 9 (integer)
    double fm3 = fmod(12.0, 4.0);         // 12 % 4 = 0 (integer)
    double rem2 = remainder(15.0, 5.0);   // 15 % 5 = 0 (integer)
    
    // 5. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);                  // Should be 3
    int i2 = trunc(7.89);                 // Should be 7
    int i3 = floor(6.1);                  // Should be 6
    int i4 = ceil(5.01);                  // Should be 6
    
    // Comparisons with integers
    if (trunc(8.9) == 8) {
        checksum += 1;
    }
    
    if (floor(3.2) == 3) {
        checksum += 2;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;             // arr[2] = 5
    arr[(int)round(3.2)] = 7;             // arr[3] = 7
    
    // Binary operations with integers
    double d1 = pow(2.0, 4.0) + 1;        // 16 + 1 = 17
    double d2 = fmod(20.0, 5.0) * 2;      // 0 * 2 = 0
    
    // 6. Use nearbyint (another integer-valued function)
    double n1 = nearbyint(2.3);           // Should be 2.0
    double n2 = __builtin_nearbyint(7.8); // Should be 8.0
    
    // 7. Mixed constant expressions
    double mixed1 = trunc(4.9) + floor(3.7) - ceil(2.1);
    double mixed2 = pow(2.0, 3.0) * fmod(10.0, 3.0);
    
    // 8. Conditional expressions
    int cond_val = (rint(4.4) > 4) ? 1 : 0;
    checksum += cond_val;
    
    // 9. Complex constant folding case
    // This should trigger analysis of pow's integer-valued property
    const double base = 2.0;
    const double exp = 8.0;
    double power_result = pow(base, exp); // 2^8 = 256 (integer)
    int power_int = (int)power_result;
    
    // 10. Use results to avoid dead code elimination
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)p3 + (int)fm3 + (int)rem2;
    checksum += i1 + i2 + i3 + i4;
    checksum += (int)d1 + (int)d2;
    checksum += (int)n1 + (int)n2;
    checksum += (int)mixed1 + (int)mixed2;
    checksum += power_int;
    
    // Add array values
    for (int j = 0; j < 10; j++) {
        checksum += arr[j];
    }
    
    printf("Checksum: %d\n", checksum);
    
    // Additional test: switch with integer-valued function result
    // (though constant folding for labels happens earlier)
    switch ((int)round(2.5)) {
        case 3:
            checksum += 100;
            break;
        default:
            checksum += 200;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
