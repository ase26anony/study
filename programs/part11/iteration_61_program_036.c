/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables for use in function calls */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double rnd1 = round(7.5);       /* Should be 8.0 */
    double nr1 = nearbyint(2.3);    /* Should be 2.0 */
    
    /* 2. One-argument functions with const variables */
    double r2 = __builtin_rint(a);      /* Should be 11.0 */
    double t2 = __builtin_trunc(a);     /* Should be 10.0 */
    double f2 = __builtin_floor(a);     /* Should be 10.0 */
    double c2 = __builtin_ceil(a);      /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    /* pow with integer result */
    double p1 = pow(2.0, 3.0);          /* 2^3 = 8 (integer) */
    double p2 = __builtin_pow(c, d);    /* 2^4 = 16 (integer) */
    
    /* fmod with remainder 0 (integer result) */
    double fm1 = fmod(9.0, 3.0);        /* 9 % 3 = 0 (integer) */
    double fm2 = __builtin_fmod(e, b);  /* 9 % 3 = 0 (integer) */
    
    /* remainder with 0 remainder */
    double rem1 = remainder(10.0, 5.0); /* Should be 0 */
    double rem2 = __builtin_remainder(15.0, 5.0); /* Should be 0 */
    
    /* 4. Functions in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);                /* Should be 3 */
    int i2 = trunc(8.99);               /* Should be 8 */
    int i3 = floor(7.0);                /* Should be 7 */
    
    /* Comparisons with integers */
    if (trunc(4.8) == 4) {
        checksum += 1;
    }
    
    if (__builtin_floor(9.1) == 9) {
        checksum += 2;
    }
    
    /* Array indexing (requires integer context) */
    int arr[10];
    arr[(int)floor(2.8)] = 42;          /* arr[2] = 42 */
    checksum += arr[2];
    
    /* Binary operations with integers */
    double d1 = pow(3.0, 2.0) + 1;      /* 9 + 1 = 10 */
    double d2 = __builtin_rint(5.5) * 2; /* 6 * 2 = 12 */
    
    /* 5. Mixed function calls in complex expressions */
    double complex1 = rint(trunc(floor(ceil(3.7)))); /* Nested calls */
    double complex2 = pow(fmod(16.0, 4.0), 2.0);     /* 0^2 = 0 */
    
    /* 6. Use results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rnd1 + (int)nr1;
    checksum += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2 + i1 + i2 + i3;
    checksum += (int)d1 + (int)d2 + (int)complex1 + (int)complex2;
    
    /* 7. Additional calls to ensure both argument extraction paths */
    /* Single argument extraction */
    double single1 = __builtin_trunc(100.7);
    double single2 = __builtin_round(50.3);
    
    /* Two argument extraction */
    double double1 = __builtin_pow(5.0, 2.0);    /* 25 */
    double double2 = __builtin_fmod(20.0, 5.0);  /* 0 */
    double double3 = __builtin_remainder(30.0, 6.0); /* 0 */
    
    checksum += (int)single1 + (int)single2;
    checksum += (int)double1 + (int)double2 + (int)double3;
    
    printf("Checksum: %d\n", checksum);
    
    /* Use results in conditional to ensure all code paths are considered */
    if (checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
