/* test.c - Cover integer_valued_real_p lines 15993-16000 in fold-const.cc */
#include <stdio.h>
#include <math.h>

int main(void) {
    /* 1. Declare const variables for constant folding */
    const double a = 10.5;
    const double b = 3.0;
    const double c = 2.0;
    const int i_const = 4;
    
    int checksum = 0;
    
    /* 2. One-argument integer-valued functions with literals */
    double r1 = rint(4.7);           /* Should be 5.0 */
    double t1 = trunc(5.9);          /* Should be 5.0 */
    double f1 = floor(3.2);          /* Should be 3.0 */
    double c1 = ceil(2.3);           /* Should be 3.0 */
    double rd1 = round(6.5);         /* Should be 7.0 */
    
    /* 3. One-argument functions with const variables */
    double r2 = __builtin_rint(a);   /* Should be 11.0 */
    double t2 = __builtin_trunc(a);  /* Should be 10.0 */
    double f2 = __builtin_floor(a);  /* Should be 10.0 */
    double c2 = __builtin_ceil(a);   /* Should be 11.0 */
    
    /* 4. Two-argument functions with constant pairs */
    double p1 = pow(2.0, 3.0);       /* 8.0 - integer result */
    double p2 = __builtin_pow(c, 5.0); /* 32.0 - integer result */
    double fm1 = fmod(9.0, 3.0);     /* 0.0 - integer result */
    double fm2 = __builtin_fmod(14.0, 4.0); /* 2.0 - integer result */
    double rem1 = remainder(10.0, 3.0); /* 1.0 - integer result */
    
    /* 5. Mixed: integer constant arguments (implicit conversion) */
    double p3 = pow(2, i_const);     /* 16.0 - integer result */
    double f3 = floor(4);            /* 4.0 - integer result */
    
    /* 6. Embed in contexts requiring integer-valued analysis */
    /* Assignments to integer types (implicit conversion analysis) */
    int ir1 = rint(3.14);            /* Should become 3 */
    int it1 = trunc(7.89);           /* Should become 7 */
    
    /* Comparisons with integers */
    if (floor(2.8) == 2) {
        checksum += 1;
    }
    if (__builtin_ceil(3.1) == 4) {
        checksum += 2;
    }
    
    /* Array indexing (prompts integer-valued analysis) */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;
    checksum += arr[2];
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 3.0) + 1;   /* 8.0 + 1 = 9.0 */
    double d2 = rint(4.3) * 2;       /* 4.0 * 2 = 8.0 */
    
    /* 7. Use results to avoid dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)rd1;
    checksum += (int)r2 + (int)t2 + (int)f2 + (int)c2;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)p3 + (int)f3;
    checksum += ir1 + it1;
    checksum += (int)d1 + (int)d2;
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
