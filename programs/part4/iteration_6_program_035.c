#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int iterations = 3;
    
    /* Results storage */
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant argument calls - will be folded at compile time */
    
    /* Two-argument functions (pow) */
    double r1 = pow(2.0, 3.0);          /* 8.0 - integer result */
    double r2 = __builtin_pow(3.0, 2.0); /* 9.0 - integer result */
    double r3 = pow(4.0, 0.5);          /* 2.0 - integer result (sqrt) */
    
    /* One-argument functions */
    double r4 = sqrt(25.0);             /* 5.0 - integer result */
    double r5 = __builtin_sqrt(16.0);   /* 4.0 - integer result */
    double r6 = exp2(4.0);              /* 16.0 - integer result */
    double r7 = __builtin_exp2(3.0);    /* 8.0 - integer result */
    double r8 = cbrt(27.0);             /* 3.0 - integer result */
    double r9 = log2(8.0);              /* 3.0 - integer result */
    double r10 = __builtin_log2(16.0);  /* 4.0 - integer result */
    
    /* Store in floating-point array */
    fp_results[idx++] = r1;
    fp_results[idx++] = r2;
    fp_results[idx++] = r3;
    fp_results[idx++] = r4;
    fp_results[idx++] = r5;
    fp_results[idx++] = r6;
    fp_results[idx++] = r7;
    fp_results[idx++] = r8;
    fp_results[idx++] = r9;
    fp_results[idx++] = r10;
    
    /* 3. Assign to integer variables - triggers integer-valued check */
    int i1 = pow(2.0, 3.0);            /* Should fold to 8 */
    int i2 = sqrt(49.0);               /* Should fold to 7 */
    int i3 = __builtin_exp2(5.0);      /* Should fold to 32 */
    int i4 = log2(32.0);               /* Should fold to 5 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    int_results[3] = i4;
    
    /* 4. Use in array indexing */
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idx1 = pow(2.0, 2.0);          /* 4 */
    int idx2 = sqrt(9.0);              /* 3 */
    int val1 = array[idx1];            /* Should be 4 */
    int val2 = array[idx2];            /* Should be 3 */
    
    int_results[4] = val1;
    int_results[5] = val2;
    
    /* 5. Compare to integer constants */
    int cmp_results[4] = {0};
    if (pow(3.0, 2.0) == 9) cmp_results[0] = 1;
    if (sqrt(64.0) == 8) cmp_results[1] = 1;
    if (__builtin_exp2(4.0) == 16) cmp_results[2] = 1;
    if (log2(128.0) == 7) cmp_results[3] = 1;
    
    /* 6. Nested calls and combined expressions */
    double complex1 = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    double complex2 = exp2(log2(32.0) / 2.0);                /* sqrt(32) ≈ 5.657, not integer */
    double complex3 = cbrt(pow(2.0, 9.0));                   /* cube root of 512 = 8 */
    
    fp_results[idx++] = complex1;
    fp_results[idx++] = complex2;
    fp_results[idx++] = complex3;
    
    /* 7. Loop with invariant constants - still foldable */
    double loop_sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        /* These arguments are loop-invariant and constant */
        loop_sum += pow(base, (double)n);  /* 2^5 = 32, repeated 3 times = 96 */
    }
    fp_results[idx++] = loop_sum;
    
    /* 8. Trigonometric functions with special cases */
    double trig1 = sin(0.0);      /* 0 - integer result */
    double trig2 = cos(0.0);      /* 1 - integer result */
    double trig3 = __builtin_sin(M_PI);  /* ~0 (should be exactly 0 with -ffast-math) */
    double trig4 = __builtin_cos(0.0);   /* 1 */
    
    fp_results[idx++] = trig1;
    fp_results[idx++] = trig2;
    fp_results[idx++] = trig3;
    fp_results[idx++] = trig4;
    
    /* 9. More one-argument functions */
    double r11 = log(1.0);        /* 0 - integer result */
    double r12 = log10(100.0);    /* 2 - integer result */
    double r13 = __builtin_log(exp(1.0)));  /* 1 - integer result */
    
    fp_results[idx++] = r11;
    fp_results[idx++] = r12;
    fp_results[idx++] = r13;
    
    /* 10. Use in integer context with casts */
    int i5 = (int)pow(5.0, 2.0);      /* 25 */
    int i6 = (int)__builtin_sqrt(81.0); /* 9 */
    int i7 = (int)exp2(6.0);          /* 64 */
    
    int_results[6] = i5;
    int_results[7] = i6;
    int_results[8] = i7;
    
    /* Calculate checksum to prevent dead code elimination */
    double fp_sum = 0.0;
    for (int i = 0; i < idx; i++) {
        fp_sum += fp_results[i];
    }
    
    int int_sum = 0;
    for (int i = 0; i < 9; i++) {
        int_sum += int_results[i];
    }
    
    for (int i = 0; i < 4; i++) {
        int_sum += cmp_results[i];
    }
    
    /* Print checksum - provides observable output */
    printf("Floating-point checksum: %f\n", fp_sum);
    printf("Integer checksum: %d\n", int_sum);
    
    return 0;
}
