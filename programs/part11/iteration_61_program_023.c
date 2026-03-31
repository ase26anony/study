/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * This exercises integer-valued real built-in function analysis during constant folding.
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    const double pi = 3.141592653589793;
    const double e = 2.718281828459045;
    int checksum = 0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(4.2);         /* Should be 4.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double r2 = round(2.5);         /* Should be 3.0 */
    double n1 = nearbyint(6.3);     /* Should be 6.0 */
    
    /* Use __builtin_ prefixed versions explicitly */
    double r3 = __builtin_rint(7.8);    /* Should be 8.0 */
    double t2 = __builtin_trunc(9.1);   /* Should be 9.0 */
    double f2 = __builtin_floor(5.0);   /* Should be 5.0 */
    double c2 = __builtin_ceil(2.01);   /* Should be 3.0 */
    
    /* 2. Two-argument integer-valued functions with constant pairs */
    double p1 = pow(2.0, 3.0);          /* 2^3 = 8 (integer) */
    double p2 = pow(3.0, 2.0);          /* 3^2 = 9 (integer) */
    double fm1 = fmod(9.0, 3.0);        /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(12.5, 2.5);       /* 12.5 % 2.5 = 0 (integer) */
    double rem1 = remainder(10.0, 5.0); /* 10 % 5 = 0 (integer) */
    double rem2 = remainder(7.0, 3.0);  /* 7 % 3 = 1 (integer) */
    
    /* __builtin_ versions of two-argument functions */
    double p3 = __builtin_pow(4.0, 2.0);        /* 4^2 = 16 (integer) */
    double fm3 = __builtin_fmod(15.0, 5.0);     /* 15 % 5 = 0 (integer) */
    double rem3 = __builtin_remainder(20.0, 4.0); /* 20 % 4 = 0 (integer) */
    
    /* 3. Functions with const variables as arguments */
    double f3 = floor(pi);              /* Should be 3.0 */
    double c3 = ceil(e);                /* Should be 3.0 */
    double t3 = trunc(pi * 2.0);        /* Should be 6.0 */
    double p4 = pow(e, 2.0);            /* e^2 ≈ 7.389 (not integer - for contrast) */
    
    /* 4. Embed calls in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);                /* Should become 3 */
    int i2 = trunc(8.99);               /* Should become 8 */
    int i3 = floor(6.0);                /* Should become 6 */
    
    /* Comparisons with integers */
    if (trunc(4.7) == 4) {
        checksum += 1;
    }
    
    if (floor(5.2) == 5) {
        checksum += 2;
    }
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 42;          /* arr[2] = 42 */
    arr[(int)round(3.2)] = 99;          /* arr[3] = 99 */
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;      /* 16 + 1 = 17 */
    double d2 = fmod(20.0, 6.0) * 2;    /* 2 * 2 = 4 */
    
    /* 5. Mixed expressions to encourage analysis */
    double mixed1 = trunc(pow(2.0, 3.0)) + floor(4.7);
    double mixed2 = rint(fmod(17.0, 5.0)) * ceil(3.3);
    
    /* 6. Use results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2 + (int)n1;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1 + (int)rem2;
    checksum += (int)p3 + (int)fm3 + (int)rem3;
    checksum += (int)f3 + (int)c3 + (int)t3;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)mixed1 + (int)mixed2;
    checksum += arr[2] + arr[3];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls in return expression */
    return (int)(rint(checksum / 10.0) + trunc(checksum % 10.0));
}
