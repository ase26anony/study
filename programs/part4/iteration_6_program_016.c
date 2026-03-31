#include <math.h>
#include <stdio.h>

int main(void) {
    /* 1. Constant integer results from built-in math functions */
    const double c1 = pow(2.0, 3.0);      /* 8.0 - integer */
    const double c2 = sqrt(25.0);         /* 5.0 - integer */
    const double c3 = exp2(4.0);          /* 16.0 - integer */
    const double c4 = __builtin_log2(8.0); /* 3.0 - integer */
    const double c5 = __builtin_cbrt(27.0); /* 3.0 - integer */
    
    /* 2. Symbolic arguments with constant relationships */
    const int n = 5;
    const double s1 = pow(2.0, n);        /* 32.0 - integer */
    const double s2 = __builtin_sqrt(n * n); /* 5.0 - integer */
    
    /* 3. Results used in integer contexts */
    int i1 = pow(3.0, 2.0);               /* 9.0 -> 9 */
    int i2 = sqrt(36.0);                  /* 6.0 -> 6 */
    int i3 = __builtin_exp2(3.0);         /* 8.0 -> 8 */
    
    /* 4. Array indexing with math function results */
    int array[100] = {0};
    array[(int)sqrt(64.0)] = 1;           /* array[8] */
    array[(int)__builtin_pow(2.0, 4.0)] = 2; /* array[16] */
    
    /* 5. Comparisons with integer constants */
    int cmp1 = (pow(4.0, 1.5) == 8.0);    /* 8.0 == 8.0 -> true */
    int cmp2 = (__builtin_log(256.0) / __builtin_log(2.0) == 8.0); /* log2(256) */
    
    /* 6. Loop with invariant math computations */
    double loop_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        /* These arguments are loop-invariant constants */
        loop_sum += pow(2.0, i + 1.0);    /* 2^1, 2^2, 2^3 = 2, 4, 8 */
        loop_sum += sqrt((i + 1.0) * (i + 1.0)); /* 1, 2, 3 */
    }
    
    /* 7. Nested and combined calls */
    double nested = pow(sqrt(81.0), log(8.0) / log(2.0)); /* 9^3 = 729 */
    double combined = __builtin_exp2(__builtin_log2(32.0)) + 
                     __builtin_cbrt(64.0); /* 32 + 4 = 36 */
    
    /* 8. Single-argument functions */
    double single1 = exp2(5.0);           /* 32.0 */
    double single2 = __builtin_sqrt(49.0); /* 7.0 */
    double single3 = log(1.0);            /* 0.0 */
    double single4 = sin(0.0);            /* 0.0 */
    double single5 = cos(0.0);            /* 1.0 */
    
    /* 9. Two-argument functions */
    double two1 = pow(3.0, 2.0);          /* 9.0 */
    double two2 = __builtin_pow(4.0, 0.5); /* 2.0 (sqrt) */
    
    /* 10. Use results to prevent dead code elimination */
    int checksum = (int)c1 + (int)c2 + (int)c3 + (int)c4 + (int)c5 +
                   (int)s1 + (int)s2 + i1 + i2 + i3 +
                   array[8] + array[16] + cmp1 + cmp2 +
                   (int)loop_sum + (int)nested + (int)combined +
                   (int)single1 + (int)single2 + (int)single3 +
                   (int)single4 + (int)single5 + (int)two1 + (int)two2;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional use of results to ensure they're not optimized away */
    if (checksum > 0) {
        printf("All computations performed\n");
    }
    
    return 0;
}
