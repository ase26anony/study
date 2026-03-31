/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
 */

#include <stdio.h>
#include <math.h>

int main(void) {
    int checksum = 0;
    
    /* Constant variables to use as arguments */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const double d = 4.0;
    const double e = 9.0;
    const double f = 3.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(7.3);          /* Should be 8.0 */
    double r2 = round(8.6);         /* Should be 9.0 */
    
    /* 2. One-argument functions with const variables */
    double r3 = __builtin_rint(a);  /* Should be 11.0 */
    double t2 = __builtin_trunc(a); /* Should be 10.0 */
    double f2 = __builtin_floor(a); /* Should be 10.0 */
    double c2 = __builtin_ceil(a);  /* Should be 11.0 */
    
    /* 3. Two-argument functions with literals */
    double p1 = pow(2.0, 3.0);      /* Should be 8.0 (integer) */
    double p2 = pow(3.0, 2.0);      /* Should be 9.0 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* Should be 0.0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* Should be 0.0 (integer) */
    
    /* 4. Two-argument functions with const variables */
    double p3 = __builtin_pow(c, d);    /* 2^4 = 16.0 (integer) */
    double fm2 = __builtin_fmod(e, f);  /* 9 % 3 = 0.0 (integer) */
    double rem2 = __builtin_remainder(e, f); /* Should be 0.0 (integer) */
    
    /* 5. Integer-valued functions in integer contexts */
    int i1 = rint(3.14);            /* Implicit conversion to int */
    int i2 = trunc(7.89);           /* Implicit conversion to int */
    int i3 = (int)floor(2.8);       /* Explicit cast */
    
    /* 6. Functions in conditional expressions */
    if (trunc(4.5) == 4) {
        checksum += 1;
    }
    
    if (rint(5.5) == 6) {
        checksum += 2;
    }
    
    /* 7. Functions in array indexing context */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    arr[(int)ceil(2.2)] = 7;        /* arr[3] = 7 */
    
    /* 8. Functions in arithmetic with integers */
    double arith1 = pow(2.0, 3.0) + 1;      /* 8.0 + 1 = 9.0 */
    double arith2 = fmod(15.0, 5.0) * 2;    /* 0.0 * 2 = 0.0 */
    double arith3 = remainder(12.0, 3.0) - 1; /* 0.0 - 1 = -1.0 */
    
    /* 9. Nearbyint function (another integer-valued real function) */
    double n1 = nearbyint(3.7);     /* Should be 4.0 */
    double n2 = __builtin_nearbyint(6.2); /* Should be 6.0 */
    
    /* 10. Mixed calls to ensure both argument extraction paths */
    /* Single argument extraction */
    double single1 = rint(1.1);
    double single2 = trunc(2.2);
    double single3 = floor(3.3);
    double single4 = ceil(4.4);
    double single5 = round(5.5);
    
    /* Two argument extraction */
    double double1 = pow(1.5, 2.0);     /* 2.25 (not integer, but still calls the function) */
    double double2 = fmod(7.0, 2.5);    /* 2.0 (integer result) */
    double double3 = remainder(8.0, 3.0); /* 2.0 (integer result) */
    
    /* 11. Use integer constants that convert to double */
    double ic1 = floor(4);          /* Integer constant 4 -> double */
    double ic2 = ceil(5);           /* Integer constant 5 -> double */
    double ic3 = pow(2, 5);         /* 2^5 = 32.0 (integer) */
    
    /* Aggregate results into checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)rem1;
    checksum += (int)p3 + (int)fm2 + (int)rem2;
    checksum += i1 + i2 + i3;
    checksum += (int)arith1 + (int)arith2 + (int)arith3;
    checksum += (int)n1 + (int)n2;
    checksum += (int)single1 + (int)single2 + (int)single3 + (int)single4 + (int)single5;
    checksum += (int)double1 + (int)double2 + (int)double3;
    checksum += (int)ic1 + (int)ic2 + (int)ic3;
    checksum += arr[2] + arr[3];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test: Use in switch context (though constant folding may happen earlier) */
    switch ((int)round(3.7)) {
        case 4:
            printf("Round test passed\n");
            break;
        default:
            printf("Round test failed\n");
    }
    
    return 0;
}
