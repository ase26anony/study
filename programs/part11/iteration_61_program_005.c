/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
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
    /* These should trigger arg0 extraction and integer-valued analysis */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double t1 = trunc(5.9);         /* Should be 5.0 */
    double f1 = floor(6.2);         /* Should be 6.0 */
    double c1 = ceil(3.1);          /* Should be 4.0 */
    double r2 = round(2.5);         /* Should be 3.0 */
    
    /* Use __builtin_ prefixed versions explicitly */
    double br1 = __builtin_rint(7.3);   /* Should be 7.0 */
    double bt1 = __builtin_trunc(8.9);  /* Should be 8.0 */
    double bf1 = __builtin_floor(9.1);  /* Should be 9.0 */
    double bc1 = __builtin_ceil(1.1);   /* Should be 2.0 */
    
    /* 2. Two-argument integer-valued functions with constants */
    /* These should trigger both arg0 and arg1 extraction */
    double p1 = pow(2.0, 3.0);          /* 2^3 = 8 (integer) */
    double p2 = pow(c, d);              /* 2^4 = 16 (integer) */
    double fm1 = fmod(9.0, 3.0);        /* 9 % 3 = 0 (integer) */
    double fm2 = fmod(e, f);            /* 9 % 3 = 0 (integer) */
    double rem1 = remainder(10.0, 2.0); /* 10 % 2 = 0 (integer) */
    
    /* __builtin_ versions of two-argument functions */
    double bp1 = __builtin_pow(3.0, 2.0);   /* 3^2 = 9 (integer) */
    double bfm1 = __builtin_fmod(12.0, 4.0); /* 12 % 4 = 0 (integer) */
    
    /* 3. Functions with integer constants (implicit conversion) */
    double f2 = floor(4);              /* Integer constant 4 -> double */
    double p3 = pow(2, 5);             /* 2^5 = 32 (integer) */
    
    /* 4. Embed calls in contexts that require integer-valued analysis */
    
    /* Assignments to integer types (implicit conversion analysis) */
    int i1 = rint(3.14);               /* Should be 3 */
    int i2 = trunc(a);                 /* trunc(10.5) = 10 */
    int i3 = floor(7.8);               /* Should be 7 */
    
    /* Comparisons with integers (may trigger analysis) */
    if (trunc(5.9) == 5) {
        checksum += 1;
    }
    
    if (floor(4.2) == 4) {
        checksum += 2;
    }
    
    /* Array indexing (requires integer value) */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;          /* arr[2] = 5 */
    checksum += arr[2];
    
    /* Binary operations with integers */
    double d1 = pow(2.0, 3.0) + 1;     /* 8 + 1 = 9 */
    double d2 = fmod(15.0, 5.0) * 2;   /* 0 * 2 = 0 */
    
    /* 5. Mixed usage to ensure various paths are taken */
    
    /* Use const variables as arguments */
    double r3 = rint(a);               /* rint(10.5) = 10 or 11 (depends on rounding) */
    double t2 = trunc(a + b);          /* trunc(13.5) = 13 */
    
    /* Chain of integer-valued operations */
    double chain = floor(pow(2.0, 3.0) / 2.0); /* floor(8/2) = floor(4) = 4 */
    
    /* 6. Compute checksum to prevent dead code elimination */
    checksum += (int)r1 + (int)t1 + (int)f1 + (int)c1 + (int)r2;
    checksum += (int)br1 + (int)bt1 + (int)bf1 + (int)bc1;
    checksum += (int)p1 + (int)p2 + (int)fm1 + (int)fm2 + (int)rem1;
    checksum += (int)bp1 + (int)bfm1;
    checksum += (int)f2 + (int)p3;
    checksum += i1 + i2 + i3;
    checksum += (int)d1 + (int)d2;
    checksum += (int)r3 + (int)t2 + (int)chain;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional print to use values and prevent optimization */
    printf("Results: %.1f %.1f %.1f %.1f %.1f\n", r1, p1, fm1, d1, chain);
    
    return 0;
}
