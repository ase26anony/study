#include <stdio.h>
#include <math.h>

int main(void) {
    // Use const variables to encourage constant folding
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    // One-argument integer-valued functions with literals
    double r1 = rint(4.7);          // Should be 5.0
    double t1 = trunc(5.9);         // Should be 5.0
    double f1 = floor(6.2);         // Should be 6.0
    double c1 = ceil(3.1);          // Should be 4.0
    double rd1 = round(7.5);        // Should be 8.0
    
    // One-argument functions with const variables
    double r2 = rint(a);            // Should be 11.0
    double t2 = trunc(a);           // Should be 10.0
    double f2 = floor(a);           // Should be 10.0
    double c2 = ceil(a);            // Should be 11.0
    
    // Two-argument functions with constant pairs
    double p1 = pow(2.0, 3.0);      // 8.0 (integer)
    double p2 = pow(c, d);          // 32.0 (integer)
    double fm1 = fmod(9.0, 3.0);    // 0.0 (integer)
    double fm2 = fmod(10.0, b);     // 1.0 (integer)
    double rem1 = remainder(10.0, 3.0); // 1.0 (integer)
    
    // __builtin versions to ensure internal function handling
    double br1 = __builtin_rint(2.3);      // 2.0
    double bt1 = __builtin_trunc(8.9);     // 8.0
    double bp1 = __builtin_pow(3.0, 2.0);  // 9.0
    
    // Use results in integer contexts to prompt integer-valued analysis
    int ir1 = (int)rint(3.14);             // Implicit conversion
    int it1 = trunc(7.8);                  // Assignment to int
    int ip1 = pow(2.0, 4.0);               // 16, integer result
    
    // Comparisons with integers
    if (trunc(4.9) == 4) {
        result += 1;
    }
    
    if (floor(5.1) == 5) {
        result += 2;
    }
    
    // Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;      // arr[2] = 5
    arr[(int)round(3.2)] = 7;      // arr[3] = 7
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;     // 8.0 + 1 = 9.0
    double d2 = rint(4.3) * 2;         // 4.0 * 2 = 8.0
    
    // More complex expressions
    double complex1 = fmod(pow(2.0, 3.0), 2.0);  // 8 % 2 = 0
    double complex2 = ceil(trunc(6.7) / 2.0);    // ceil(6/2) = 3
    
    // Use results to prevent dead code elimination
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    result += (int)br1 + (int)bt1 + (int)bp1;
    result += ir1 + it1 + ip1;
    result += (int)d1 + (int)d2;
    result += (int)complex1 + (int)complex2;
    result += arr[2] + arr[3];
    
    printf("Result: %d\n", result);
    
    return 0;
}
