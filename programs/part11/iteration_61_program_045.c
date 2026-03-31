/* Test program to cover lines 15993-16000 in fold-const.cc
 * These lines handle argument extraction for built-in math functions
 * in integer_valued_real_p, which determines if a floating-point
 * expression always yields integer results.
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables to use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const int n = 4;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double rd1 = round(7.5);        /* Should be 8.0 */
    
    /* 2. One-argument functions with const variables */
    double r2 = rint(a);            /* Should be 11.0 */
    double t2 = trunc(a);           /* Should be 10.0 */
    double f2 = floor(a);           /* Should be 10.0 */
    double c2 = ceil(a);            /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);      /* 2^3 = 8 (integer) */
    double p2 = pow(c, 4.0);        /* 2^4 = 16 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(10.0, b);     /* 10 % 3 = 1 (integer) */
    double rem1 = remainder(15.0, 5.0); /* 15 rem 5 = 0 */
    double rem2 = remainder(17.0, 3.0); /* 17 rem 3 = -1 */
    
    /* 4. Using __builtin_ prefixed versions explicitly */
    double br1 = __builtin_rint(2.3);      /* Should be 2.0 */
    double bt1 = __builtin_trunc(8.9);     /* Should be 8.0 */
    double bp1 = __builtin_pow(3.0, 2.0);  /* 3^2 = 9 (integer) */
    double bfm1 = __builtin_fmod(12.0, 4.0); /* 12 % 4 = 0 */
    
    /* 5. Embed calls in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);            /* Should be 3 */
    int i2 = trunc(7.8);            /* Should be 7 */
    int i3 = floor(9.9);            /* Should be 9 */
    int i4 = (int)pow(5.0, 2.0);    /* 5^2 = 25 */
    
    /* Comparisons with integers */
    if (trunc(4.7) == 4) checksum += 1;
    if (floor(5.1) == 5) checksum += 2;
    if (ceil(6.9) == 7) checksum += 4;
    if (rint(8.3) == 8) checksum += 8;
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    arr[(int)round(3.2)] = 7;       /* arr[3] = 7 */
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 3.0) + 1;  /* 8 + 1 = 9 */
    double d2 = fmod(14.0, 7.0) * 2; /* 0 * 2 = 0 */
    double d3 = remainder(20.0, 6.0) - 2; /* 2 - 2 = 0 */
    
    /* 6. Mixed argument counts to ensure both extraction paths */
    /* One-arg: */ double m1 = nearbyint(6.7);
    /* Two-arg: */ double m2 = pow(2.0, (double)n); /* n is const int */
    /* Two-arg with mixed const/literal: */ double m3 = fmod(a, 2.5);
    
    /* 7. Use results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    checksum += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2;
    checksum += (int)br1 + (int)bt1 + (int)bp1 + (int)bfm1;
    checksum += i1 + i2 + i3 + i4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += arr[2] + arr[3];
    checksum += (int)m1 + (int)m2 + (int)m3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
