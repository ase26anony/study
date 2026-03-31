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
    double r1 = rint(4.7);           // Should be 5.0
    double t1 = trunc(5.9);          // Should be 5.0
    double f1 = floor(6.2);          // Should be 6.0
    double c1 = ceil(3.1);           // Should be 4.0
    double rd1 = round(7.5);         // Should be 8.0
    
    // 2. One-argument functions with const variables
    double r2 = __builtin_rint(a);   // Should be 11.0
    double t2 = __builtin_trunc(a);  // Should be 10.0
    double f2 = __builtin_floor(a);  // Should be 10.0
    double c2 = __builtin_ceil(a);   // Should be 11.0
    
    // 3. Two-argument functions with constant pairs
    double p1 = pow(2.0, 3.0);       // 8.0 (integer)
    double p2 = __builtin_pow(c, d); // 32.0 (integer)
    double fm1 = fmod(9.0, 3.0);     // 0.0 (integer)
    double fm2 = __builtin_fmod(14.0, 4.0); // 2.0 (integer)
    double rem1 = remainder(10.0, 2.0);     // 0.0 (integer)
    
    // 4. Use results in integer contexts to prompt analysis
    int i1 = rint(3.14);             // Implicit conversion
    int i2 = (int)trunc(8.9);        // Explicit cast
    int i3 = floor(4.0);             // Direct assignment
    
    // 5. Comparisons with integers
    if (trunc(7.8) == 7) {
        result += 1;
    }
    
    if (__builtin_floor(9.2) == 9) {
        result += 2;
    }
    
    // 6. Array indexing with integer-valued results
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;        // arr[2] = 5
    
    // 7. Binary operations with integers
    double d1 = pow(3.0, 2.0) + 1;   // 9.0 + 1 = 10.0
    double d2 = rint(4.3) * 2;       // 4.0 * 2 = 8.0
    
    // 8. Mixed expressions
    double mixed = trunc(12.7) + __builtin_floor(3.8) - ceil(2.1);
    
    // 9. Use nearbyint (another integer-valued function)
    double n1 = nearbyint(6.4);      // Should be 6.0
    double n2 = __builtin_nearbyint(7.6); // Should be 8.0
    
    // 10. Additional two-argument cases
    double p3 = pow(4.0, 0.5);       // 2.0 (integer result)
    double fm3 = fmod(17.0, 5.0);    // 2.0 (integer)
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    result += i1 + i2 + i3;
    result += (int)d1 + (int)d2;
    result += (int)mixed;
    result += (int)n1 + (int)n2;
    result += (int)p3 + (int)fm3;
    result += arr[2];
    
    printf("Result: %d\n", result);
    
    // Additional test: switch with integer-valued real function
    switch ((int)round(3.7)) {
        case 4:
            result += 100;
            break;
        default:
            result += 200;
    }
    
    printf("Final result: %d\n", result);
    
    return 0;
}
