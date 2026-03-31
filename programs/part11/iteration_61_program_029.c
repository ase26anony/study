/* This program is designed to trigger coverage of lines 15993-16000 in
   GCC's fold-const.cc, specifically the argument extraction and
   integer-valued real function analysis within `integer_valued_real_p`.
   It uses various built-in math functions with constant arguments in
   contexts that encourage compile-time analysis. */

#include <stdio.h>
#include <math.h>

int main(void) {
    /* Use const variables to provide constant arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const int    n = 5;
    
    int result = 0;
    
    /* 1. One-argument integer-valued functions with literals */
    double d1 = rint(4.7);          /* __builtin_rint */
    double d2 = trunc(5.9);         /* __builtin_trunc */
    double d3 = floor(8.2);         /* __builtin_floor */
    double d4 = ceil(7.1);          /* __builtin_ceil */
    double d5 = round(3.5);         /* __builtin_round */
    double d6 = nearbyint(6.3);     /* __builtin_nearbyint */
    
    /* Use results in integer assignments (prompts analysis) */
    int i1 = (int)d1;
    int i2 = (int)d2;
    int i3 = (int)d3;
    int i4 = (int)d4;
    int i5 = (int)d5;
    int i6 = (int)d6;
    
    result += i1 + i2 + i3 + i4 + i5 + i6;
    
    /* 2. One-argument functions with const variables */
    double d7 = rint(a);            /* 10.5 -> 10.0 or 11.0 depending on rounding */
    double d8 = trunc(a);           /* 10.5 -> 10.0 */
    double d9 = floor(a);           /* 10.5 -> 10.0 */
    double d10 = ceil(a);           /* 10.5 -> 11.0 */
    
    /* Use in comparisons with integers */
    if (d7 == 10.0) result += 1;
    if (d8 == 10.0) result += 2;
    if (d9 == 10.0) result += 3;
    if (d10 == 11.0) result += 4;
    
    /* 3. Two-argument functions with constant literals */
    double d11 = pow(2.0, 3.0);     /* 8.0, integer result */
    double d12 = fmod(9.0, 3.0);    /* 0.0, integer result */
    double d13 = remainder(10.0, 3.0); /* 1.0, integer result */
    double d14 = pow(c, (double)n); /* 2^5 = 32.0, integer result */
    
    /* Use in array indexing context */
    int arr[10] = {0};
    arr[(int)d11] = 1;              /* arr[8] = 1 */
    arr[(int)d12] = 2;              /* arr[0] = 2 */
    arr[(int)d13] = 3;              /* arr[1] = 3 */
    
    result += arr[8] + arr[0] + arr[1];
    
    /* 4. Two-argument functions with mixed const arguments */
    double d15 = fmod(a, b);        /* 10.5 % 3.0 = 1.5 (not integer) */
    double d16 = pow(b, 2.0);       /* 3^2 = 9.0, integer result */
    
    /* Use in binary operations with integers */
    double d17 = d15 + 1;           /* 2.5, not integer */
    double d18 = d16 + 0;           /* 9.0, integer */
    
    if (d18 == 9.0) result += 5;
    
    /* 5. Explicit __builtin_ prefixed calls (direct built-in recognition) */
    double d19 = __builtin_rint(3.14);
    double d20 = __builtin_trunc(7.89);
    double d21 = __builtin_pow(3.0, 2.0);  /* 9.0 */
    double d22 = __builtin_fmod(12.0, 4.0); /* 0.0 */
    
    /* Use in switch-like logic (prompts integer-valued analysis) */
    int val = (int)d19 + (int)d20 + (int)d21 + (int)d22;
    switch (val) {
        case 19: result += 10; break; /* 3 + 7 + 9 + 0 = 19 */
        default: result += 0;
    }
    
    /* 6. Additional integer-valued calls with integer constants */
    double d23 = floor(4);          /* integer constant promoted to double */
    double d24 = pow(2, 5);         /* integer constants, result 32.0 */
    double d25 = fmod(15.0, 5.0);   /* 0.0 */
    
    result += (int)d23 + (int)d24 + (int)d25;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
