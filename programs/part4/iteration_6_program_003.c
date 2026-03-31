/* Test program to trigger integer_valued_real_call_p in GCC's fold-const.cc */
#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exponent = 4;
    const double perfect_square = 64.0;
    
    /* Results array to prevent dead code elimination */
    double results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 2. Direct constant argument calls - will fold at compile time */
    /* Two-argument functions (pow, atan2) */
    results[idx++] = pow(3.0, 2.0);           /* 9.0 - integer */
    results[idx++] = __builtin_pow(2.0, 5.0); /* 32.0 - integer */
    results[idx++] = pow(base, 3.0);          /* 8.0 - integer */
    
    /* One-argument functions */
    results[idx++] = sqrt(25.0);              /* 5.0 - integer */
    results[idx++] = __builtin_sqrt(36.0);    /* 6.0 - integer */
    results[idx++] = cbrt(27.0);              /* 3.0 - integer */
    results[idx++] = exp2(4.0);               /* 16.0 - integer */
    results[idx++] = __builtin_exp2(3.0);     /* 8.0 - integer */
    results[idx++] = log2(8.0);               /* 3.0 - integer */
    results[idx++] = __builtin_log2(16.0);    /* 4.0 - integer */
    
    /* 3. Assign to integer variables - triggers integer-valued check */
    int i1 = pow(2.0, 3.0);                   /* 8 */
    int i2 = __builtin_sqrt(49.0);            /* 7 */
    int i3 = exp2(2.0);                       /* 4 */
    
    int_results[0] = i1;
    int_results[1] = i2;
    int_results[2] = i3;
    
    /* 4. Use in array indexing */
    double test_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    results[idx++] = test_array[(int)sqrt(9.0)];      /* test_array[3] = 3 */
    results[idx++] = test_array[(int)__builtin_pow(2.0, 2.0)]; /* test_array[4] = 4 */
    
    /* 5. Compare to integers in control flow */
    if (pow(2.0, n) == 32.0) {
        results[idx++] = 1.0;  /* This branch will be taken */
    }
    
    if (sqrt(perfect_square) == 8.0) {
        results[idx++] = 2.0;  /* This branch will be taken */
    }
    
    /* 6. Loop with invariant constants - still foldable */
    double loop_result = 0.0;
    for (int j = 0; j < 3; j++) {
        /* j is not constant, but 2.0 and 3.0 are */
        loop_result += pow(2.0, 3.0);  /* Adds 8.0 each iteration */
    }
    results[idx++] = loop_result;  /* 24.0 */
    
    /* 7. Nested calls */
    results[idx++] = pow(sqrt(16.0), log2(8.0));  /* pow(4.0, 3.0) = 64.0 */
    results[idx++] = __builtin_sqrt(__builtin_pow(3.0, 2.0));  /* sqrt(9.0) = 3.0 */
    
    /* 8. More mixed contexts */
    double d1 = sin(0.0);          /* 0.0 - integer */
    double d2 = cos(0.0);          /* 1.0 - integer */
    double d3 = __builtin_sin(0.0); /* 0.0 - integer */
    double d4 = __builtin_cos(0.0); /* 1.0 - integer */
    
    results[idx++] = d1;
    results[idx++] = d2;
    results[idx++] = d3;
    results[idx++] = d4;
    
    /* 9. Additional integer-valued calls */
    results[idx++] = pow(10.0, 2.0);      /* 100.0 */
    results[idx++] = sqrt(100.0);         /* 10.0 */
    results[idx++] = cbrt(125.0);         /* 5.0 */
    results[idx++] = exp2(5.0);           /* 32.0 */
    
    /* Ensure we don't exceed array bounds */
    if (idx > ARRAY_SIZE) idx = ARRAY_SIZE;
    
    /* 10. Compute checksum for observable output */
    double sum = 0.0;
    for (int k = 0; k < idx; k++) {
        sum += results[k];
    }
    
    int int_sum = 0;
    for (int k = 0; k < 3; k++) {
        int_sum += int_results[k];
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: double_sum = %.2f, int_sum = %d\n", sum, int_sum);
    
    return 0;
}
