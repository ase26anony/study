/* Test program to cover integer_valued_real_p built-in function analysis
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
    double rd1 = round(7.5);        /* Should be 8.0 */
    
    /* 2. One-argument functions with const variables */
    double r2 = __builtin_rint(a);  /* Should be 11.0 */
    double t2 = __builtin_trunc(a); /* Should be 10.0 */
    double f2 = __builtin_floor(a); /* Should be 10.0 */
    double c2 = __builtin_ceil(a);  /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    /* pow with integer result: 2^3 = 8 */
    double p1 = pow(2.0, 3.0);
    double p2 = __builtin_pow(c, 3.0);
    
    /* fmod with zero remainder: 9.0 % 3.0 = 0.0 */
    double fm1 = fmod(9.0, 3.0);
    double fm2 = __builtin_fmod(e, b);
    
    /* remainder with zero remainder */
    double rem1 = remainder(10.0, 5.0);
    double rem2 = __builtin_remainder(20.0, 4.0);
    
    /* 4. Functions in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);            /* Should be 3 */
    int i2 = trunc(8.99);           /* Should be 8 */
    int i3 = (int)floor(7.2);       /* Should be 7 */
    
    /* Comparisons with integers */
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    
    if (__builtin_floor(10.1) == 10) {
        checksum += 2;
    }
    
    /* Array indexing with integer-valued results */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    arr[(int)__builtin_round(3.3)] = 7; /* arr[3] = 7 */
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;  /* 16 + 1 = 17 */
    double d2 = __builtin_fmod(15.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* 5. nearbyint variants */
    double n1 = nearbyint(4.3);     /* Should be 4.0 */
    double n2 = __builtin_nearbyint(5.8); /* Should be 6.0 */
    
    /* 6. Mixed constant expressions */
    double m1 = floor(4) + ceil(4.1); /* 4 + 5 = 9 */
    double m2 = __builtin_pow(3.0, 2.0) - 9; /* 9 - 9 = 0 */
    
    /* 7. Use results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    checksum += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2 + (int)n1 + (int)n2;
    checksum += i1 + i2 + i3 + (int)d1 + (int)d2;
    checksum += (int)m1 + (int)m2;
    checksum += arr[2] + arr[3];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls to ensure all paths are considered */
    volatile double v1 = __builtin_rint(100.7);
    volatile double v2 = __builtin_trunc(200.3);
    volatile double v3 = __builtin_pow(5.0, 2.0); /* 25 */
    volatile double v4 = __builtin_fmod(100.0, 25.0); /* 0 */
    
    return 0;
}
