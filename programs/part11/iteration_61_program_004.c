/* Test program to cover integer_valued_real_p lines 15993-16000 in fold-const.cc
 * Compile with: gcc -O2 -fno-math-errno -o test test.c
 * Or with: gcc -O3 -ffast-math -o test test.c
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
    
    /* 1. One-argument integer-valued functions with literals */
    /* These should trigger arg0 extraction and integer_valued_real_call_p */
    double r1 = rint(4.7);          /* Should be 5.0 */
    double r2 = trunc(5.9);         /* Should be 5.0 */
    double r3 = floor(3.2);         /* Should be 3.0 */
    double r4 = ceil(2.3);          /* Should be 3.0 */
    double r5 = round(6.5);         /* Should be 7.0 (rounds away from zero) */
    double r6 = nearbyint(1.1);     /* Should be 1.0 */
    
    /* 2. One-argument functions with const variables */
    double r7 = rint(a);            /* Should be 11.0 */
    double r8 = trunc(a);           /* Should be 10.0 */
    double r9 = floor(a);           /* Should be 10.0 */
    double r10 = ceil(a);           /* Should be 11.0 */
    
    /* 3. Two-argument integer-valued functions with literals */
    /* These should trigger both arg0 and arg1 extraction */
    double r11 = pow(2.0, 3.0);     /* Should be 8.0 (integer) */
    double r12 = fmod(9.0, 3.0);    /* Should be 0.0 (integer) */
    double r13 = remainder(10.0, 2.0); /* Should be 0.0 (integer) */
    double r14 = pow(3.0, 2.0);     /* Should be 9.0 (integer) */
    
    /* 4. Two-argument functions with const variables */
    double r15 = pow(c, d);         /* 2^4 = 16.0 (integer) */
    double r16 = fmod(e, b);        /* 9 % 3 = 0.0 (integer) */
    double r17 = remainder(e, b);   /* 9 % 3 = 0.0 (integer) */
    
    /* 5. Using __builtin_ prefixed versions explicitly */
    /* These directly map to GCC internal functions */
    double r18 = __builtin_rint(7.3);      /* Should be 7.0 */
    double r19 = __builtin_trunc(8.9);     /* Should be 8.0 */
    double r20 = __builtin_floor(6.1);     /* Should be 6.0 */
    double r21 = __builtin_ceil(5.01);     /* Should be 6.0 */
    double r22 = __builtin_pow(5.0, 2.0);  /* 25.0 (integer) */
    double r23 = __builtin_fmod(12.0, 4.0); /* 0.0 (integer) */
    
    /* 6. Embed calls in contexts that require integer-valued analysis */
    /* Assignments to integer types (implicit conversion) */
    int i1 = rint(3.14);            /* Should be 3 */
    int i2 = trunc(4.99);           /* Should be 4 */
    int i3 = floor(7.8);            /* Should be 7 */
    
    /* Comparisons with integers */
    if (trunc(3.7) == 3) {
        checksum += 1;
    }
    
    if (floor(9.2) == 9) {
        checksum += 2;
    }
    
    /* Array indexing */
    int arr[10] = {0};
    arr[(int)floor(2.8)] = 5;       /* arr[2] = 5 */
    
    /* Binary operations with integers */
    double r24 = pow(2.0, 4.0) + 1; /* 16 + 1 = 17 */
    double r25 = fmod(15.0, 5.0) * 2; /* 0 * 2 = 0 */
    
    /* 7. Mixed expressions to ensure all paths are exercised */
    double r26 = rint(__builtin_pow(2.0, 5.0)); /* rint(32.0) = 32.0 */
    double r27 = trunc(fmod(20.0, 6.0));        /* trunc(2.0) = 2.0 */
    
    /* 8. Use results in computations to prevent dead code elimination */
    checksum += (int)r1 + (int)r2 + (int)r3 + (int)r4 + (int)r5;
    checksum += (int)r6 + (int)r7 + (int)r8 + (int)r9 + (int)r10;
    checksum += (int)r11 + (int)r12 + (int)r13 + (int)r14 + (int)r15;
    checksum += (int)r16 + (int)r17 + (int)r18 + (int)r19 + (int)r20;
    checksum += (int)r21 + (int)r22 + (int)r23 + (int)r24 + (int)r25;
    checksum += (int)r26 + (int)r27;
    checksum += i1 + i2 + i3;
    checksum += arr[2];  /* Should be 5 */
    
    printf("Result checksum: %d\n", checksum);
    printf("Expected: 5 + 5 + 3 + 3 + 7 + 1 + 11 + 10 + 10 + 11 + ", checksum);
    printf("8 + 0 + 0 + 9 + 16 + 0 + 0 + 7 + 8 + 6 + 6 + 25 + 0 + 17 + 0 + ");
    printf("32 + 2 + 3 + 4 + 7 + 5 = 211\n");
    
    return 0;
}
