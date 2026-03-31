#include <stdio.h>
#include <math.h>

int main(void) {
    // Declare constant variables for use as arguments
    const double a = 10.5;
    const double b = 3.0;
    const double c = 7.2;
    const double d = 2.0;
    const int e = 4;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(3.14);
    double f2 = __builtin_floor(3.14);
    double c1 = ceil(2.3);
    double c2 = __builtin_ceil(2.3);
    double rd1 = round(6.5);
    double rd2 = __builtin_round(6.5);
    
    // Use in integer context to prompt analysis
    int i1 = rint(3.14);           // Assignment to int
    int i2 = (int)__builtin_trunc(8.9);
    
    // 2. One-argument functions with const variables
    double r3 = rint(a);
    double t3 = trunc(c);
    double f3 = floor(a);
    double c3 = ceil(c);
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);          // 2^3 = 8 (integer)
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(d, e);              // 2^4 = 16 (integer)
    double p4 = __builtin_pow(d, e);
    
    double fm1 = fmod(9.0, 3.0);        // 9 % 3 = 0 (integer)
    double fm2 = __builtin_fmod(9.0, 3.0);
    double fm3 = fmod(12.5, 2.5);       // 12.5 % 2.5 = 0 (integer)
    double fm4 = __builtin_fmod(12.5, 2.5);
    
    double rem1 = remainder(10.0, 3.0); // 10 % 3 = 1 (integer)
    double rem2 = __builtin_remainder(10.0, 3.0);
    
    // 4. Use two-argument results in integer contexts
    int i3 = pow(3.0, 2.0);             // 3^2 = 9
    int i4 = (int)__builtin_fmod(15.0, 5.0);
    
    // 5. Comparisons with integers (may prompt analysis)
    if (trunc(4.8) == 4) {
        result += 1;
    }
    
    if (__builtin_floor(5.1) == 5) {
        result += 2;
    }
    
    // 6. Array indexing with integer-valued functions
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;           // arr[2] = 5
    arr[(int)__builtin_round(3.2)] = 7; // arr[3] = 7
    
    // 7. Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;      // 8 + 1 = 9
    double d2 = __builtin_fmod(20.0, 6.0) * 2; // 2 * 2 = 4
    
    // 8. Mixed expressions
    double mixed = rint(3.7) + trunc(4.2) + floor(5.9);
    
    // 9. Additional builtins that are integer-valued
    double n1 = nearbyint(6.3);
    double n2 = __builtin_nearbyint(6.3);
    
    // 10. Use in switch (though constant folding may happen earlier)
    int val = (int)round(2.6);  // val = 3
    switch (val) {
        case 3:
            result += 4;
            break;
        default:
            break;
    }
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)t1 + (int)t2;
    result += (int)f1 + (int)f2 + (int)c1 + (int)c2;
    result += (int)rd1 + (int)rd2 + (int)r3 + (int)t3;
    result += (int)f3 + (int)c3 + (int)p1 + (int)p2;
    result += (int)p3 + (int)p4 + (int)fm1 + (int)fm2;
    result += (int)fm3 + (int)fm4 + (int)rem1 + (int)rem2;
    result += i1 + i2 + i3 + i4;
    result += (int)d1 + (int)d2 + (int)mixed;
    result += (int)n1 + (int)n2;
    result += arr[2] + arr[3];
    
    printf("Result: %d\n", result);
    
    return 0;
}
