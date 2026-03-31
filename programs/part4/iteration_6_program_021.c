#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 4;
    const double exp_double = 4.0;
    
    /* Results storage */
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant arguments producing integer results */
    /* Two-argument calls (pow) */
    double r1 = pow(2.0, 3.0);           /* 8.0 */
    double r2 = __builtin_pow(3.0, 2.0); /* 9.0 */
    
    /* One-argument calls */
    double r3 = sqrt(25.0);              /* 5.0 */
    double r4 = __builtin_sqrt(36.0);    /* 6.0 */
    double r5 = exp2(3.0);               /* 8.0 */
    double r6 = __builtin_exp2(4.0);     /* 16.0 */
    double r7 = cbrt(27.0);              /* 3.0 */
    double r8 = __builtin_cbrt(64.0);    /* 4.0 */
    
    /* Store in arrays */
    fp_results[idx++] = r1;
    fp_results[idx++] = r2;
    fp_results[idx++] = r3;
    fp_results[idx++] = r4;
    fp_results[idx++] = r5;
    fp_results[idx++] = r6;
    fp_results[idx++] = r7;
    fp_results[idx++] = r8;
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    double r9 = pow(base, exp_double);   /* 16.0 */
    double r10 = __builtin_pow(base, n); /* 32.0 */
    
    /* Logarithm of powers */
    double r11 = log(exp(3.0));          /* 3.0 */
    double r12 = __builtin_log2(32.0);   /* 5.0 */
    double r13 = log10(1000.0);          /* 3.0 */
    
    fp_results[idx++] = r9;
    fp_results[idx++] = r10;
    fp_results[idx++] = r11;
    fp_results[idx++] = r12;
    fp_results[idx++] = r13;
    
    /* 4. Assign to integer variables (triggers integer-valued check) */
    int i1 = pow(2.0, 3.0);              /* Should fold to 8 */
    int i2 = __builtin_sqrt(81.0);       /* Should fold to 9 */
    int i3 = exp2(5.0);                  /* Should fold to 32 */
    int i4 = __builtin_cbrt(125.0);      /* Should fold to 5 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    int_results[3] = i4;
    
    /* 5. Use in array indexing */
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idx1 = (int)sqrt(16.0);          /* 4 */
    int idx2 = (int)__builtin_pow(2.0, 2.0); /* 4 */
    int val1 = array[idx1];              /* Should be 4 */
    int val2 = array[idx2];              /* Should be 4 */
    
    int_results[4] = val1;
    int_results[5] = val2;
    
    /* 6. Compare to integer constants */
    int cmp_results[4] = {0};
    if (pow(2.0, 3.0) == 8) {
        cmp_results[0] = 1;
    }
    if (__builtin_sqrt(49.0) == 7) {
        cmp_results[1] = 1;
    }
    if (exp2(4.0) == 16) {
        cmp_results[2] = 1;
    }
    if (__builtin_cbrt(216.0) == 6) {
        cmp_results[3] = 1;
    }
    
    /* 7. Nested calls and complex expressions */
    double r14 = pow(sqrt(64.0), log(8.0) / log(2.0)); /* 8^3 = 512 */
    double r15 = __builtin_exp2(__builtin_log2(32.0)); /* 32 */
    double r16 = sqrt(pow(6.0, 2.0));                  /* 6 */
    
    fp_results[idx++] = r14;
    fp_results[idx++] = r15;
    fp_results[idx++] = r16;
    
    /* 8. Trigonometric functions with special cases */
    double r17 = sin(0.0);               /* 0.0 */
    double r18 = __builtin_cos(0.0);     /* 1.0 */
    double r19 = sin(M_PI);              /* ~0.0 */
    double r20 = __builtin_cos(M_PI/2);  /* ~0.0 */
    
    fp_results[idx++] = r17;
    fp_results[idx++] = r18;
    fp_results[idx++] = r19;
    fp_results[idx++] = r20;
    
    /* 9. Loop with invariant constants (3 iterations) */
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        /* Arguments are constant within loop */
        loop_sum += pow(2.0, j) + sqrt(j * j * 4.0);
    }
    fp_results[idx++] = loop_sum;
    
    /* 10. Mixed integer/float context with atan2 */
    double r21 = atan2(0.0, 1.0);        /* 0.0 */
    double r22 = __builtin_atan2(1.0, 0.0); /* π/2, not integer */
    fp_results[idx++] = r21;
    fp_results[idx++] = r22;
    
    /* Final checksum calculation */
    double fp_sum = 0.0;
    for (int k = 0; k < idx; k++) {
        fp_sum += fp_results[k];
    }
    
    int int_sum = 0;
    for (int k = 0; k < 6; k++) {
        int_sum += int_results[k];
    }
    
    for (int k = 0; k < 4; k++) {
        int_sum += cmp_results[k];
    }
    
    /* Print results to prevent optimization */
    printf("Floating-point sum: %f\n", fp_sum);
    printf("Integer sum: %d\n", int_sum);
    printf("Total: %f\n", fp_sum + int_sum);
    
    return 0;
}
