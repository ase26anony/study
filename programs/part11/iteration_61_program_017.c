#include <stdio.h>
#include <math.h>

int main() {
    // Use const variables to encourage constant folding
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);           // Standard library version
    double r2 = __builtin_trunc(5.9); // Builtin version
    double r3 = floor(4.0);          // Integer constant as double
    double r4 = ceil(a);             // Using const variable
    double r5 = round(3.14);         // Another standard function
    
    // Use results in integer context to prompt analysis
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
    
    // 2. Two-argument integer-valued functions
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);       // 2^3 = 8 (integer)
    double p2 = __builtin_pow(c, 4.0); // Builtin with const variable
    double f1 = fmod(9.0, 3.0);      // 9 % 3 = 0 (integer)
    double f2 = remainder(10.0, 2.0); // 10 % 2 = 0 (integer)
    
    // Use in comparisons with integers
    if (p1 == 8.0) checksum += 10;
    if (f1 == 0.0) checksum += 20;
    
    // 3. More complex expressions that might prompt analysis
    // Array indexing with floor result
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 42;
    checksum += arr[2];
    
    // Switch-like logic with round
    double val = 3.6;
    switch ((int)round(val)) {
        case 4: checksum += 30; break;
        default: checksum += 0;
    }
    
    // 4. Mixed expressions requiring integer-valued analysis
    // Binary operations with integers
    double expr1 = trunc(7.9) + 1;      // 7 + 1 = 8
    double expr2 = nearbyint(2.3) * 2;  // 2 * 2 = 4
    
    checksum += (int)expr1 + (int)expr2;
    
    // 5. Additional two-argument calls with different patterns
    double p3 = pow(d, 0.0);           // 5^0 = 1 (integer)
    double f3 = fmod(15.0, b);         // 15 % 3 = 0 (integer)
    
    // Use in conditional expressions
    int cond = (pow(3.0, 2.0) == 9.0) ? 50 : 0;
    checksum += cond;
    
    // 6. Builtin versions of one-argument functions
    double b1 = __builtin_rint(6.1);
    double b2 = __builtin_floor(8.9);
    double b3 = __builtin_ceil(3.2);
    
    checksum += (int)b1 + (int)b2 + (int)b3;
    
    // 7. Edge cases with exact integer results
    double e1 = pow(4.0, 0.5);         // sqrt(4) = 2 (integer)
    double e2 = fmod(16.0, 4.0);       // 16 % 4 = 0 (integer)
    
    // Final computation and output
    checksum += (int)e1 * 10;
    checksum += (e2 == 0.0) ? 25 : 0;
    
    printf("Checksum: %d\n", checksum);
    
    // Additional calls to ensure all paths are considered
    // These might be analyzed even if results aren't used
    (void)__builtin_round(1.5);
    (void)remainder(20.0, 5.0);
    (void)pow(1.0, 100.0);  // 1^100 = 1 (integer)
    
    return 0;
}
