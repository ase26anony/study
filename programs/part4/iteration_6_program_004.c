/* Test program to trigger integer_valued_real_call_p in GCC's fold-const.cc */
#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 4;
    const double perfect_square = 144.0;
    
    /* Results array to prevent dead code elimination */
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant folding cases */
    /* pow with two constant arguments - integer result */
    results[idx++] = pow(2.0, 3.0);           /* 8.0 */
    results[idx++] = __builtin_pow(3.0, 2.0); /* 9.0 */
    
    /* sqrt of perfect squares */
    results[idx++] = sqrt(25.0);              /* 5.0 */
    results[idx++] = __builtin_sqrt(64.0);    /* 8.0 */
    
    /* exp2 with integer exponents */
    results[idx++] = exp2(4.0);               /* 16.0 */
    results[idx++] = __builtin_exp2(5.0);     /* 32.0 */
    
    /* log2 of powers of 2 */
    results[idx++] = log2(8.0);               /* 3.0 */
    results[idx++] = __builtin_log2(256.0);   /* 8.0 */
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    results[idx++] = pow(base, exp_int);      /* 16.0 */
    results[idx++] = sqrt(perfect_square);    /* 12.0 */
    
    /* 4. Assign to integer variables - triggers integer-valued check */
    int i1 = pow(2.0, 5.0);                   /* 32 */
    int i2 = __builtin_sqrt(81.0);            /* 9 */
    int i3 = exp2(3.0);                       /* 8 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    /* 5. Use in array indexing */
    double test_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(49.0);          /* 7 */
    results[idx++] = test_array[array_idx];   /* 7.0 */
    
    /* 6. Compare to integer constants */
    if (pow(3.0, 2.0) == 9) {
        results[idx++] = 1.0;
    }
    if (__builtin_sqrt(100.0) == 10) {
        results[idx++] = 2.0;
    }
    
    /* 7. Nested calls and complex expressions */
    results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    results[idx++] = exp2(log2(27.0) / 3.0);                /* cube root of 27 = 3 */
    
    /* 8. Loop with invariant constants - promotes constant propagation */
    double loop_sum = 0.0;
    for (int j = 0; j < 3; j++) {
        /* j is not constant, but 2.0 and 3.0 are */
        loop_sum += pow(2.0, 3.0);  /* Always adds 8.0 */
    }
    results[idx++] = loop_sum;  /* 24.0 */
    
    /* 9. One-argument functions */
    results[idx++] = cbrt(27.0);               /* 3.0 */
    results[idx++] = __builtin_cbrt(125.0);    /* 5.0 */
    results[idx++] = cos(0.0);                 /* 1.0 */
    results[idx++] = __builtin_sin(0.0);       /* 0.0 */
    
    /* 10. Two-argument functions */
    results[idx++] = atan2(0.0, 1.0);          /* 0.0 */
    results[idx++] = __builtin_pow(4.0, 0.5);  /* 2.0 */
    
    /* 11. Mixed integer/floating context */
    int mixed = pow(2.0, 4.0) + sqrt(9.0);     /* 16 + 3 = 19 */
    int_results[3] = mixed;
    
    /* Ensure we don't exceed array bounds */
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    /* Calculate checksum of results */
    double sum = 0.0;
    for (int k = 0; k < idx; k++) {
        sum += results[k];
    }
    
    /* Add integer results to checksum */
    for (int k = 0; k < 4; k++) {
        sum += int_results[k];
    }
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %f\n", sum);
    printf("Integer results: %d, %d, %d, %d\n", 
           int_results[0], int_results[1], int_results[2], int_results[3]);
    
    return 0;
}
