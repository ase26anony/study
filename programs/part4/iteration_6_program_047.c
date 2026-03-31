#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    /* 1. Constant arguments producing integer results */
    const double two = 2.0;
    const int three = 3;
    const int four = 4;
    
    /* Direct calls with constant arguments */
    double d1 = pow(2.0, 3.0);          /* 8.0 - integer */
    double d2 = sqrt(9.0);              /* 3.0 - integer */
    double d3 = exp2(4.0);              /* 16.0 - integer */
    double d4 = cbrt(27.0);             /* 3.0 - integer */
    double d5 = log(256.0) / log(2.0);  /* 8.0 - integer (log2 via change of base) */
    
    /* __builtin_ versions */
    double d6 = __builtin_pow(3.0, 2.0);    /* 9.0 - integer */
    double d7 = __builtin_sqrt(16.0);       /* 4.0 - integer */
    double d8 = __builtin_exp2(5.0);        /* 32.0 - integer */
    
    /* 2. Symbolic arguments with constant relationships */
    const int n = 6;
    double d9 = pow(two, n);            /* 64.0 - integer (two is 2.0, n is const 6) */
    double d10 = sqrt((double)(n * n)); /* 6.0 - integer */
    
    /* 3. Assign to integer variables (triggers integer-valued check) */
    int i1 = pow(2.0, 4.0);             /* 16 - integer conversion */
    int i2 = sqrt(25.0);                /* 5 - integer conversion */
    int i3 = __builtin_exp2(3.0);       /* 8 - integer conversion */
    
    /* 4. Use in array indexing */
    array[(int)sqrt(36.0)] = 1;         /* array[6] = 1 */
    array[(int)pow(2.0, 2.0)] = 2;      /* array[4] = 2 */
    
    /* 5. Compare to integers */
    int cmp1 = (pow(2.0, 3.0) == 8.0);
    int cmp2 = (sqrt(49.0) == 7);
    int cmp3 = (__builtin_exp2(4.0) == 16);
    
    /* 6. Mixed one and two argument functions */
    double d11 = atan2(0.0, 1.0);       /* 0.0 - integer (two args) */
    double d12 = fmod(15.0, 5.0);       /* 0.0 - integer (two args) */
    double d13 = __builtin_sin(0.0);    /* 0.0 - integer (one arg) */
    double d14 = __builtin_cos(0.0);    /* 1.0 - integer (one arg) */
    
    /* 7. Nested calls and complex expressions */
    double d15 = pow(sqrt(64.0), log(8.0) / log(2.0));  /* 8^3 = 512 */
    double d16 = exp2(sqrt(9.0));                       /* 2^3 = 8 */
    double d17 = cbrt(pow(2.0, 9.0));                   /* 2^3 = 8 */
    
    /* 8. Loop with invariant constants (for conditional constant propagation) */
    double loop_sum = 0.0;
    for (int i = 0; i < 3; ++i) {
        /* i is not constant, but these expressions use constants */
        double loop_val = pow(2.0, (double)i) * sqrt(4.0);  /* sqrt(4.0) = 2.0 (constant) */
        loop_sum += loop_val;
        
        /* Constant argument through loop-invariant */
        const double base = 3.0;
        double loop_val2 = __builtin_pow(base, 2.0);  /* 9.0 - constant in loop */
        loop_sum += loop_val2;
    }
    
    /* 9. More integer context usage */
    int i4 = (int)pow(sqrt(100.0), 2.0);  /* (10)^2 = 100 */
    int i5 = (int)exp2(log2(32.0));       /* 32 */
    
    /* 10. Trigonometric functions with integer results at specific points */
    double d18 = sin(0.0);     /* 0.0 */
    double d19 = cos(0.0);     /* 1.0 */
    double d20 = tan(0.0);     /* 0.0 */
    double d21 = asin(0.0);    /* 0.0 */
    double d22 = acos(1.0);    /* 0.0 */
    double d23 = atan(0.0);    /* 0.0 */
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                     i1 + i2 + i3 + cmp1 + cmp2 + cmp3 + d11 + d12 + d13 + d14 +
                     d15 + d16 + d17 + loop_sum + i4 + i5 + d18 + d19 + d20 +
                     d21 + d22 + d23;
    
    /* Use array elements */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        checksum += array[i];
    }
    
    printf("Result: %f\n", checksum);
    printf("Integer conversions: %d, %d, %d, %d, %d\n", i1, i2, i3, i4, i5);
    printf("Comparisons: %d, %d, %d\n", cmp1, cmp2, cmp3);
    
    return (int)checksum % 256;
}
