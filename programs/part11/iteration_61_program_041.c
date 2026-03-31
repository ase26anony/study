/* Test program to cover lines 15993-16000 in fold-const.cc
 * These lines handle argument extraction for built-in math functions
 * in integer_valued_real_p, which checks if a real expression yields
 * integer values.
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* One-argument integer-valued functions with constant arguments */
    const double a = 10.5;
    const double b = -3.7;
    
    /* Using __builtin_ prefixed versions */
    double r1 = __builtin_rint(4.7);        /* Should be 5.0 */
    double r2 = __builtin_trunc(5.9);       /* Should be 5.0 */
    double r3 = __builtin_floor(a);         /* Should be 10.0 */
    double r4 = __builtin_ceil(b);          /* Should be -3.0 */
    double r5 = __builtin_round(2.3);       /* Should be 2.0 */
    double r6 = __builtin_nearbyint(6.5);   /* Depends on rounding mode */
    
    /* Using standard library functions (will be recognized as built-ins
     * with -fbuiltin or -O1 and above) */
    double r7 = rint(8.2);                  /* Should be 8.0 */
    double r8 = trunc(9.99);                /* Should be 9.0 */
    double r9 = floor(7.0);                 /* Should be 7.0 */
    double r10 = ceil(-4.2);                /* Should be -4.0 */
    double r11 = round(3.5);                /* Should be 4.0 (ties to even) */
    
    /* Two-argument integer-valued functions with constant arguments */
    const double c = 9.0;
    const double d = 3.0;
    
    /* Functions that can produce integer results with specific inputs */
    double p1 = __builtin_pow(2.0, 3.0);    /* 8.0 - integer */
    double p2 = pow(3.0, 2.0);              /* 9.0 - integer */
    double f1 = __builtin_fmod(c, d);       /* 0.0 - integer */
    double f2 = fmod(12.0, 4.0);            /* 0.0 - integer */
    double rem1 = __builtin_remainder(10.0, 5.0);  /* 0.0 - integer */
    double rem2 = remainder(14.0, 7.0);     /* 0.0 - integer */
    
    /* Embed calls in contexts that may require integer-valued analysis */
    
    /* 1. Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);                    /* Should be 3 */
    int i2 = trunc(6.8);                    /* Should be 6 */
    int i3 = (int)__builtin_floor(4.2);     /* Should be 4 */
    
    /* 2. Comparisons with integers */
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    
    if (__builtin_ceil(2.1) == 3.0) {
        checksum += 2;
    }
    
    /* 3. Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;               /* arr[2] = 5 */
    checksum += arr[2];
    
    /* 4. Binary operations with integers */
    double b1 = pow(2.0, 4.0) + 1;          /* 16.0 + 1 = 17.0 */
    double b2 = __builtin_fmod(15.0, 5.0) * 2;  /* 0.0 * 2 = 0.0 */
    
    /* 5. Conditional expressions */
    int cond = (rint(4.5) == 4.0) ? 10 : 20;
    checksum += cond;
    
    /* 6. Switch statement (prompts integer conversion analysis) */
    switch ((int)round(3.7)) {
        case 4: checksum += 4; break;
        default: checksum += 0;
    }
    
    /* 7. Mixed constant expressions */
    double m1 = __builtin_pow(2.0, 3.0) + __builtin_trunc(4.9);
    double m2 = fmod(20.0, 6.0) * floor(3.2);
    
    /* Aggregate results to avoid dead code elimination */
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4;
    checksum += (int)r5 + (int)r6 + (int)r7 + (int)r8;
    checksum += (int)r9 + (int)r10 + (int)r11;
    checksum += (int)p1 + (int)p2 + (int)f1 + (int)f2;
    checksum += (int)rem1 + (int)rem2 + (int)b1 + (int)b2;
    checksum += i1 + i2 + i3 + (int)m1 + (int)m2;
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
