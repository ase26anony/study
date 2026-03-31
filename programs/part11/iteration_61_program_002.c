/* Test program to cover integer_valued_real_p built-in function analysis
 * Lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables for use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 5.0;
    const double e = 9.0;
    const double f = 4.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);           /* Should be 5.0 */
    double t1 = trunc(5.9);          /* Should be 5.0 */
    double f1 = floor(6.2);          /* Should be 6.0 */
    double c1 = ceil(3.1);           /* Should be 4.0 */
    double r2 = round(2.5);          /* Should be 3.0 */
    double n1 = nearbyint(7.3);      /* Should be 7.0 */
    
    /* 2. One-argument functions with const variables */
    double r3 = __builtin_rint(a);   /* Should be 11.0 */
    double t2 = __builtin_trunc(a);  /* Should be 10.0 */
    double f2 = __builtin_floor(a);  /* Should be 10.0 */
    double c2 = __builtin_ceil(a);   /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);       /* 2^3 = 8 (integer) */
    double p2 = pow(c, d);           /* 2^5 = 32 (integer) */
    double fm1 = fmod(9.0, 3.0);     /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(e, b);         /* 9 % 3 = 0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* 10 rem 2 = 0 (integer) */
    double rem2 = __builtin_remainder(15.0, 5.0); /* 15 rem 5 = 0 (integer) */
    
    /* 4. Mixed built-in and standard library calls */
    double p3 = __builtin_pow(3.0, 2.0); /* 3^2 = 9 (integer) */
    double p4 = pow(4.0, 2.0);       /* 4^2 = 16 (integer) */
    
    /* 5. Functions in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);             /* Should be 3 */
    int i2 = trunc(8.99);            /* Should be 8 */
    int i3 = floor(9.1);             /* Should be 9 */
    int i4 = (int)ceil(2.01);        /* Should be 3 */
    
    /* Comparisons with integers */
    if (trunc(7.8) == 7) {
        checksum += 1;
    }
    
    if (floor(5.0) == 5) {
        checksum += 2;
    }
    
    /* Array indexing with integer-valued results */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;        /* arr[2] = 5 */
    arr[(int)round(3.2)] = 7;        /* arr[3] = 7 */
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;   /* 16 + 1 = 17 */
    double d2 = fmod(12.0, 4.0) * 2; /* 0 * 2 = 0 */
    
    /* 6. More complex expressions */
    double complex1 = rint(pow(2.0, 3.0) + 0.5); /* rint(8.5) = 9.0 */
    double complex2 = trunc(fmod(17.0, 5.0) * 2.0); /* trunc(2.0 * 2.0) = 4.0 */
    
    /* 7. Using integer constants (implicit conversion to double) */
    double fi1 = floor(4);           /* 4.0 */
    double ci1 = ceil(4);            /* 4.0 */
    double pi1 = pow(2, 5);          /* 32.0 */
    double fm3 = fmod(10, 3);        /* 1.0 */
    
    /* Aggregate results into checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2 + (int)n1;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2;
    checksum += (int)rem1 + (int)rem2 + (int)p3 + (int)p4;
    checksum += i1 + i2 + i3 + i4;
    checksum += (int)d1 + (int)d2;
    checksum += (int)complex1 + (int)complex2;
    checksum += (int)fi1 + (int)ci1 + (int)pi1 + (int)fm3;
    checksum += arr[2] + arr[3];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls to ensure all paths are taken at runtime */
    printf("Results: %.1f %.1f %.1f %.1f\n", 
           rint(1.1), trunc(2.9), floor(3.6), ceil(4.2));
    printf("Pow results: %.1f %.1f\n", pow(5.0, 2.0), __builtin_pow(6.0, 2.0));
    
    return 0;
}
