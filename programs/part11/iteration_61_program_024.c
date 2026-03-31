#include <stdio.h>
#include <math.h>

int main(void) {
    // Declare const variables with constant initializers
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double t1 = __builtin_trunc(5.9); // Builtin version
    double f1 = floor(7.2);
    double c1 = ceil(3.1);
    double rd1 = round(6.5);
    double n1 = nearbyint(2.3);
    
    // Use results in integer context to prompt analysis
    result += (int)r1;
    result += (int)t1;
    result += (int)f1;
    result += (int)c1;
    result += (int)rd1;
    result += (int)n1;
    
    // 2. One-argument functions with const variables
    // These should also trigger arg0 extraction
    double r2 = __builtin_rint(a);
    double t2 = trunc(b);
    double f2 = __builtin_floor(a + 1.5);
    double c2 = ceil(d);
    
    // Use in comparisons with integers
    if (r2 == 11.0) result += 10;
    if (t2 == 3.0) result += 20;
    if (f2 == 11.0) result += 30;
    if (c2 == 4.0) result += 40;
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 8.0 - integer result
    double p2 = __builtin_pow(c, 5.0); // 32.0 - integer result
    double fm1 = fmod(9.0, 3.0);    // 0.0 - integer result
    double fm2 = __builtin_fmod(10.0, 4.0); // 2.0 - integer result
    double rem1 = remainder(14.0, 3.0); // 2.0 - integer result
    double rem2 = __builtin_remainder(20.0, 5.0); // 0.0 - integer result
    
    // Use as array indices (converted to int)
    int arr[100];
    arr[(int)p1] = 1;
    arr[(int)p2] = 2;
    arr[(int)fm1] = 3;
    arr[(int)fm2] = 4;
    arr[(int)rem1] = 5;
    arr[(int)rem2] = 6;
    
    result += arr[8] + arr[32] + arr[0] + arr[2] + arr[2] + arr[0];
    
    // 4. Two-argument functions with const variables
    double p3 = pow(c, d);          // 2^4 = 16
    double fm3 = fmod(a, b);        // 10.5 % 3.0 = 1.5 (not integer)
    double rem3 = remainder(d, c);  // 4.0 % 2.0 = 0.0
    
    // Use in binary operations with integers
    result += (int)(p3 + 1);
    result += (int)(fm3 * 2);       // 3.0 - integer after operation
    result += (int)rem3;
    
    // 5. Mixed usage in complex expressions
    // This may trigger deeper analysis
    double complex1 = pow(trunc(4.8), 2.0);  // 4^2 = 16
    double complex2 = floor(rint(3.7)) + ceil(trunc(2.9));
    
    // Use in switch-like logic
    int val = (int)complex1;
    switch (val) {
        case 16: result += 100; break;
        default: result += 0;
    }
    
    if (complex2 == 5.0) result += 50;
    
    // 6. Edge cases with exact integer arguments
    double e1 = rint(5.0);      // Already integer
    double e2 = floor(6.0);     // Already integer
    double e3 = pow(3.0, 0.0);  // 1.0 - integer
    double e4 = fmod(8.0, 2.0); // 0.0 - integer
    
    result += (int)e1 + (int)e2 + (int)e3 + (int)e4;
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
