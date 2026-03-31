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
    
    /* 2. Direct constant argument calls */
    /* Two-argument functions (pow) */
    double r1 = pow(2.0, 3.0);           /* 8.0 - integer */
    double r2 = __builtin_pow(3.0, 2.0); /* 9.0 - integer */
    
    /* One-argument functions */
    double r3 = sqrt(25.0);              /* 5.0 - integer */
    double r4 = __builtin_sqrt(16.0);    /* 4.0 - integer */
    double r5 = exp2(3.0);               /* 8.0 - integer */
    double r6 = __builtin_exp2(4.0);     /* 16.0 - integer */
    double r7 = cbrt(27.0);              /* 3.0 - integer */
    double r8 = __builtin_cbrt(8.0);     /* 2.0 - integer */
    
    /* Store results */
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
    double r9 = pow(base, exp_double);   /* 16.0 - integer */
    double r10 = __builtin_pow(base, n); /* 32.0 - integer */
    
    /* Logarithm of powers */
    double r11 = log(8.0) / log(2.0);    /* 3.0 - integer */
    double r12 = __builtin_log2(8.0);    /* 3.0 - integer */
    
    fp_results[idx++] = r9;
    fp_results[idx++] = r10;
    fp_results[idx++] = r11;
    fp_results[idx++] = r12;
    
    /* 4. Assign to integer variables (triggers integer-valued check) */
    int i1 = pow(2.0, 3.0);              /* Should fold to 8 */
    int i2 = __builtin_sqrt(49.0);       /* Should fold to 7 */
    int i3 = exp2(5.0);                  /* Should fold to 32 */
    int i4 = __builtin_cbrt(64.0);       /* Should fold to 4 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    int_results[3] = i4;
    
    /* 5. Use in array indexing */
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idx1 = (int)sqrt(36.0);          /* 6 */
    int idx2 = (int)__builtin_pow(2.0, 2.0); /* 4 */
    int val1 = array[idx1];              /* Should be 6 */
    int val2 = array[idx2];              /* Should be 4 */
    
    int_results[4] = val1;
    int_results[5] = val2;
    
    /* 6. Compare to integer constants */
    int cmp_results[4] = {0};
    if (pow(2.0, 3.0) == 8) {
        cmp_results[0] = 1;
    }
    if (__builtin_sqrt(81.0) == 9) {
        cmp_results[1] = 1;
    }
    if (exp2(4.0) == 16) {
        cmp_results[2] = 1;
    }
    if (__builtin_cbrt(125.0) == 5) {
        cmp_results[3] = 1;
    }
    
    /* 7. Loop with invariant arguments (constant propagation) */
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        /* i is not constant, but 2.0 and 3.0 are */
        loop_sum += pow(2.0, 3.0);       /* Always adds 8.0 */
        loop_sum += __builtin_sqrt(9.0); /* Always adds 3.0 */
    }
    fp_results[idx++] = loop_sum;        /* Should be 33.0 */
    
    /* 8. Nested calls */
    double nested = pow(sqrt(16.0), log(8.0) / log(2.0)); /* 4^3 = 64 */
    double nested2 = __builtin_exp2(__builtin_log2(32.0)); /* 32 */
    
    fp_results[idx++] = nested;
    fp_results[idx++] = nested2;
    
    /* 9. Trigonometric functions with special cases */
    double r13 = sin(0.0);               /* 0.0 - integer */
    double r14 = __builtin_cos(0.0);     /* 1.0 - integer */
    double r15 = sin(3.14159265358979323846); /* ~0.0 */
    
    fp_results[idx++] = r13;
    fp_results[idx++] = r14;
    fp_results[idx++] = r15;
    
    /* 10. Mixed integer/float context with atan2 */
    double r16 = atan2(0.0, 1.0);        /* 0.0 - integer */
    fp_results[idx++] = r16;
    
    /* Calculate checksum */
    double fp_sum = 0.0;
    for (int j = 0; j < idx; j++) {
        fp_sum += fp_results[j];
    }
    
    int int_sum = 0;
    for (int j = 0; j < 6; j++) {
        int_sum += int_results[j];
    }
    
    int cmp_sum = 0;
    for (int j = 0; j < 4; j++) {
        cmp_sum += cmp_results[j];
    }
    
    /* Final output to prevent elimination */
    printf("Floating-point sum: %f\n", fp_sum);
    printf("Integer sum: %d\n", int_sum);
    printf("Comparison successes: %d\n", cmp_sum);
    printf("Total: %f\n", fp_sum + int_sum + cmp_sum);
    
    return 0;
}
