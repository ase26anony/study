/* Test program to cover integer_valued_real_p lines in fold-const.cc */
#include <stdio.h>
#include <math.h>

/* Use __builtin_ prefix for some functions to ensure GCC internal handling */
#define rint __builtin_rint
#define trunc __builtin_trunc
#define floor __builtin_floor
#define ceil __builtin_ceil
#define round __builtin_round
#define nearbyint __builtin_nearbyint
#define pow __builtin_pow
#define fmod __builtin_fmod
#define remainder __builtin_remainder

int main(void) {
    int checksum = 0;
    
    /* Constant variables to use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(7.2);         /* Should be 7.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double r2 = round(6.5);         /* Should be 7.0 (rounds away from zero) */
    double n1 = nearbyint(8.3);     /* Should be 8.0 */
    
    /* Use in integer context to prompt analysis */
    int i1 = (int)rint(3.14);
    int i2 = (int)trunc(a);         /* Using const variable */
    checksum += i1 + i2;
    
    /* 2. Two-argument integer-valued functions with constant pairs */
    /* pow with integer result */
    double p1 = pow(2.0, 3.0);      /* 8.0 - integer */
    double p2 = pow(c, 4.0);        /* 16.0 - using const variable */
    
    /* fmod with zero remainder */
    double fm1 = fmod(9.0, 3.0);    /* 0.0 - integer */
    double fm2 = fmod(e, b);        /* 0.0 - using const variables */
    
    /* remainder with zero remainder */
    double rem1 = remainder(10.0, 5.0); /* 0.0 - integer */
    double rem2 = remainder(20.0, 4.0); /* 0.0 - integer */
    
    /* Use in comparisons with integers */
    if (trunc(p1) == 8) {
        checksum += 1;
    }
    
    /* 3. Mixed expressions that require integer-valued analysis */
    /* Array indexing context */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    checksum += arr[2];
    
    /* Binary operations with integers */
    double mixed1 = pow(3.0, 2.0) + 1;  /* 9.0 + 1 = 10.0 */
    double mixed2 = fmod(15.0, 5.0) * 2; /* 0.0 * 2 = 0.0 */
    
    /* 4. Standard library calls (without __builtin_ prefix) */
    /* These will be recognized as built-ins with -fbuiltin */
    double std_rint = rint(2.3);
    double std_pow = pow(5.0, 2.0);
    double std_fmod = fmod(12.0, 6.0);
    
    /* Use results in integer assignments */
    int j1 = (int)std_rint;
    int j2 = (int)std_pow;
    int j3 = (int)std_fmod;
    checksum += j1 + j2 + j3;
    
    /* 5. More complex constant expressions */
    const double pi = 3.141592653589793;
    double complex1 = floor(pi * 10.0);  /* floor(31.4159...) = 31.0 */
    double complex2 = ceil(pi * 7.0);    /* ceil(21.9911...) = 22.0 */
    
    /* Use in conditional context */
    if (round(complex1) == 31) {
        checksum += 31;
    }
    
    /* 6. Edge cases with exact integers */
    double exact1 = rint(4.0);      /* Already integer */
    double exact2 = trunc(9.0);     /* Already integer */
    double exact3 = pow(1.0, 100.0); /* 1.0 - integer */
    
    /* Final computation and output to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2 + (int)n1;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1 + (int)rem2;
    checksum += (int)mixed1 + (int)mixed2;
    checksum += (int)complex1 + (int)complex2;
    checksum += (int)exact1 + (int)exact2 + (int)exact3;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile use to ensure all computations are kept */
    volatile double keep1 = r1;
    volatile double keep2 = p1;
    volatile double keep3 = fm1;
    
    return 0;
}
