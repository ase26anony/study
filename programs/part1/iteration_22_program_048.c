/* test_nan_folding.c - Target specific uncovered lines in fold-const.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int global_counter = 0;
static void __attribute__((noinline)) dummy_side_effect(int x) {
    global_counter += x;
}

/* Non-inlineable helper with complex NaN-aware comparisons */
static int __attribute__((noinline,noipa))
test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int result = 0;
    volatile double vd1, vd2;
    volatile float vf1, vf2;
    
    /* Complex switch-like structure to generate different comparison tree codes */
    switch (opcode & 3) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using __builtin_isunordered and equality check */
            if (__builtin_isunordered(a, b) || (a == b)) {
                dummy_side_effect(1);
                vd1 = a + b;
            }
            /* Using isunordered macro */
            if (isunordered(c, d) || (c == d)) {
                dummy_side_effect(2);
                vf1 = c * d;
            }
            /* Direct NaN checks with __builtin_isnan */
            if (__builtin_isnan(a) || __builtin_isnan(b) || a == b) {
                result |= 1;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal (less than or greater than) */
            /* Using __builtin_isgreater/isless without equality */
            if ((__builtin_isgreater(a, b) || __builtin_isless(a, b)) && 
                !__builtin_isunordered(a, b)) {
                dummy_side_effect(3);
                vd2 = a - b;
            }
            /* Using comparison macros */
            if ((isgreater(c, d) || isless(c, d)) && !isunordered(c, d)) {
                dummy_side_effect(4);
                vf2 = c / (d + 1.0f);
            }
            /* Alternative formulation */
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 2;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ and LTGT in same basic block */
            int uneq_test = (__builtin_isunordered(a, b) || a == b) ? 1 : 0;
            int ltgt_test = (!__builtin_isunordered(a, b) && a != b) ? 1 : 0;
            
            dummy_side_effect(uneq_test + ltgt_test * 2);
            
            /* Force use of results */
            vd1 = uneq_test ? a : b;
            vd2 = ltgt_test ? b : a;
            result = uneq_test | (ltgt_test << 1);
            break;
            
        case 3: /* Complex expression with side effects */
            /* Nested comparisons that might fold */
            double temp = (__builtin_isnan(a) || a == b) ? 
                         (__builtin_isgreater(b, 0.0) ? a : b) : 
                         (__builtin_isless(a, b) ? a + b : a - b);
            
            float ftemp = (isunordered(c, d) || c == d) ?
                         (islessequal(c, 0.0f) ? c : d) :
                         (isgreaterequal(c, d) ? c * d : c / d);
            
            dummy_side_effect((int)temp + (int)ftemp);
            vd1 = temp;
            vf1 = ftemp;
            result = 3;
            break;
    }
    
    /* Additional volatile stores to prevent dead code elimination */
    *(volatile double*)&vd1 = vd1;
    *(volatile float*)&vf1 = vf1;
    
    return result;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;                    /* Quiet NaN */
    double neg_nan_dbl = -__builtin_nan("");       /* Negative quiet NaN */
    double inf_dbl = __builtin_inf();              /* Positive infinity */
    double neg_inf_dbl = -__builtin_inf();         /* Negative infinity */
    double normal_dbl = 3.141592653589793;
    double zero_dbl = 0.0;
    
    float nan_flt = 0.0f / 0.0f;                   /* Float NaN */
    float neg_nan_flt = -__builtin_nanf("");       /* Negative float NaN */
    float inf_flt = __builtin_inff();              /* Float infinity */
    float neg_inf_flt = -__builtin_inff();         /* Negative float infinity */
    float normal_flt = 2.718281828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN generation methods */
    double sqrt_nan = __builtin_sqrt(-1.0);        /* NaN from sqrt(-1) */
    double asin_nan = __builtin_asin(2.0);         /* NaN from asin(2) */
    float log_nan = __builtin_logf(-1.0f);         /* NaN from log(-1) */
    
    int checksum = 0;
    
    /* Test different combinations to trigger various comparison patterns */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan, asin_nan
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, log_nan
    };
    
    int num_dbl_tests = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt_tests = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations and opcodes */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_dbl_tests; i++) {
            for (int j = 0; j < num_dbl_tests; j++) {
                for (int k = 0; k < num_flt_tests; k++) {
                    for (int l = 0; l < num_flt_tests; l++) {
                        /* Limit iterations for practical runtime */
                        if ((i + j + k + l) % 3 == 0) {
                            int res = test_nan_comparisons(
                                op, 
                                test_values_dbl[i], 
                                test_values_dbl[j],
                                test_values_flt[k],
                                test_values_flt[l]
                            );
                            checksum = (checksum * 31 + res) & 0xFFFF;
                        }
                    }
                }
            }
        }
    }
    
    /* Also test specific edge cases */
    checksum += test_nan_comparisons(0, nan_dbl, nan_dbl, nan_flt, nan_flt);
    checksum += test_nan_comparisons(1, nan_dbl, normal_dbl, nan_flt, normal_flt);
    checksum += test_nan_comparisons(2, normal_dbl, normal_dbl, normal_flt, normal_flt);
    checksum += test_nan_comparisons(3, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt);
    
    /* Test with same value comparisons */
    checksum += test_nan_comparisons(0, zero_dbl, zero_dbl, zero_flt, zero_flt);
    checksum += test_nan_comparisons(1, inf_dbl, inf_dbl, inf_flt, inf_flt);
    
    printf("Final checksum: %d\n", checksum);
    printf("Global side effect counter: %d\n", global_counter);
    
    return 0;
}
