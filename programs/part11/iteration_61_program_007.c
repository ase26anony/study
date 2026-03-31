#include <stdio.h>
#include <math.h>

int main() {
    // Use const variables to encourage constant folding
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int result = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.2);
    double f2 = __builtin_floor(4.2);
    double c1 = ceil(4.2);
    double c2 = __builtin_ceil(4.2);
    double rd1 = round(3.6);
    double rd2 = __builtin_round(3.6);
    
    // Use in integer context to prompt integer-valued analysis
    int i1 = (int)rint(3.14);
    int i2 = (int)__builtin_trunc(7.89);
    
    // 2. Two-argument functions with constants
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);           // 8.0 - integer result
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(c, d);               // 2^5 = 32 - using const variables
    double fm1 = fmod(9.0, 3.0);         // 0.0 - integer result
    double fm2 = __builtin_fmod(9.0, 3.0);
    double rem1 = remainder(10.0, 5.0);  // 0.0 - integer result
    double rem2 = __builtin_remainder(10.0, 5.0);
    
    // Use two-argument functions in integer contexts
    int i3 = (int)pow(3.0, 2.0);         // 9
    int i4 = (int)__builtin_fmod(15.0, 5.0); // 0
    
    // 3. Mixed contexts that require integer-valued analysis
    // Comparisons with integers
    if (trunc(a) == 10) {
        result += 1;
    }
    
    if (__builtin_floor(4.8) == 4) {
        result += 2;
    }
    
    // Array indexing with integer-valued functions
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    arr[(int)__builtin_round(3.2)] = 7;
    
    // Binary operations with integers
    double d1 = pow(2.0, 4.0) + 1;           // 16 + 1 = 17
    double d2 = __builtin_rint(5.3) * 2;     // 5 * 2 = 10
    
    // 4. Additional integer-valued functions
    double n1 = nearbyint(6.7);
    double n2 = __builtin_nearbyint(6.7);
    
    // 5. Functions that are integer-valued for specific constant inputs
    double p4 = pow(4.0, 0.5);           // sqrt(4) = 2.0 - integer result
    double p5 = __builtin_pow(8.0, 1.0/3.0); // cube root of 8 = 2.0
    
    // 6. Use constexpr-like behavior with const variables
    const double e = 12.7;
    double r3 = rint(e);
    double t3 = trunc(e);
    
    // 7. Complex expressions involving multiple integer-valued functions
    double complex_expr = floor(pow(2.0, 3.0)) + ceil(trunc(4.9));
    
    // 8. Switch-like logic (though switch requires integer constant)
    int val = (int)round(3.5);
    if (val == 4) {
        result += 4;
    }
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)t1 + (int)t2;
    result += (int)f1 + (int)f2 + (int)c1 + (int)c2;
    result += (int)rd1 + (int)rd2 + (int)p1 + (int)p2;
    result += (int)p3 + (int)fm1 + (int)fm2 + (int)rem1;
    result += (int)rem2 + i1 + i2 + i3 + i4;
    result += (int)d1 + (int)d2 + (int)n1 + (int)n2;
    result += (int)p4 + (int)p5 + (int)r3 + (int)t3;
    result += (int)complex_expr + arr[2] + arr[3];
    
    printf("Result: %d\n", result);
    
    return 0;
}
