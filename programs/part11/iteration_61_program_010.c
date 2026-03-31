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
    const double d = 4.0;
    const double e = 9.0;
    const double f = 3.0;
    
    /* 1. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);           /* Should be 5.0 */
    double t1 = trunc(5.9);          /* Should be 5.0 */
    double f1 = floor(6.2);          /* Should be 6.0 */
    double c1 = ceil(3.1);           /* Should be 4.0 */
    double r2 = round(2.5);          /* Should be 3.0 */
    double n1 = nearbyint(7.3);      /* Should be 7.0 */
    
    /* 2. One-argument functions with const variables */
    double r3 = rint(a);             /* Should be 11.0 */
    double t2 = trunc(a);            /* Should be 10.0 */
    double f2 = floor(a);            /* Should be 10.0 */
    double c2 = ceil(a);             /* Should be 11.0 */
    
    /* 3. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);       /* 8.0 - integer result */
    double p2 = pow(c, d);           /* 16.0 - integer result */
    double fm1 = fmod(9.0, 3.0);     /* 0.0 - integer result */
    double fm2 = fmod(e, f);         /* 0.0 - integer result */
    double rem1 = remainder(10.0, 3.0); /* 1.0 - integer result */
    
    /* 4. Built-in versions explicitly */
    double br1 = __builtin_rint(8.9);    /* Should be 9.0 */
    double bt1 = __builtin_trunc(7.2);   /* Should be 7.0 */
    double bp1 = __builtin_pow(3.0, 2.0); /* 9.0 - integer result */
    double bf1 = __builtin_floor(5.8);   /* Should be 5.0 */
    
    /* 5. Use results in integer contexts to prompt analysis */
    int i1 = (int)rint(3.14);        /* Implicit conversion */
    int i2 = trunc(8.9);             /* Assignment to int */
    int i3 = floor(4.2);             /* Assignment to int */
    
    /* 6. Comparisons with integers */
    if (trunc(6.7) == 6) {
        checksum += 1;
    }
    
    if (rint(3.5) == 4) {
        checksum += 2;
    }
    
    /* 7. Array indexing with integer-valued results */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;        /* arr[2] = 5 */
    arr[(int)trunc(3.2)] = 7;        /* arr[3] = 7 */
    
    /* 8. Binary operations with integers */
    double d1 = pow(2.0, 4.0) + 1;   /* 16 + 1 = 17 */
    double d2 = fmod(15.0, 4.0) * 2; /* 3 * 2 = 6 */
    
    /* 9. Mixed expressions */
    double mix1 = rint(trunc(9.8) + 1.5); /* rint(9 + 1.5) = rint(10.5) = 11 */
    double mix2 = floor(pow(2.0, 3.0) / 2.0); /* floor(8/2) = floor(4) = 4 */
    
    /* 10. More two-argument function variations */
    double p3 = pow(5.0, 0.0);       /* 1.0 - integer result */
    double fm3 = fmod(17.0, 5.0);    /* 2.0 - integer result */
    double rem2 = remainder(20.0, 6.0); /* 2.0 - integer result */
    
    /* Aggregate results into checksum to prevent optimization */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2 + (int)n1;
    checksum += (int)r3 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)br1 + (int)bt1 + (int)bp1 + (int)bf1;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)mix1 + (int)mix2;
    checksum += (int)p3 + (int)fm3 + (int)rem2;
    checksum += arr[2] + arr[3];
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional calls in return expression context */
    return (int)(rint(checksum / 10.0) + trunc(checksum % 10.0));
}
