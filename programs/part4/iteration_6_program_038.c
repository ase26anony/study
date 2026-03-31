#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int iterations = 3;
    
    /* Variables to store results */
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Constant arguments producing integer results */
    /* Standard library calls */
    fp_results[idx++] = pow(2.0, 3.0);          /* 8.0 */
    fp_results[idx++] = sqrt(9.0);              /* 3.0 */
    fp_results[idx++] = exp2(4.0);              /* 16.0 */
    fp_results[idx++] = cbrt(27.0);             /* 3.0 */
    fp_results[idx++] = log(256.0) / log(2.0);  /* 8.0 */
    
    /* __builtin versions */
    fp_results[idx++] = __builtin_pow(3.0, 2.0);    /* 9.0 */
    fp_results[idx++] = __builtin_sqrt(25.0);       /* 5.0 */
    fp_results[idx++] = __builtin_exp2(5.0);        /* 32.0 */
    fp_results[idx++] = __builtin_cbrt(64.0);       /* 4.0 */
    fp_results[idx++] = __builtin_log2(64.0);       /* 6.0 */
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    fp_results[idx++] = pow(base, n);           /* 2^5 = 32.0 */
    fp_results[idx++] = sqrt((double)(n * n));  /* sqrt(25) = 5.0 */
    fp_results[idx++] = exp2((double)n);        /* 32.0 */
    
    /* 4. Mixed integer and floating-point contexts */
    /* Direct assignment to integer */
    int_results[0] = pow(4.0, 2.0);             /* 16 */
    int_results[1] = sqrt(49.0);                /* 7 */
    int_results[2] = __builtin_exp2(3.0);       /* 8 */
    
    /* Use in array indexing */
    int array[10] = {0};
    array[(int)sqrt(16.0)] = 42;                /* array[4] = 42 */
    array[(int)__builtin_pow(2.0, 2.0)] = 99;   /* array[4] = 99 */
    
    /* Compare to integer */
    if (pow(2.0, 3.0) == 8) {
        int_results[3] = 1;
    }
    if (__builtin_sqrt(81.0) == 9) {
        int_results[4] = 1;
    }
    
    /* 5. Multiple and nested calls */
    fp_results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64.0 */
    fp_results[idx++] = exp2(log2(pow(2.0, 4.0)));             /* 16.0 */
    fp_results[idx++] = sqrt(pow(3.0, 2.0) + pow(4.0, 2.0));   /* 5.0 */
    
    /* 6. Conditional constant propagation in loops */
    for (int i = 0; i < iterations; i++) {
        /* Loop-invariant constants */
        fp_results[idx++] = pow(2.0, (double)i);      /* 1.0, 2.0, 4.0 */
        int_results[5 + i] = __builtin_sqrt((double)((i+1) * (i+1))); /* 1, 2, 3 */
    }
    
    /* 7. One-argument functions */
    fp_results[idx++] = exp2(6.0);      /* 64.0 */
    fp_results[idx++] = sqrt(36.0);     /* 6.0 */
    fp_results[idx++] = cbrt(125.0);    /* 5.0 */
    fp_results[idx++] = __builtin_log(1.0);  /* 0.0 */
    
    /* 8. Two-argument functions */
    fp_results[idx++] = pow(5.0, 2.0);      /* 25.0 */
    fp_results[idx++] = __builtin_pow(6.0, 2.0); /* 36.0 */
    
    /* Ensure we don't exceed array bounds */
    if (idx >= ARRAY_SIZE) idx = ARRAY_SIZE - 1;
    
    /* 9. Compute checksum to prevent dead code elimination */
    double fp_sum = 0.0;
    int int_sum = 0;
    
    for (int i = 0; i < idx; i++) {
        fp_sum += fp_results[i];
    }
    
    for (int i = 0; i < 10; i++) {
        int_sum += int_results[i];
        int_sum += array[i];
    }
    
    /* Print checksums */
    printf("Floating-point checksum: %f\n", fp_sum);
    printf("Integer checksum: %d\n", int_sum);
    
    return 0;
}
