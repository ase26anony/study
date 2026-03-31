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
    // These should trigger arg0 extraction and integer-valued analysis
    
    // Standard library versions (rely on -fbuiltin)
    double r1 = rint(4.7);          // Should be 5.0
    double t1 = trunc(5.9);         // Should be 5.0
    double f1 = floor(6.2);         // Should be 6.0
    double c1 = ceil(3.1);          // Should be 4.0
    double rnd1 = round(2.5);       // Should be 3.0
    
    // __builtin versions (explicit built-in calls)
    double r2 = __builtin_rint(7.3);
    double t2 = __builtin_trunc(8.8);
    double f2 = __builtin_floor(9.1);
    double c2 = __builtin_ceil(1.9);
    double rnd2 = __builtin_round(6.6);
    
    // 2. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    
    // pow with integer result (2^3 = 8)
    double p1 = pow(2.0, 3.0);
    double p2 = __builtin_pow(3.0, 2.0);  // 9.0
    
    // fmod with exact division (9.0 % 3.0 = 0.0)
    double fm1 = fmod(9.0, 3.0);
    double fm2 = __builtin_fmod(12.0, 4.0);
    
    // remainder with integer result
    double rem1 = remainder(10.0, 5.0);  // 0.0
    double rem2 = __builtin_remainder(14.0, 7.0);
    
    // 3. Use const variables as arguments
    double r3 = rint(a);           // rint(10.5) = 11.0
    double f3 = floor(a);          // floor(10.5) = 10.0
    double p3 = pow(c, d);         // 2^5 = 32.0
    
    // 4. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);           // Should be 3
    int i2 = trunc(7.89);          // Should be 7
    int i3 = (int)pow(2.0, 4.0);   // 16
    
    // Comparisons with integers (prompts analysis)
    if (trunc(4.7) == 4) {
        checksum += 1;
    }
    
    if (floor(5.2) == 5) {
        checksum += 2;
    }
    
    // Array indexing (requires integer value)
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;      // arr[2] = 5
    
    // Binary operations with integers
    double d1 = pow(2.0, 3.0) + 1;     // 8 + 1 = 9
    double d2 = rint(4.3) * 2;         // 4 * 2 = 8
    
    // 5. Complex expressions mixing different functions
    double complex1 = floor(pow(2.0, 3.0) + 0.5);  // floor(8.5) = 8
    double complex2 = trunc(remainder(15.0, 4.0) + 3.5);  // trunc(3.0 + 3.5) = 6
    
    // 6. Use results in checksum to prevent dead code elimination
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rnd1;
    checksum += (int)r2 + (int)t2 + (int)f2 + (int)c2 + (int)rnd2;
    checksum += (int)p1 + (int)p2;
    checksum += (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2;
    checksum += (int)r3 + (int)f3 + (int)p3;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)complex1 + (int)complex2;
    checksum += arr[2];
    
    printf("Checksum: %d\n", checksum);
    
    // Additional calls to ensure all paths are considered
    // nearbyint is another integer-valued function
    double n1 = nearbyint(3.7);
    double n2 = __builtin_nearbyint(8.2);
    
    // lrint and llrint return integer types directly
    long lr = lrint(4.5);
    long long llr = llrint(6.7);
    
    printf("Additional: %f, %f, %ld, %lld\n", n1, n2, lr, llr);
    
    return 0;
}
