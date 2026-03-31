#include <stdio.h>
#include <math.h>

int main(void) {
    // Use const variables to encourage compile-time evaluation
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    
    int checksum = 0;
    
    // 1. One-argument integer-valued functions with literals
    // These should trigger arg0 extraction
    double r1 = rint(4.7);          // standard library version
    double t1 = trunc(5.9);         // truncates to 5.0
    double f1 = floor(8.2);         // floors to 8.0
    double c1 = ceil(3.1);          // ceils to 4.0
    double rd1 = round(6.5);        // rounds to 7.0
    
    // Use __builtin_ versions explicitly
    double r2 = __builtin_rint(2.3);
    double t2 = __builtin_trunc(7.8);
    double f2 = __builtin_floor(a);  // using const variable
    
    // 2. Two-argument functions with constant pairs
    // These should trigger both arg0 and arg1 extraction
    double p1 = pow(2.0, 3.0);      // 8.0 - integer result
    double p2 = __builtin_pow(c, d); // 2^5 = 32.0
    double fm1 = fmod(9.0, 3.0);    // 0.0 - integer result
    double fm2 = __builtin_fmod(14.0, 7.0); // 0.0
    double rem1 = remainder(10.0, 5.0); // 0.0
    
    // 3. Embed calls in contexts that require integer-valued analysis
    
    // Assignments to integer types (implicit conversion)
    int i1 = rint(3.14);            // Should be 3
    int i2 = trunc(9.99);           // Should be 9
    int i3 = floor(4.0);            // Should be 4
    
    // Comparisons with integers
    if (trunc(7.8) == 7) {
        checksum += 1;
    }
    
    if (floor(5.2) == 5) {
        checksum += 2;
    }
    
    // Array indexing
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       // arr[2] = 5
    checksum += arr[2];
    
    // Binary operations with integers
    double d1 = pow(2.0, 4.0) + 1;  // 16.0 + 1 = 17.0
    double d2 = fmod(15.0, 5.0) * 2; // 0.0 * 2 = 0.0
    
    // 4. Mixed constant and variable usage
    // (Some variables to prevent all compile-time evaluation)
    double x = 3.14159;
    double y = 2.71828;
    
    // These might still be analyzed for integer-valued property
    double r3 = rint(x);
    double p3 = pow(x, 2.0);
    
    // 5. Use results to compute checksum (prevent dead code elimination)
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    checksum += (int)r2 + (int)t2 + (int)f2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)r3 + (int)p3;
    
    printf("Checksum: %d\n", checksum);
    
    // Additional calls to ensure coverage of different built-ins
    // nearbyint is another integer-valued function
    double n1 = nearbyint(4.3);
    double n2 = __builtin_nearbyint(7.6);
    
    // lrint, lround return long int but take double arguments
    long l1 = lrint(4.5);
    long l2 = lround(3.7);
    
    printf("Additional: %f %f %ld %ld\n", n1, n2, l1, l2);
    
    return 0;
}
