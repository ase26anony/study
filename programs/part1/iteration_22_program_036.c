/* test_nan_comparisons.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    switch (opcode % 4) {
        case 0: /* UNEQ_EXPR: unordered or equal */
            /* Using GCC builtins for unordered comparisons */
            if (__builtin_isunordered(a, b) || __builtin_isgreater(a, b) || 
                __builtin_isless(a, b) || a == b) {
                result = 1;
                sink += 1;
            }
            /* Also test with float types */
            if (__builtin_isunordered(c, d) || c == d) {
                result |= 2;
                sink += 2;
            }
            break;
            
        case 1: /* LTGT_EXPR: less than or greater than (ordered and not equal) */
            /* Ordered comparison: not unordered AND not equal */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
                sink += 3;
            }
            /* Alternative using comparison macros */
            if (isgreater(a, b) || isless(a, b)) {
                result |= 2;
                sink += 4;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isnan(a) && __builtin_isnan(b)) {
                result = 3;
                sink += 5;
            }
            if (isless(c, d) && !__builtin_isnan(c) && !__builtin_isnan(d)) {
                result |= 4;
                sink += 6;
            }
            break;
            
        case 3: /* More complex expressions */
            /* This should generate UNEQ_EXPR or LTGT_EXPR depending on optimization */
            result = (__builtin_isunordered(a, b) || a == b) ? 5 : 0;
            result += (!__builtin_isunordered(c, d) && c != d) ? 10 : 0;
            sink += result;
            break;
    }
    
    /* Ensure sink is used */
    if (sink > 1000) {
        printf("Impossible\n");
    }
    
    return result;
}

/* Another non-inlineable function to increase chances */
static __attribute__((noinline, optimize("no-tree-vectorize")))
int compare_with_nans(double x, double y, int mode) {
    volatile double vsink = 0.0;
    int res = 0;
    
    /* Generate UNEQ_EXPR */
    if (mode == 0) {
        int temp = (__builtin_isunordered(x, y) || x == y);
        res = temp;
        vsink = x + y;
    }
    /* Generate LTGT_EXPR */
    else if (mode == 1) {
        int temp = (!__builtin_isunordered(x, y) && x != y);
        res = temp;
        vsink = x - y;
    }
    /* Mixed */
    else {
        res = (isgreater(x, y) || isless(x, y) || __builtin_isunordered(x, y));
        vsink = x * y;
    }
    
    return res + (int)(vsink * 0);
}

int main(void) {
    /* Initialize test values */
    double nan_dbl = 0.0 / 0.0;           /* Generate NaN */
    double inf_dbl = __builtin_inf();     /* Infinity */
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    double neg_zero_dbl = -0.0;
    
    /* Explicit NaN using builtin */
    double explicit_nan = __builtin_nan("");
    double signaling_nan = __builtin_nans("1");
    
    /* Float versions */
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Array of test pairs */
    struct {
        double a;
        double b;
        float c;
        float d;
    } test_pairs[] = {
        {nan_dbl, normal_dbl, nan_flt, normal_flt},
        {normal_dbl, nan_dbl, normal_flt, nan_flt},
        {nan_dbl, nan_dbl, nan_flt, nan_flt},
        {inf_dbl, neg_inf_dbl, inf_flt, -inf_flt},
        {normal_dbl, zero_dbl, normal_flt, zero_flt},
        {zero_dbl, neg_zero_dbl, zero_flt, -zero_flt},
        {explicit_nan, signaling_nan, nan_flt, nan_flt},
        {inf_dbl, normal_dbl, inf_flt, normal_flt},
    };
    
    int checksum = 0;
    int num_pairs = sizeof(test_pairs) / sizeof(test_pairs[0]);
    
    /* Test with various opcodes and values */
    for (int i = 0; i < num_pairs; i++) {
        for (int op = 0; op < 8; op++) {
            /* Call test function */
            int res1 = test_nan_comparisons(
                op, 
                test_pairs[i].a,
                test_pairs[i].b,
                test_pairs[i].c,
                test_pairs[i].d
            );
            
            /* Call second test function */
            int res2 = compare_with_nans(
                test_pairs[i].a,
                test_pairs[i].b,
                op % 3
            );
            
            /* Update checksum deterministically */
            checksum += (res1 * 31 + res2) ^ (i * 17 + op);
            checksum &= 0x7FFFFFFF; /* Keep positive */
        }
    }
    
    /* Additional tests with sqrt(-1) to generate NaN */
    double sqrt_neg = __builtin_sqrt(-1.0);
    float sqrt_neg_f = __builtin_sqrtf(-1.0f);
    
    for (int i = 0; i < 4; i++) {
        int res = test_nan_comparisons(
            i + 4,
            sqrt_neg,
            normal_dbl,
            sqrt_neg_f,
            normal_flt
        );
        checksum += res * (i + 1);
    }
    
    /* Test with division by zero producing inf */
    double div_zero = 1.0 / zero_dbl;
    float div_zero_f = 1.0f / zero_flt;
    
    for (int i = 0; i < 4; i++) {
        int res = compare_with_nans(div_zero, inf_dbl, i % 3);
        checksum += res * (i + 5);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
