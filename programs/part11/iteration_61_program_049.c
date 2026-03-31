#include <stdio.h>
#include <math.h>

int main(void) {
    // Use const variables to encourage constant folding analysis
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.2);
    double f2 = __builtin_floor(4.2);
    double c1 = ceil(3.1);
    double c2 = __builtin_ceil(3.1);
    double rd1 = round(6.5);
    double rd2 = __builtin_round(6.5);
    
    // Use in integer context to prompt integer-valued analysis
    checksum += (int)r1;
    checksum += (int)r2;
    checksum += (int)t1;
    checksum += (int)t2;
    
    // 2. One-argument functions with const variables
    // These may be analyzed differently
    double r3 = rint(a);
    double t3 = trunc(a);
    double f3 = floor(a);
    double c3 = ceil(a);
    
    checksum += (int)r3;
    checksum += (int)t3;
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);           // 8.0 - integer result
    double p2 = __builtin_pow(2.0, 5.0); // 32.0 - integer result
    double p3 = pow(c, d);               // 2^4 = 16 - integer result
    
    double fm1 = fmod(9.0, 3.0);           // 0.0 - integer result
    double fm2 = __builtin_fmod(10.0, 2.0); // 0.0 - integer result
    double fm3 = fmod(a, b);               // 10.5 % 3.0 = 1.5 - non-integer
    
    double rem1 = remainder(12.0, 5.0);    // 2.0 - integer result
    double rem2 = __builtin_remainder(14.0, 7.0); // 0.0 - integer result
    
    // Use in comparisons with integers
    if (p1 == 8.0) checksum += 1;
    if (fm1 == 0.0) checksum += 2;
    if (rem1 == 2.0) checksum += 3;
    
    // 4. Mixed expressions that require integer-valued analysis
    // Array indexing context
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    checksum += arr[2];
    
    // Binary operations with integers
    double mixed1 = pow(2.0, 3.0) + 1;      // 8 + 1 = 9
    double mixed2 = rint(4.3) * 2;          // 4 * 2 = 8
    double mixed3 = trunc(7.9) - 3;         // 7 - 3 = 4
    
    checksum += (int)mixed1;
    checksum += (int)mixed2;
    checksum += (int)mixed3;
    
    // 5. Conditional expressions requiring analysis
    int cond1 = (trunc(5.9) == 5) ? 10 : 0;
    int cond2 = (pow(3.0, 2.0) == 9) ? 20 : 0;
    int cond3 = (fmod(15.0, 5.0) == 0) ? 30 : 0;
    
    checksum += cond1 + cond2 + cond3;
    
    // 6. Additional builtin calls with different signatures
    double nb1 = nearbyint(3.7);
    double nb2 = __builtin_nearbyint(8.2);
    
    checksum += (int)nb1 + (int)nb2;
    
    // 7. Use integer constants that convert to double
    double from_int1 = floor(4);      // integer constant
    double from_int2 = pow(2, 5);     // two integer constants
    double from_int3 = fmod(9, 3);    // integer constants
    
    checksum += (int)from_int1 + (int)from_int2 + (int)from_int3;
    
    // Print result to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    // Return value based on checksum to ensure all code paths matter
    return (checksum > 100) ? 0 : 1;
}
