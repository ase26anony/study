#include <stdio.h>
#include <math.h>

int main(void) {
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    
    int result = 0;
    
    // One-argument integer-valued functions with literals
    double r1 = rint(4.7);           // Should be 5.0
    double t1 = trunc(5.9);          // Should be 5.0
    double f1 = floor(6.2);          // Should be 6.0
    double c1 = ceil(3.1);           // Should be 4.0
    double rd1 = round(2.5);         // Should be 3.0
    
    // One-argument functions with const variables
    double r2 = __builtin_rint(a);   // Should be 11.0
    double t2 = __builtin_trunc(a);  // Should be 10.0
    double f2 = __builtin_floor(a);  // Should be 10.0
    double c2 = __builtin_ceil(a);   // Should be 11.0
    
    // Two-argument functions with literals
    double p1 = pow(2.0, 3.0);       // Should be 8.0 (integer)
    double p2 = __builtin_pow(3.0, 2.0); // Should be 9.0 (integer)
    double fm1 = fmod(9.0, 3.0);     // Should be 0.0 (integer)
    double fm2 = __builtin_fmod(10.0, 4.0); // Should be 2.0 (integer)
    double rem1 = remainder(14.0, 3.0); // Should be -1.0 (integer)
    
    // Two-argument functions with const variables
    double p3 = pow(c, d);           // 2^4 = 16.0 (integer)
    double fm3 = fmod(a, b);         // 10.5 % 3.0 = 1.5 (not integer - for variety)
    
    // Use results in integer contexts to prompt analysis
    int i1 = rint(3.14);             // Implicit conversion
    int i2 = trunc(7.8);
    int i3 = (int)floor(2.8);
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        result += 1;
    }
    
    if (__builtin_rint(4.3) == 4) {
        result += 2;
    }
    
    // Array indexing with floor/ceil
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    arr[(int)ceil(1.2)] = 7;
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;   // 8 + 1 = 9
    double d2 = fmod(9.0, 3.0) * 2;  // 0 * 2 = 0
    
    // Switch with round (compile-time constant)
    switch ((int)round(3.6)) {
        case 4:
            result += 4;
            break;
        default:
            result -= 1;
    }
    
    // Use nearbyint as well (another integer-valued function)
    double n1 = nearbyint(2.3);
    double n2 = __builtin_nearbyint(7.8);
    
    // Aggregate results to prevent optimization
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    result += (int)p3 + (int)fm3;
    result += i1 + i2 + i3;
    result += (int)d1 + (int)d2;
    result += (int)n1 + (int)n2;
    result += arr[2] + arr[1];
    
    printf("Result: %d\n", result);
    
    return 0;
}
