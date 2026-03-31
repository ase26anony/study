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
    double r1 = rint(4.7);          // Standard library version
    double r2 = __builtin_trunc(5.9); // Builtin version
    double r3 = floor(4.0);         // Integer constant as double
    double r4 = ceil(a);            // Using const variable
    double r5 = round(3.49);        // Another standard function
    
    // 2. Two-argument integer-valued functions with constants
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 2^3 = 8 (integer)
    double p2 = __builtin_pow(c, d); // 2^5 = 32 (integer)
    double f1 = fmod(9.0, 3.0);     // 9 % 3 = 0 (integer)
    double f2 = remainder(10.0, 2.0); // 10 % 2 = 0 (integer)
    
    // 3. Use results in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);            // Should be 3
    int i2 = trunc(7.89);           // Should be 7
    int i3 = floor(2.8);            // Should be 2
    
    // Comparisons with integers
    if (trunc(5.9) == 5) {
        result += 1;
    }
    
    if (__builtin_floor(4.2) == 4) {
        result += 2;
    }
    
    // Array indexing (requires integer value)
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       // arr[2] = 5
    
    // Binary operations with integers
    double calc1 = pow(2.0, 4.0) + 10;  // 16 + 10 = 26
    double calc2 = fmod(15.0, 5.0) * 3; // 0 * 3 = 0
    
    // 4. More complex expressions with mixed functions
    double complex1 = rint(trunc(floor(ceil(3.3)))); // Nested calls
    double complex2 = pow(floor(4.8), ceil(2.1));    // 4^3 = 64
    
    // 5. Use nearbyint (another integer-valued function)
    double n1 = nearbyint(6.3);
    double n2 = __builtin_nearbyint(7.8);
    
    // 6. Additional two-argument cases
    double p3 = pow(3.0, 2.0);      // 3^2 = 9
    double f3 = fmod(20.0, 4.0);    // 20 % 4 = 0
    double rem1 = remainder(25.0, 5.0); // 25 % 5 = 0
    
    // 7. Use in conditional expressions
    int choice = (rint(2.5) > 2) ? 1 : 0;  // Should be 1
    
    // 8. Mix with arithmetic
    double mixed = pow(2.0, 3.0) + floor(4.7) - ceil(3.2);
    // 8 + 4 - 4 = 8
    
    // Aggregate results to prevent dead code elimination
    result += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
    result += (int)p1 + (int)p2 + (int)f1 + (int)f2;
    result += i1 + i2 + i3;
    result += arr[2];
    result += (int)calc1 + (int)calc2;
    result += (int)complex1 + (int)complex2;
    result += (int)n1 + (int)n2;
    result += (int)p3 + (int)f3 + (int)rem1;
    result += choice;
    result += (int)mixed;
    
    printf("Result: %d\n", result);
    
    // Expected result calculation:
    // r1=5, r2=5, r3=4, r4=11, r5=3 → 5+5+4+11+3 = 28
    // p1=8, p2=32, f1=0, f2=0 → 8+32+0+0 = 40
    // i1=3, i2=7, i3=2 → 3+7+2 = 12
    // arr[2]=5 → 5
    // calc1=26, calc2=0 → 26+0 = 26
    // complex1=3, complex2=64 → 3+64 = 67
    // n1=6, n2=8 → 6+8 = 14
    // p3=9, f3=0, rem1=0 → 9+0+0 = 9
    // choice=1 → 1
    // mixed=8 → 8
    // Total: 28+40+12+5+26+67+14+9+1+8 = 210
    
    return 0;
}
