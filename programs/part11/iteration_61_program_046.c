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
    double r2 = __builtin_rint(4.7); // Builtin version
    double t1 = trunc(5.9);
    double t2 = __builtin_trunc(5.9);
    double f1 = floor(4.2);
    double f2 = __builtin_floor(4.2);
    double c1 = ceil(3.1);
    double c2 = __builtin_ceil(3.1);
    double r3 = round(6.5);
    double r4 = __builtin_round(6.5);
    
    // Use in integer context to prompt integer-valued analysis
    checksum += (int)r1;
    checksum += (int)r2;
    checksum += (int)t1;
    checksum += (int)t2;
    
    // 2. One-argument functions with const variables
    // These may be folded differently
    double r5 = rint(a);
    double f3 = floor(a);
    double c3 = ceil(a);
    double t3 = trunc(a);
    
    checksum += (int)r5;
    checksum += (int)f3;
    
    // 3. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);           // 8.0 - integer result
    double p2 = __builtin_pow(2.0, 3.0);
    double p3 = pow(c, 3.0);            // Using const variable
    double p4 = pow(3.0, 2.0);          // 9.0 - integer result
    
    double fm1 = fmod(9.0, 3.0);        // 0.0 - integer result
    double fm2 = __builtin_fmod(9.0, 3.0);
    double fm3 = fmod(10.0, b);         // Using const variable
    
    double rem1 = remainder(10.0, 3.0); // 1.0 - integer result
    double rem2 = __builtin_remainder(10.0, 3.0);
    
    // Use in integer context
    checksum += (int)p1;
    checksum += (int)p2;
    checksum += (int)fm1;
    checksum += (int)fm2;
    checksum += (int)rem1;
    
    // 4. Mixed expressions that require integer-valued analysis
    // Comparisons with integers
    if (trunc(7.8) == 7) {
        checksum += 10;
    }
    
    if (floor(9.2) == 9) {
        checksum += 20;
    }
    
    // Array indexing (prompts conversion analysis)
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    checksum += arr[2];
    
    // Binary operations with integers
    double mixed1 = pow(2.0, 4.0) + 1;      // 16 + 1 = 17
    double mixed2 = rint(3.7) * 2;          // 4 * 2 = 8
    
    checksum += (int)mixed1;
    checksum += (int)mixed2;
    
    // 5. Additional cases with different constant patterns
    // Integer constants converted to double
    double f4 = floor(4);                   // Integer constant
    double c4 = ceil(4);
    double p5 = pow(2, 5);                  // Both integer constants
    
    // Negative values
    double r6 = rint(-3.3);
    double f5 = floor(-3.3);
    double c5 = ceil(-3.3);
    
    checksum += (int)f4;
    checksum += (int)c4;
    checksum += (int)p5;
    checksum += (int)r6;
    checksum += (int)f5;
    checksum += (int)c5;
    
    // 6. Use in switch context (may trigger analysis)
    double val = round(3.6);
    switch ((int)val) {
        case 4:
            checksum += 100;
            break;
        default:
            checksum += 200;
    }
    
    // Print result to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    // Also print some values to ensure they're used
    printf("Results: %.2f %.2f %.2f %.2f\n", p1, fm1, rem1, mixed1);
    
    return 0;
}
