/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
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
    
    /* Using standard library functions (will be recognized as built-ins with -fbuiltin) */
    double r7 = rint(2.3);                  /* Should be 2.0 */
    double r8 = trunc(8.9);                 /* Should be 8.0 */
    double r9 = floor(7.0);                 /* Should be 7.0 */
    double r10 = ceil(c);                   /* Should be 4.0 */
    double r11 = round(9.6);                /* Should be 10.0 */
    
    /* Two-argument integer-valued functions with constant arguments */
    double p1 = __builtin_pow(2.0, 3.0);    /* 2³ = 8 (integer) */
    double p2 = pow(3.0, 2.0);              /* 3² = 9 (integer) */
    double f1 = __builtin_fmod(9.0, 3.0);   /* 9 % 3 = 0 (integer) */
    double f2 = fmod(14.0, 7.0);            /* 14 % 7 = 0 (integer) */
    double rem1 = remainder(20.0, 5.0);     /* 20 rem 5 = 0 (integer) */
    double rem2 = __builtin_remainder(18.0, 6.0); /* 18 rem 6 = 0 (integer) */
    
    /* Mix with non-integer results to ensure analysis is triggered */
    double p3 = __builtin_pow(2.0, 0.5);    /* √2 ≈ 1.414 (non-integer) */
    double f3 = fmod(10.0, 3.0);            /* 10 % 3 = 1 (integer) */
    
    /* Embed in contexts that require integer-valued analysis */
    
    /* 1. Assignments to integer types (implicit conversion analysis) */
    int i1 = __builtin_rint(3.14);          /* Should be 3 */
    int i2 = trunc(6.8);                    /* Should be 6 */
    int i3 = floor(9.1);                    /* Should be 9 */
    checksum += i1 + i2 + i3;
    
    /* 2. Comparisons with integers */
    if (__builtin_ceil(4.2) == 5) {
        checksum += 100;
    }
    if (round(7.5) == 8) {
        checksum += 200;
    }
    
    /* 3. Array indices */
    int arr[10] = {0};
    arr[(int)__builtin_floor(2.8)] = 5;     /* arr[2] = 5 */
    checksum += arr[2];
    
    /* 4. Binary operations with integers */
    double d1 = __builtin_pow(2.0, 4.0) + 1;    /* 16 + 1 = 17 */
    double d2 = fmod(15.0, 4.0) * 2;            /* 3 * 2 = 6 */
    checksum += (int)d1 + (int)d2;
    
    /* 5. Conditional expressions */
    int val = (__builtin_remainder(25.0, 5.0) == 0) ? 50 : 0;
    checksum += val;
    
    /* 6. Switch statement (prompts integer-valued analysis for conversion) */
    int sw = (int)__builtin_round(3.6);
    switch (sw) {
        case 4: checksum += 400; break;
        case 3: checksum += 300; break;
        default: checksum += 0;
    }
    
    /* Use all results to prevent dead code elimination */
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5 + (int)r6;
    checksum += (int)r7 + (int)r8 + (int)r9 + (int)r10 + (int)r11;
    checksum += (int)p1 + (int)p2 + (int)f1 + (int)f2 + (int)rem1 + (int)rem2;
    checksum += (int)(p3 * 100) + (int)(f3 * 10);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
