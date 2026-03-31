/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables for use in calls */
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
    double r2 = round(7.5);         /* Should be 8.0 */
    
    /* 2. One-argument functions with const variables */
    double r3 = __builtin_rint(a);  /* Should be 11.0 */
    double t2 = __builtin_trunc(a); /* Should be 10.0 */
    double f2 = __builtin_floor(a); /* Should be 10.0 */
    double c2 = __builtin_ceil(a);  /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);      /* 2^3 = 8 (integer) */
    double p2 = __builtin_pow(c, d); /* 2^4 = 16 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* 9 % 3 = 0 (integer) */
    double fm2 = __builtin_fmod(e, b); /* 9 % 3 = 0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* 10 rem 2 = 0 (integer) */
    
    /* 4. Functions in assignment to integer types (prompts analysis) */
    int i1 = rint(3.14);            /* Implicit conversion */
    int i2 = trunc(8.99);
    int i3 = (int)floor(2.8);
    
    /* 5. Functions in comparisons with integers */
    if (trunc(7.3) == 7) {
        checksum += 1;
    }
    
    if (__builtin_floor(9.9) == 9) {
        checksum += 2;
    }
    
    /* 6. Functions as array indices */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    arr[(int)__builtin_ceil(1.1)] = 3;
    
    /* 7. Functions in binary operations with integers */
    double d1 = pow(2.0, 3.0) + 1;      /* 8 + 1 = 9 */
    double d2 = __builtin_fmod(15.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* 8. nearbyint function (another integer-valued builtin) */
    double n1 = nearbyint(6.7);         /* Should be 7.0 */
    double n2 = __builtin_nearbyint(2.3); /* Should be 2.0 */
    
    /* 9. Mixed: function calls with integer constants */
    double m1 = rint(4);                /* Integer constant promoted to double */
    double m2 = pow(3, 2);              /* Integer constants, 3^2 = 9 */
    
    /* 10. Use results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)n1 + (int)n2;
    checksum += (int)m1 + (int)m2;
    checksum += arr[2] + arr[1];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls to ensure coverage of different paths */
    volatile double v1 = __builtin_rint(100.1);
    volatile double v2 = __builtin_trunc(200.9);
    volatile double v3 = __builtin_pow(5.0, 2.0);  /* 25 */
    volatile double v4 = __builtin_fmod(20.0, 4.0); /* 0 */
    
    return 0;
}
