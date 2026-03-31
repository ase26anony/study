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
    const int    n = 4;
    
    int checksum = 0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* __builtin_rint */
    double t1 = trunc(5.9);         /* __builtin_trunc */
    double f1 = floor(8.2);         /* __builtin_floor */
    double c1 = ceil(6.1);          /* __builtin_ceil */
    double r2 = round(3.5);         /* __builtin_round */
    
    /* Use results in integer assignments to prompt analysis */
    checksum += (int)r1;
    checksum += (int)t1;
    checksum += (int)f1;
    checksum += (int)c1;
    checksum += (int)r2;
    
    /* 2. One-argument functions with const variables */
    double r3 = __builtin_rint(a);   /* Explicit builtin */
    double t2 = __builtin_trunc(a);
    double f2 = floor(a);            /* Standard library version */
    double c2 = ceil(a);
    
    /* Use in comparisons with integers */
    if (trunc(a) == 10) checksum += 10;
    if (ceil(a) == 11)  checksum += 11;
    
    /* 3. Two-argument functions with constant pairs */
    /* pow with integer result */
    double p1 = pow(2.0, 3.0);      /* 8.0 - integer */
    double p2 = __builtin_pow(c, (double)n); /* 16.0 - integer */
    
    /* fmod with zero remainder */
    double fm1 = fmod(9.0, 3.0);    /* 0.0 - integer */
    double fm2 = __builtin_fmod(14.0, 7.0); /* 0.0 - integer */
    
    /* remainder with zero remainder */
    double rem1 = remainder(10.0, 5.0); /* 0.0 - integer */
    double rem2 = __builtin_remainder(20.0, 4.0); /* 0.0 - integer */
    
    /* Use in integer context */
    checksum += (int)p1;
    checksum += (int)p2;
    checksum += (int)fm1;
    checksum += (int)fm2;
    checksum += (int)rem1;
    checksum += (int)rem2;
    
    /* 4. Mixed constant and variable arguments (still analyzable) */
    const double pi = 3.141592653589793;
    double r4 = rint(pi);           /* 3.0 - integer */
    double t3 = trunc(pi);          /* 3.0 - integer */
    
    /* Use as array index */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    checksum += arr[2];
    
    /* 5. Binary operations with integers */
    double d = pow(2.0, 4.0) + 1;   /* 16.0 + 1 = 17.0 */
    checksum += (int)d;
    
    /* 6. Nested calls */
    double nested = floor(pow(2.5, 2.0)); /* floor(6.25) = 6.0 */
    checksum += (int)nested;
    
    /* 7. Use in switch (prompts integer analysis) */
    switch ((int)round(3.7)) {
        case 4: checksum += 4; break;
        default: break;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
