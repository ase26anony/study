#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main(void) {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exponent = 4;
    const double perfect_square = 81.0;
    const int iterations = 3;
    
    /* 2. Results storage */
    double fp_results[ARRAY_SIZE] = {0};
    int int_results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    /* 3. Direct constant argument calls - will be folded at compile time */
    fp_results[idx++] = pow(2.0, 3.0);          /* 8.0 - integer result */
    fp_results[idx++] = sqrt(25.0);             /* 5.0 - integer result */
    fp_results[idx++] = exp2(4.0);              /* 16.0 - integer result */
    fp_results[idx++] = cbrt(27.0);             /* 3.0 - integer result */
    fp_results[idx++] = log2(8.0);              /* 3.0 - integer result */
    
    /* 4. Using __builtin_ versions explicitly */
    fp_results[idx++] = __builtin_pow(3.0, 2.0);    /* 9.0 */
    fp_results[idx++] = __builtin_sqrt(36.0);       /* 6.0 */
    fp_results[idx++] = __builtin_exp2(5.0);        /* 32.0 */
    
    /* 5. Symbolic arguments with constant relationships */
    fp_results[idx++] = pow(base, exponent);        /* 16.0 */
    fp_results[idx++] = sqrt(perfect_square);       /* 9.0 */
    
    /* 6. Assign to integer variables - triggers integer-valued check */
    int i1 = pow(2.0, 3.0);      /* Should fold to 8 */
    int i2 = sqrt(49.0);         /* Should fold to 7 */
    int i3 = exp2(3.0);          /* Should fold to 8 */
    int_results[0] = i1 + i2 + i3;
    
    /* 7. Use in array indexing */
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int array_idx = (int)sqrt(64.0);  /* Should fold to 8 */
    fp_results[idx++] = array[array_idx];
    
    /* 8. Compare to integer */
    int count = 0;
    if (pow(2.0, 4.0) == 16) count++;      /* Should fold to true */
    if (sqrt(100.0) == 10) count++;        /* Should fold to true */
    if (exp2(5.0) == 32) count++;          /* Should fold to true */
    int_results[1] = count;
    
    /* 9. Loop with invariant constants - still foldable */
    double loop_sum = 0.0;
    for (int j = 0; j < iterations; j++) {
        /* These arguments are loop-invariant */
        loop_sum += pow(2.0, j);           /* j is constant in each iteration */
        loop_sum += sqrt((double)(j * j)); /* Perfect square when j is integer */
    }
    fp_results[idx++] = loop_sum;
    
    /* 10. Nested calls and combined expressions */
    fp_results[idx++] = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    fp_results[idx++] = exp2(log2(32.0));                      /* 32 */
    fp_results[idx++] = cbrt(pow(3.0, 3.0));                   /* 3 */
    
    /* 11. Single argument functions */
    fp_results[idx++] = sin(0.0);          /* 0.0 - integer result */
    fp_results[idx++] = cos(0.0);          /* 1.0 - integer result */
    fp_results[idx++] = __builtin_sin(0.0); /* 0.0 */
    fp_results[idx++] = __builtin_cos(0.0); /* 1.0 */
    
    /* 12. More complex integer-valued cases */
    fp_results[idx++] = pow(4.0, 1.5);     /* 4^1.5 = 8 - integer result */
    fp_results[idx++] = sqrt(pow(6.0, 2.0)); /* 6 - integer result */
    
    /* 13. Conditional with constant propagation */
    int x = 3;
    if (x > 2) {
        /* x is known to be 3 here for constant propagation */
        fp_results[idx++] = pow(2.0, (double)x);  /* 8.0 */
    }
    
    /* 14. Compute checksum to prevent dead code elimination */
    double checksum_fp = 0.0;
    int checksum_int = 0;
    
    for (int k = 0; k < idx && k < ARRAY_SIZE; k++) {
        checksum_fp += fp_results[k];
    }
    
    for (int k = 0; k < 2; k++) {
        checksum_int += int_results[k];
    }
    
    /* 15. Print results to ensure execution */
    printf("Floating-point checksum: %f\n", checksum_fp);
    printf("Integer checksum: %d\n", checksum_int);
    printf("Array element accessed: %f\n", array[array_idx]);
    
    return 0;
}
