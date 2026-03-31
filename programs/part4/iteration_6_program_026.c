#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exponent = 4;
    const double perfect_square = 144.0;
    const int loop_iterations = 3;
    
    /* 2. Results storage */
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 3. Direct constant calls - will be folded at compile time */
    /* Two-argument calls (pow, atan2) */
    double r1 = pow(2.0, 3.0);          /* 8.0 - integer result */
    double r2 = __builtin_pow(3.0, 2.0); /* 9.0 - integer result */
    double r3 = pow(base, exponent);    /* 16.0 - integer result */
    
    /* One-argument calls */
    double r4 = sqrt(25.0);             /* 5.0 - integer result */
    double r5 = __builtin_sqrt(36.0);   /* 6.0 - integer result */
    double r6 = cbrt(27.0);             /* 3.0 - integer result */
    double r7 = exp2(4.0);              /* 16.0 - integer result */
    double r8 = __builtin_exp2(5.0);    /* 32.0 - integer result */
    double r9 = log2(8.0);              /* 3.0 - integer result */
    double r10 = __builtin_log2(16.0);  /* 4.0 - integer result */
    
    /* Store floating-point results */
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
    
    /* 4. Assign to integer variables - triggers integer_valued_real_call_p */
    int i1 = pow(2.0, 3.0);             /* Should fold to 8 */
    int i2 = sqrt(49.0);                /* Should fold to 7 */
    int i3 = __builtin_exp2(3.0);       /* Should fold to 8 */
    int i4 = cbrt(64.0);                /* Should fold to 4 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    int_results[3] = i4;
    
    /* 5. Use in array indexing */
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    double val1 = array[(int)sqrt(9.0)];      /* array[3] = 3.0 */
    double val2 = array[(int)__builtin_pow(2.0, 2.0)]; /* array[4] = 4.0 */
    
    fp_results[idx++] = val1;
    fp_results[idx++] = val2;
    
    /* 6. Compare to integer constants */
    int cmp1 = (pow(3.0, 2.0) == 9.0);
    int cmp2 = (__builtin_sqrt(100.0) == 10.0);
    int cmp3 = (exp2(5.0) == 32.0);
    
    int_results[4] = cmp1;
    int_results[5] = cmp2;
    int_results[6] = cmp3;
    
    /* 7. Nested and combined calls */
    double nested1 = pow(sqrt(16.0), log(8.0) / log(2.0)); /* 4^3 = 64 */
    double nested2 = __builtin_exp2(__builtin_log2(32.0)); /* 32 */
    double nested3 = sqrt(pow(6.0, 2.0));                  /* 6 */
    
    fp_results[idx++] = nested1;
    fp_results[idx++] = nested2;
    fp_results[idx++] = nested3;
    
    /* 8. Loop with invariant constants - still foldable */
    double loop_sum = 0.0;
    for (int i = 0; i < loop_iterations; i++) {
        /* These have constant arguments despite being in loop */
        loop_sum += pow(2.0, i);          /* 1 + 2 + 4 = 7 */
        loop_sum += sqrt((double)(i * i * 4)); /* sqrt(0), sqrt(4), sqrt(16) */
    }
    fp_results[idx++] = loop_sum;
    
    /* 9. More one-argument functions */
    double r11 = sin(0.0);               /* 0.0 - integer result */
    double r12 = __builtin_cos(0.0);     /* 1.0 - integer result */
    double r13 = log(1.0);               /* 0.0 - integer result */
    double r14 = __builtin_log10(1.0);   /* 0.0 - integer result */
    
    fp_results[idx++] = r11;
    fp_results[idx++] = r12;
    fp_results[idx++] = r13;
    fp_results[idx++] = r14;
    
    /* 10. Two-argument atan2 with special cases */
    double r15 = atan2(0.0, 1.0);        /* 0.0 - integer result */
    double r16 = __builtin_atan2(0.0, -1.0); /* π, not integer */
    
    fp_results[idx++] = r15;
    fp_results[idx++] = r16;
    
    /* 11. Check integer-valued results in conditionals */
    int count = 0;
    if (pow(2.0, 4.0) == 16.0) count++;          /* true */
    if (sqrt(121.0) == 11.0) count++;            /* true */
    if (__builtin_exp2(6.0) == 64.0) count++;    /* true */
    if (cbrt(125.0) == 5.0) count++;             /* true */
    
    int_results[7] = count;
    
    /* 12. Compute checksum for observable output */
    double fp_sum = 0.0;
    int int_sum = 0;
    
    for (int i = 0; i < idx; i++) {
        fp_sum += fp_results[i];
    }
    
    for (int i = 0; i < 8; i++) {
        int_sum += int_results[i];
    }
    
    /* Print checksums to prevent dead code elimination */
    printf("Floating-point checksum: %f\n", fp_sum);
    printf("Integer checksum: %d\n", int_sum);
    printf("Array element examples: %f, %f\n", val1, val2);
    
    return 0;
}
