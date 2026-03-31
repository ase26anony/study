#include <stdio.h>
#include <math.h>

int main(void) {
    // Declare const variables with constant initializers
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.7;
    const double e = 9.0;
    const double f = 3.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library function
    double t1 = trunc(5.9);          // Standard library function  
    double f1 = floor(4.2);          // Standard library function
    double c1 = ceil(3.1);           // Standard library function
    double r2 = round(2.5);          // Standard library function
    
    // 2. One-argument functions with const variables
    // These should also trigger arg0 extraction
    double r3 = __builtin_rint(a);   // Builtin version
    double t2 = __builtin_trunc(d);  // Builtin version
    double f2 = __builtin_floor(a);  // Builtin version
    double c2 = __builtin_ceil(d);   // Builtin version
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);       // 8.0 - integer result
    double p2 = __builtin_pow(c, 5.0); // 32.0 - integer result
    double fm1 = fmod(9.0, 3.0);     // 0.0 - integer result
    double fm2 = __builtin_fmod(e, f); // 0.0 - integer result
    double rem1 = remainder(10.0, 2.0); // 0.0 - integer result
    
    // 4. Integer conversions that may prompt integer-valued analysis
    int i1 = rint(3.14);             // Implicit conversion to int
    int i2 = (int)trunc(7.8);        // Explicit cast
    int i3 = floor(6.0);             // Direct assignment
    
    // 5. Comparisons with integers (may prompt analysis)
    if (trunc(4.9) == 4) {
        result += 1;
    }
    
    if (__builtin_floor(5.1) == 5) {
        result += 2;
    }
    
    // 6. Array indexing with integer-valued functions
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 3;        // arr[2] = 3
    arr[(int)__builtin_round(3.2)] = 4; // arr[3] = 4
    
    // 7. Binary operations with integers
    double d1 = pow(2.0, 4.0) + 1;   // 16 + 1 = 17
    double d2 = __builtin_fmod(15.0, 4.0) * 2; // 3 * 2 = 6
    
    // 8. Mixed expressions
    double complex = rint(trunc(floor(ceil(3.7)))); // Nested calls
    
    // 9. Use nearbyint (another integer-valued function)
    double n1 = nearbyint(3.5);
    double n2 = __builtin_nearbyint(4.3);
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    result += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    result += i1 + i2 + i3;
    result += arr[2] + arr[3];
    result += (int)d1 + (int)d2 + (int)complex + (int)n1 + (int)n2;
    
    printf("Result: %d\n", result);
    
    return 0;
}
