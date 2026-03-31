/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -lm test.c -o test
 * Or: gcc -O3 -ffast-math -lm test.c -o test
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* One-argument integer-valued functions with constant arguments */
    const double a = 10.5;
    const double b = -3.7;
    const double c = 4.0;
    
    /* Using __builtin_ prefixed versions */
    double r1 = __builtin_rint(4.7);        /* Should be 5.0 */
    double r2 = __builtin_trunc(5.9);       /* Should be 5.0 */
    double r3 = __builtin_floor(a);         /* Should be 10.0 */
    double r4 = __builtin_ceil(b);          /* Should be -3.0 */
    double r5 = __builtin_round(3.2);       /* Should be 3.0 */
    double r6 = __builtin_nearbyint(6.5);   /* Should be 6.0 or 7.0 depending on rounding mode */
    
    /* Using standard library functions (recognized as built-ins with -fbuiltin) */
    double r7 = rint(8.3);                  /* Should be 8.0 */
    double r8 = trunc(-2.9);                /* Should be -2.0 */
    double r9 = floor(7.0);                 /* Should be 7.0 */
    double r10 = ceil(c);                   /* Should be 4.0 */
    double r11 = round(9.6);                /* Should be 10.0 */
    
    /* Two-argument integer-valued functions with constant arguments */
    double p1 = __builtin_pow(2.0, 3.0);    /* 8.0 - integer result */
    double p2 = pow(3.0, 2.0);              /* 9.0 - integer result */
    double f1 = __builtin_fmod(9.0, 3.0);   /* 0.0 - integer result */
    double f2 = fmod(14.0, 7.0);            /* 0.0 - integer result */
    double rem1 = remainder(10.0, 5.0);     /* 0.0 - integer result */
    double rem2 = __builtin_remainder(21.0, 7.0); /* 0.0 - integer result */
    
    /* Mixed constant expressions */
    double p3 = __builtin_pow(4.0, 0.5);    /* 2.0 - integer result */
    double f3 = fmod(17.0, 1.0);            /* 0.0 - integer result */
    
    /* Embed calls in contexts that require integer-valued analysis */
    
    /* 1. Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);                    /* Should be 3 */
    int i2 = __builtin_trunc(8.99);         /* Should be 8 */
    int i3 = floor(6.0);                    /* Should be 6 */
    
    /* 2. Comparisons with integers */
    if (trunc(4.7) == 4) {
        checksum += 1;
    }
    if (__builtin_ceil(2.1) == 3) {
        checksum += 2;
    }
    
    /* 3. Array indices */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;               /* arr[2] = 5 */
    checksum += arr[2];
    
    /* 4. Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;          /* 16.0 + 1 = 17.0 */
    double d2 = __builtin_fmod(20.0, 6.0) * 2; /* 2.0 * 2 = 4.0 */
    
    /* 5. Switch statement (constant folding for labels) */
    int val = (int)round(3.5);              /* Should be 4 */
    switch (val) {
        case 4:
            checksum += 4;
            break;
        default:
            checksum += 0;
    }
    
    /* 6. Conditional expressions */
    double result = (rint(5.4) > 5) ? 10.0 : 20.0; /* Should be 10.0 */
    checksum += (int)result;
    
    /* 7. Complex constant expressions */
    const double x = 12.3;
    const double y = 4.0;
    double complex1 = ceil(x) - floor(x);   /* 13.0 - 12.0 = 1.0 */
    double complex2 = pow(2.0, floor(y));   /* 2^4 = 16.0 */
    
    /* Aggregate results into checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
    checksum += (int)r6 + (int)r7 + (int)r8 + (int)r9 + (int)r10;
    checksum += (int)r11 + (int)p1 + (int)p2 + (int)f1 + (int)f2;
    checksum += (int)rem1 + (int)rem2 + (int)p3 + (int)f3;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)complex1 + (int)complex2;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls to ensure coverage of both argument extraction paths */
    
    /* Functions with exactly one argument */
    volatile double v1 = __builtin_trunc(100.9);
    volatile double v2 = __builtin_floor(100.9);
    volatile double v3 = __builtin_ceil(100.1);
    volatile double v4 = __builtin_rint(100.5);
    
    /* Functions with two arguments */
    volatile double v5 = __builtin_pow(5.0, 2.0);   /* 25.0 */
    volatile double v6 = __builtin_fmod(25.0, 5.0); /* 0.0 */
    volatile double v7 = __builtin_remainder(30.0, 5.0); /* 0.0 */
    
    /* Use volatile results to prevent optimization */
    checksum += (int)v1 + (int)v2 + (int)v3 + (int)v4;
    checksum += (int)v5 + (int)v6 + (int)v7;
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
