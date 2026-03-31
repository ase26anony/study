#include <stdio.h>
#include <math.h>

int main(void) {
    // Use const variables to encourage constant folding
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
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
    double f2 = __builtin_floor(a);
    double c2 = __builtin_ceil(b);
    
    // Use in comparisons with integers
    if (r2 == 11.0) result += 10;
    if (t2 == 3.0) result += 20;
    if (f2 == 10.0) result += 30;
    if (c2 == 3.0) result += 40;
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 8.0 - integer result
    double p2 = __builtin_pow(c, d); // 32.0 - integer result
    double fm1 = fmod(9.0, 3.0);    // 0.0 - integer result
    double fm2 = __builtin_fmod(14.0, 4.0); // 2.0 - integer result
    double rem1 = remainder(10.0, 3.0); // 1.0 - integer result
    double rem2 = __builtin_remainder(20.0, 6.0); // 2.0 - integer result
    
    // Use in array indexing context
    int arr[10] = {0};
    arr[(int)p1] = 100;
    arr[(int)fm1] = 200;
    result += arr[8] + arr[0];
    
    // Use in arithmetic with integers
    result += (int)(p2 + 1);
    result += (int)(fm2 * 2);
    result += (int)(rem1 - 0);
    result += (int)(rem2 / 1);
    
    // 4. Mixed constant and non-constant arguments
    // Still should trigger argument extraction
    double x = 8.0;
    double p3 = pow(x, 0.0);  // 1.0 - integer result
    double p4 = pow(1.0, x);  // 1.0 - integer result
    
    result += (int)p3;
    result += (int)p4;
    
    // 5. Integer constants converted to double
    // These are still real expressions
    double f3 = floor(4);     // integer constant
    double c3 = ceil(9);      // integer constant
    double p5 = pow(2, 5);    // both integer constants
    
    result += (int)f3;
    result += (int)c3;
    result += (int)p5;
    
    // 6. Nested calls and complex expressions
    double nested = floor(pow(2.5, 2.0));  // floor(6.25) = 6.0
    double complex_expr = rint(fmod(17.0, 5.0)) + trunc(3.7); // 2.0 + 3.0 = 5.0
    
    result += (int)nested;
    result += (int)complex_expr;
    
    // 7. Use in switch context (prompts integer analysis)
    int val = (int)round(3.6);  // 4
    switch (val) {
        case 4:
            result += 1000;
            break;
        default:
            result += 0;
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional calls to ensure coverage of different paths
    // These might be analyzed even if not all results are used
    (void)__builtin_rint(0.0);
    (void)trunc(1.5);
    (void)__builtin_floor(2.99);
    (void)ceil(3.01);
    (void)__builtin_round(4.4);
    (void)nearbyint(5.6);
    (void)__builtin_pow(3.0, 2.0);
    (void)fmod(15.0, 4.0);
    (void)__builtin_remainder(25.0, 5.0);
    
    return 0;
}
