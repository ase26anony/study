/* Test program to trigger integer_valued_real_call_p in GCC's fold-const.cc */
#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20
static int array[ARRAY_SIZE] = {0};

int main(void) {
    /* 1. Constant arguments producing integer results */
    const double two = 2.0;
    const int three = 3;
    const int four = 4;
    
    /* Direct assignments to integer variables */
    int i1 = pow(2.0, 3.0);               /* 8 */
    int i2 = __builtin_pow(3.0, 2.0);     /* 9 */
    int i3 = sqrt(16.0);                  /* 4 */
    int i4 = __builtin_sqrt(25.0);        /* 5 */
    int i5 = exp2(3.0);                   /* 8 */
    int i6 = __builtin_exp2(4.0);         /* 16 */
    int i7 = cbrt(27.0);                  /* 3 */
    int i8 = __builtin_cbrt(64.0);        /* 4 */
    
    /* 2. Symbolic arguments with constant relationships */
    const int n = 5;
    double base = 2.0;
    int i9 = pow(base, n);                /* 32 */
    int i10 = __builtin_pow(3.0, n-2);    /* 27 */
    
    /* 3. Use in integer array indexing */
    array[(int)sqrt(9.0)] = 100;
    array[(int)__builtin_pow(2.0, 3.0)] = 200;
    array[(int)exp2(2.0)] = 300;
    
    /* 4. Compare to integer constants */
    int cmp1 = (pow(2.0, 4.0) == 16);
    int cmp2 = (__builtin_sqrt(36.0) == 6);
    int cmp3 = (exp2(5.0) == 32);
    
    /* 5. Mixed integer and floating-point contexts */
    double d1 = pow(2.0, 3.0);            /* 8.0 */
    double d2 = __builtin_sqrt(49.0);     /* 7.0 */
    double d3 = exp2(6.0);                /* 64.0 */
    
    /* Convert to integer after computation */
    int i11 = (int)d1;
    int i12 = (int)d2;
    int i13 = (int)d3;
    
    /* 6. Nested calls and combined expressions */
    double val1 = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    double val2 = __builtin_exp2(__builtin_log2(32.0));  /* 32 */
    double val3 = cbrt(pow(2.0, 9.0));                   /* 8 */
    
    int i14 = (int)val1;
    int i15 = (int)val2;
    int i16 = (int)val3;
    
    /* 7. Loop with invariant constants */
    int sum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        /* Loop-invariant computations that should fold */
        double loop_val = pow(2.0, iter + 1);  /* 2, 4, 8 */
        sum += (int)loop_val;
        
        /* Use __builtin_ version inside loop */
        double loop_val2 = __builtin_sqrt((double)((iter + 1) * 4));  /* sqrt(4), sqrt(8), sqrt(12) */
        sum += (int)loop_val2;
    }
    
    /* 8. One-argument functions */
    double one_arg1 = exp2(4.0);          /* 16 */
    double one_arg2 = __builtin_log(exp(1.0));  /* 1 */
    double one_arg3 = sin(0.0);           /* 0 */
    double one_arg4 = __builtin_cos(0.0); /* 1 */
    
    int i17 = (int)one_arg1;
    int i18 = (int)one_arg2;
    int i19 = (int)one_arg3;
    int i20 = (int)one_arg4;
    
    /* 9. Two-argument functions */
    double two_arg1 = pow(2.0, 5.0);      /* 32 */
    double two_arg2 = __builtin_pow(3.0, 3.0);  /* 27 */
    
    int i21 = (int)two_arg1;
    int i22 = (int)two_arg2;
    
    /* 10. Complex expression mixing everything */
    int complex_result = (int)(pow(sqrt(81.0), 1.5) + exp2(log2(8.0)) - cbrt(125.0));
    
    /* Final checksum to prevent dead code elimination */
    int checksum = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
                   i11 + i12 + i13 + i14 + i15 + i16 + i17 + i18 + i19 + i20 +
                   i21 + i22 + complex_result + sum + cmp1 + cmp2 + cmp3;
    
    /* Use array elements to prevent elimination */
    for (int j = 0; j < ARRAY_SIZE; ++j) {
        checksum += array[j];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}
