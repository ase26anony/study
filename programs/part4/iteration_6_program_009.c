#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int iterations = 3;
    
    /* Results array to prevent dead code elimination */
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant folding cases */
    /* Two-argument calls (pow) */
    results[idx++] = pow(2.0, 3.0);           /* 8.0 - integer result */
    results[idx++] = __builtin_pow(3.0, 2.0); /* 9.0 - integer result */
    results[idx++] = pow(4.0, 0.5);           /* 2.0 - integer result (sqrt via pow) */
    
    /* One-argument calls */
    results[idx++] = sqrt(16.0);              /* 4.0 - integer result */
    results[idx++] = __builtin_sqrt(25.0);    /* 5.0 - integer result */
    results[idx++] = cbrt(27.0);              /* 3.0 - integer result */
    results[idx++] = exp2(4.0);               /* 16.0 - integer result */
    results[idx++] = __builtin_exp2(5.0);     /* 32.0 - integer result */
    results[idx++] = log2(8.0);               /* 3.0 - integer result */
    results[idx++] = __builtin_log2(64.0);    /* 6.0 - integer result */
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    results[idx++] = pow(base, (double)n);    /* 32.0 - integer result */
    results[idx++] = sqrt((double)(n * n));   /* 5.0 - integer result */
    
    /* 4. Assign to integer variables (triggers integer-valued check) */
    int i1 = pow(2.0, 4.0);                   /* 16 - integer result */
    int i2 = __builtin_sqrt(36.0);            /* 6 - integer result */
    int i3 = exp2(3.0);                       /* 8 - integer result */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    /* 5. Use in array indexing */
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int index1 = (int)pow(2.0, 2.0);          /* 4 */
    int index2 = (int)__builtin_sqrt(9.0);    /* 3 */
    results[idx++] = array[index1];
    results[idx++] = array[index2];
    
    /* 6. Compare to integer constants */
    if (pow(2.0, 3.0) == 8) {
        results[idx++] = 1.0;
    }
    if (__builtin_sqrt(49.0) == 7) {
        results[idx++] = 2.0;
    }
    
    /* 7. Loop with invariant constants (conditional constant propagation) */
    double loop_result = 0.0;
    for (int j = 0; j < iterations; j++) {
        /* Loop-invariant computation */
        loop_result += pow(2.0, (double)j);   /* 1 + 2 + 4 = 7 */
    }
    results[idx++] = loop_result;
    
    /* 8. Nested calls */
    results[idx++] = pow(sqrt(64.0), log2(8.0));  /* pow(8.0, 3.0) = 512.0 */
    results[idx++] = exp2(log2(32.0));            /* 32.0 */
    
    /* 9. Trigonometric functions with special cases */
    results[idx++] = sin(0.0);                    /* 0.0 - integer result */
    results[idx++] = __builtin_sin(0.0);          /* 0.0 - integer result */
    results[idx++] = cos(0.0);                    /* 1.0 - integer result */
    results[idx++] = __builtin_cos(0.0);          /* 1.0 - integer result */
    
    /* 10. More complex expressions */
    results[idx++] = pow(2.0, log2(8.0));         /* 8.0 */
    results[idx++] = sqrt(pow(3.0, 2.0));         /* 3.0 */
    
    /* 11. Use in integer context with casts */
    int i4 = (int)pow(10.0, 2.0);                 /* 100 */
    int i5 = (int)__builtin_exp2(6.0);            /* 64 */
    int_results[3] = i4;
    int_results[4] = i5;
    
    /* 12. Check edge cases with one-argument functions */
    results[idx++] = log(1.0);                    /* 0.0 - integer result */
    results[idx++] = __builtin_log(1.0);          /* 0.0 - integer result */
    
    /* 13. Mixed calls in arithmetic expressions */
    double mixed = pow(2.0, 3.0) + sqrt(25.0) - exp2(2.0); /* 8 + 5 - 4 = 9 */
    results[idx++] = mixed;
    
    /* Ensure idx doesn't exceed array bounds */
    if (idx >= ARRAY_SIZE) idx = ARRAY_SIZE - 1;
    
    /* 14. Compute checksum for observable output */
    double checksum = 0.0;
    for (int k = 0; k <= idx; k++) {
        checksum += results[k];
    }
    for (int k = 0; k < 5; k++) {
        checksum += int_results[k];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Integer results: %d, %d, %d, %d, %d\n", 
           int_results[0], int_results[1], int_results[2], 
           int_results[3], int_results[4]);
    
    return 0;
}
