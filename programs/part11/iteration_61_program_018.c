/* Test program to cover integer_valued_real_p lines in GCC's fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int result = 0;
    
    /* Constant variables to use as arguments */
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
    double r2 = rint(a);            /* Should be 11.0 */
    double t2 = trunc(a);           /* Should be 10.0 */
    double f2 = floor(a);           /* Should be 10.0 */
    double c2 = ceil(a);            /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);      /* 2³ = 8 (integer) */
    double p2 = pow(c, 5.0);        /* 2⁵ = 32 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(e, b);        /* 9 % 3 = 0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* 10 rem 2 = 0 (integer) */
    double rem2 = remainder(14.0, 7.0); /* 14 rem 7 = 0 (integer) */
    
    /* 4. Built-in versions explicitly */
    double br1 = __builtin_rint(3.14);
    double bt1 = __builtin_trunc(8.9);
    double bp1 = __builtin_pow(3.0, 2.0);  /* 3² = 9 (integer) */
    double bfm1 = __builtin_fmod(15.0, 5.0); /* 15 % 5 = 0 (integer) */
    
    /* 5. Use results in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);            /* Should be 3 */
    int i2 = trunc(7.8);            /* Should be 7 */
    int i3 = (int)floor(9.9);       /* Should be 9 */
    
    /* Comparisons with integers */
    if (trunc(4.7) == 4) {
        result += 1;
    }
    
    if (floor(5.2) == 5) {
        result += 2;
    }
    
    if (ceil(6.1) == 7) {
        result += 4;
    }
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 42;      /* arr[2] = 42 */
    result += arr[2];
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;  /* 16 + 1 = 17 */
    double d2 = fmod(20.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* Conditional expressions */
    int cond = (rint(8.3) > 8) ? 10 : 20;
    result += cond;
    
    /* Switch statement (may trigger analysis for case labels) */
    int val = (int)round(3.6);      /* Should be 4 */
    switch (val) {
        case 4:
            result += 8;
            break;
        default:
            result += 16;
    }
    
    /* 6. Mixed expressions to ensure all paths are exercised */
    double mixed1 = rint(pow(2.0, 3.0));  /* rint(8.0) = 8.0 */
    double mixed2 = trunc(fmod(17.0, 4.0)); /* trunc(1.0) = 1.0 */
    
    /* 7. Edge cases with integer arguments */
    double e1 = floor(4);           /* Integer constant */
    double e2 = ceil(5);            /* Integer constant */
    double e3 = pow(2, 3);          /* Integer constants */
    
    /* 8. Use results to prevent dead code elimination */
    result += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rnd1 + (int)nr1;
    result += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    result += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1 + (int)rem2;
    result += (int)br1 + (int)bt1 + (int)bp1 + (int)bfm1;
    result += (int)d1 + (int)d2;
    result += (int)mixed1 + (int)mixed2;
    result += (int)e1 + (int)e2 + (int)e3;
    
    printf("Result: %d\n", result);
    
    /* Additional calls to ensure coverage of different function codes */
    /* These might be analyzed even if results aren't used */
    (void)rintf(4.7f);      /* float version */
    (void)truncl(5.9L);     /* long double version */
    (void)__builtin_floorf(3.2f);
    (void)__builtin_ceill(6.7L);
    
    return 0;
}
