/* Test program to cover integer_valued_real_p lines in GCC's fold-const.cc
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
    const double d = 4.0;
    const double e = 9.0;
    const double f = 3.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double r2 = round(2.5);         /* Should be 3.0 */
    
    /* 2. One-argument functions with const variables */
    double r3 = __builtin_rint(a);  /* Should be 11.0 */
    double t2 = __builtin_trunc(a); /* Should be 10.0 */
    double f2 = __builtin_floor(a); /* Should be 10.0 */
    double c2 = __builtin_ceil(a);  /* Should be 11.0 */
    
    /* 3. Two-argument functions with literals */
    double p1 = pow(2.0, 3.0);      /* Should be 8.0 (integer) */
    double fm1 = fmod(9.0, 3.0);    /* Should be 0.0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* Should be 0.0 (integer) */
    
    /* 4. Two-argument functions with const variables */
    double p2 = __builtin_pow(c, d);    /* 2^4 = 16.0 (integer) */
    double fm2 = __builtin_fmod(e, f);  /* 9 % 3 = 0.0 (integer) */
    double rem2 = __builtin_remainder(14.0, 7.0); /* Should be 0.0 */
    
    /* 5. Integer conversions that may prompt integer-valued analysis */
    int i1 = (int)rint(3.14);       /* Implicit conversion */
    int i2 = (int)trunc(7.89);
    int i3 = floor(4.0);            /* Direct assignment to int */
    
    /* 6. Use in comparisons with integers */
    if (trunc(3.7) == 3) {
        checksum += 1;
    }
    
    if (floor(5.1) == 5) {
        checksum += 2;
    }
    
    /* 7. Use as array indices */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    arr[(int)round(3.2)] = 7;       /* arr[3] = 7 */
    
    /* 8. Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;  /* 16 + 1 = 17 */
    double d2 = fmod(15.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* 9. Mixed expressions */
    double complex1 = rint(trunc(floor(ceil(2.3)))); /* Nested calls */
    double complex2 = pow(floor(4.8), 2.0); /* floor(4.8)=4, 4^2=16 */
    
    /* 10. nearbyint function (another integer-valued real function) */
    double n1 = nearbyint(3.5);     /* Should be 4.0 */
    double n2 = __builtin_nearbyint(2.3); /* Should be 2.0 */
    
    /* 11. Use in switch context (simulated) */
    int val = (int)round(2.6);      /* val = 3 */
    switch (val) {
        case 3: checksum += 4; break;
        default: break;
    }
    
    /* 12. More two-argument cases with mixed constant types */
    double p3 = pow(3, 2);          /* Integer constants: 3^2 = 9 */
    double fm3 = fmod(10, 4);       /* 10 % 4 = 2 */
    double p4 = __builtin_pow(5.0, 0.0); /* 5^0 = 1 */
    
    /* Aggregate results to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)fm1 + (int)rem1;
    checksum += (int)p2 + (int)fm2 + (int)rem2;
    checksum += i1 + i2 + i3;
    checksum += arr[2] + arr[3];
    checksum += (int)d1 + (int)d2;
    checksum += (int)complex1 + (int)complex2;
    checksum += (int)n1 + (int)n2;
    checksum += (int)p3 + (int)fm3 + (int)p4;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional print to use values and prevent optimization */
    printf("Results: %.1f %.1f %.1f %.1f %.1f\n", r1, p1, fm1, d1, complex2);
    
    return 0;
}
