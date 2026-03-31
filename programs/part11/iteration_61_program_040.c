/* Test program to cover lines 15993-16000 in fold-const.cc
 * These lines handle argument extraction for built-in math functions
 * in integer_valued_real_p, which determines if a real expression
 * produces integer results.
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables to use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    
    /* 1. One-argument integer-valued functions with literals */
    /* rint - rounds to nearest integer according to current rounding mode */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double r2 = __builtin_rint(2.3); /* Builtin version */
    
    /* trunc - truncates toward zero */
    double t1 = trunc(5.9);          /* Should be 5.0 */
    double t2 = __builtin_trunc(7.2); /* Builtin version */
    
    /* floor - largest integer not greater than argument */
    double f1 = floor(8.9);          /* Should be 8.0 */
    double f2 = __builtin_floor(a);  /* Using const variable: floor(10.5) = 10.0 */
    
    /* ceil - smallest integer not less than argument */
    double c1 = ceil(3.1);           /* Should be 4.0 */
    double c2 = __builtin_ceil(6.8); /* Builtin version */
    
    /* round - rounds to nearest integer, away from zero */
    double rd1 = round(1.5);         /* Should be 2.0 */
    double rd2 = __builtin_round(9.4); /* Builtin version */
    
    /* nearbyint - rounds to nearby integer according to current rounding mode */
    double n1 = nearbyint(3.6);      /* Should be 4.0 */
    double n2 = __builtin_nearbyint(7.1); /* Builtin version */
    
    /* 2. Two-argument integer-valued functions with constants */
    /* pow - with integer results */
    double p1 = pow(2.0, 3.0);       /* 2^3 = 8 (integer) */
    double p2 = __builtin_pow(c, d); /* 2^4 = 16 (integer) */
    
    /* fmod - remainder of division */
    double fm1 = fmod(9.0, 3.0);     /* 9 % 3 = 0 (integer) */
    double fm2 = __builtin_fmod(e, b); /* 9 % 3 = 0 (integer) */
    
    /* remainder - IEEE remainder */
    double rem1 = remainder(10.0, 5.0); /* 10 % 5 = 0 (integer) */
    double rem2 = __builtin_remainder(20.0, 4.0); /* 20 % 4 = 0 (integer) */
    
    /* 3. Use results in contexts that may prompt integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);      /* Should be 3 */
    int i2 = trunc(6.99);     /* Should be 6 */
    int i3 = floor(4.2);      /* Should be 4 */
    int i4 = ceil(5.1);       /* Should be 6 */
    int i5 = round(2.5);      /* Should be 3 */
    
    /* Comparisons with integers */
    if (trunc(7.8) == 7) {
        checksum += 1;
    }
    
    if (floor(9.1) == 9) {
        checksum += 2;
    }
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;  /* arr[2] = 5 */
    
    /* Binary operations with integers */
    double b1 = pow(3.0, 2.0) + 1;   /* 9 + 1 = 10 */
    double b2 = fmod(15.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* Complex expressions mixing different functions */
    double complex1 = rint(trunc(4.9) + floor(3.2)); /* rint(4 + 3) = 7 */
    double complex2 = pow(2.0, ceil(1.1)) - floor(3.9); /* 2^2 - 3 = 1 */
    
    /* 4. Aggregate results into checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)r2;
    checksum += (int)t1 + (int)t2;
    checksum += (int)f1 + (int)f2;
    checksum += (int)c1 + (int)c2;
    checksum += (int)rd1 + (int)rd2;
    checksum += (int)n1 + (int)n2;
    checksum += (int)p1 + (int)p2;
    checksum += (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2;
    checksum += i1 + i2 + i3 + i4 + i5;
    checksum += (int)b1 + (int)b2;
    checksum += (int)complex1 + (int)complex2;
    checksum += arr[2];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls to ensure all paths are exercised */
    /* Functions with exactly 0 arguments won't trigger the uncovered lines,
       but functions with 1 or 2 arguments will */
    
    /* Edge case: pow with negative exponent but integer result */
    double p3 = pow(4.0, -2.0); /* 4^-2 = 0.0625 (not integer) */
    checksum += (p3 > 0) ? 1 : 0;
    
    /* Use constexpr-like computation (C99 compound literal) */
    double cl = floor((const double){12.7}); /* Should be 12.0 */
    checksum += (int)cl;
    
    return checksum == 0 ? 0 : 1;
}
