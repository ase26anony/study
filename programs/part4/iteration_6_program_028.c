#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exponent = 4;
    const double perfect_square = 81.0;
    
    /* Results array to prevent dead code elimination */
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant argument calls */
    /* pow with integer result */
    results[idx++] = pow(2.0, 3.0);           /* 8.0 */
    results[idx++] = __builtin_pow(3.0, 2.0); /* 9.0 */
    
    /* sqrt of perfect squares */
    results[idx++] = sqrt(25.0);              /* 5.0 */
    results[idx++] = __builtin_sqrt(36.0);    /* 6.0 */
    
    /* exp2 with integer exponents */
    results[idx++] = exp2(4.0);               /* 16.0 */
    results[idx++] = __builtin_exp2(5.0);     /* 32.0 */
    
    /* log2 of powers of 2 */
    results[idx++] = log2(8.0);               /* 3.0 */
    results[idx++] = __builtin_log2(64.0);    /* 6.0 */
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    results[idx++] = pow(base, exponent);     /* 16.0 */
    results[idx++] = sqrt(perfect_square);    /* 9.0 */
    
    /* 4. Mixed integer and floating-point contexts */
    /* Assign to integer variables */
    int i1 = pow(2.0, 5.0);                   /* 32 */
    int i2 = __builtin_sqrt(49.0);            /* 7 */
    int i3 = exp2(3.0);                       /* 8 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    /* Use in array indexing */
    double array[10] = {0};
    array[(int)sqrt(16.0)] = 42.0;            /* array[4] = 42.0 */
    array[(int)__builtin_pow(2.0, 2.0)] = 23.0; /* array[4] = 23.0 */
    
    /* Compare to integer in control flow */
    if (pow(3.0, 2.0) == 9) {
        results[idx++] = 1.0;
    }
    
    if (__builtin_sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    /* 5. Nested and combined calls */
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    results[idx++] = exp2(log2(32.0) / 2.0);                /* sqrt(32) ≈ 5.65685 */
    
    /* One-argument functions */
    results[idx++] = __builtin_cbrt(27.0);    /* 3.0 */
    results[idx++] = cos(0.0);                /* 1.0 */
    results[idx++] = __builtin_sin(0.0);      /* 0.0 */
    
    /* Two-argument functions */
    results[idx++] = __builtin_pow(2.0, 3.0); /* 8.0 */
    
    /* 6. Loop with invariant constants */
    double loop_sum = 0.0;
    const int iterations = 3;
    for (int j = 0; j < iterations; j++) {
        /* Loop-invariant computation */
        loop_sum += pow(2.0, j) + sqrt(j * j + 1.0);
    }
    results[idx++] = loop_sum;
    
    /* 7. More complex expressions */
    double complex_expr = pow(2.0, log(8.0) / log(2.0)) * sqrt(9.0);
    results[idx++] = complex_expr;  /* 8 * 3 = 24 */
    
    /* Ensure idx doesn't exceed array bounds */
    if (idx >= ARRAY_SIZE) idx = ARRAY_SIZE - 1;
    
    /* 8. Compute checksum for observable output */
    double sum = 0.0;
    for (int j = 0; j <= idx; j++) {
        sum += results[j];
    }
    
    int int_sum = 0;
    for (int j = 0; j < 3; j++) {
        int_sum += int_results[j];
    }
    
    /* Add array elements */
    for (int j = 0; j < 10; j++) {
        sum += array[j];
    }
    
    printf("Floating-point checksum: %f\n", sum);
    printf("Integer checksum: %d\n", int_sum);
    printf("Total: %f\n", sum + int_sum);
    
    return 0;
}
