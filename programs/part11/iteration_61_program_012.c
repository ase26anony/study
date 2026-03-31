/* This program is designed to trigger coverage of lines 15993-16000 in
   fold-const.cc, which handle integer-valued real built-in function analysis.
   Compile with: gcc -O2 -fno-math-errno -o test test.c
   Or with: gcc -O3 -ffast-math -o test test.c
*/

#include <stdio.h>
#include <math.h>

int main(void) {
    /* Constant variables to use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const int    n = 4;
    
    int checksum = 0;
    
    /* 1. One-argument integer-valued functions using literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(7.3);          /* Should be 8.0 */
    double r2 = round(8.5);         /* Should be 9.0 (rounds away from zero) */
    
    /* Use __builtin_ prefixed versions explicitly */
    double n1 = __builtin_nearbyint(9.1);  /* Should be 9.0 */
    double t2 = __builtin_trunc(3.14);     /* Should be 3.0 */
    
    /* 2. One-argument functions using const variables */
    double f2 = floor(a);           /* Should be 10.0 */
    double c2 = ceil(a);            /* Should be 11.0 */
    
    /* 3. Two-argument integer-valued functions with constant arguments */
    double p1 = pow(2.0, 3.0);      /* 2^3 = 8 (integer) */
    double p2 = pow(c, 5.0);        /* 2^5 = 32 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(14.0, 4.0);   /* 14 % 4 = 2 (integer) */
    double rem1 = remainder(10.0, 2.0); /* Should be 0 */
    
    /* __builtin_ versions of two-argument functions */
    double p3 = __builtin_pow(3.0, 2.0);   /* 3^2 = 9 (integer) */
    double fm3 = __builtin_fmod(20.0, 5.0); /* 20 % 5 = 0 */
    
    /* 4. Embed calls in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);            /* Should become 3 */
    int i2 = trunc(8.9);            /* Should become 8 */
    int i3 = (int)floor(4.2);       /* Explicit cast */
    
    /* Comparisons with integers */
    if (trunc(7.8) == 7) {
        checksum += 1;
    }
    
    if (floor(9.1) == 9) {
        checksum += 2;
    }
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    checksum += arr[2];
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;  /* 16 + 1 = 17 */
    int i4 = (int)d1;
    checksum += i4;
    
    /* Using constexpr-like integer constant */
    double p4 = pow(2, n);          /* 2^4 = 16 (integer) */
    checksum += (int)p4;
    
    /* 5. Mix of standard and __builtin_ calls with different argument counts */
    double r3 = __builtin_rint(6.6);
    double r4 = rint(1.1);
    double f3 = __builtin_floor(3.9);
    double c3 = ceil(0.1);
    
    /* More two-argument calls */
    double rem2 = remainder(15.0, 4.0); /* Should be -1 or 3 depending on rounding */
    double fm4 = fmod(17.0, 3.0);   /* 17 % 3 = 2 */
    
    /* 6. Compute final checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)n1 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)p3 + (int)fm3 + (int)r3 + (int)r4 + (int)f3 + (int)c3;
    checksum += (int)rem2 + (int)fm4;
    checksum += i1 + i2 + i3;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
