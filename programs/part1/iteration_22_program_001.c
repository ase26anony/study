/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    side_effect += (int)(a * 100.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like behavior */
            /* unordered or equal: (a != a) || (b != b) || (a == b) */
            if (__builtin_isunordered(a, b) || (a == b)) {
                result |= 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(b) || (a == b)) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* less than or greater than (ordered and not equal) */
            if (!__builtin_isunordered(a, b) && (a != b)) {
                result |= 1;
            }
            if (!isunordered(c, d) && (c != d)) {
                result |= 2;
            }
            /* Using comparison macros */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ and LTGT patterns */
            if ((__builtin_isunordered(a, b) || (a == b)) && 
                !__builtin_isnan(c)) {
                result |= 1;
            }
            if (!isunordered(c, d) && (c != d) && 
                __builtin_isnan(a)) {
                result |= 2;
            }
            break;
            
        default:
            /* Fallback comparisons */
            result = (a == b) ? 8 : 16;
            break;
    }
    
    /* More side effects to prevent optimization */
    side_effect += result;
    return result + side_effect;
}

/* Dummy function to prevent optimization */
static __attribute__((noinline)) 
void use_result(int val) {
    volatile static int sink;
    sink = val;
}

int main(void) {
    /* Create various floating-point values */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan_dbl = -nan_dbl;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -inf_dbl;
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = 0.0f / 0.0f;
    float neg_nan_flt = -nan_flt;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN sources */
    double sqrt_nan = sqrt(-1.0);
    double log_nan = log(-1.0);
    float acos_nan = acos(2.0f);
    
    int checksum = 0;
    int iteration = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan, log_nan
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, acos_nan
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations */
    for (int op = 0; op < 3; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_flt; j++) {
                for (int k = 0; k < num_dbl; k++) {
                    for (int l = 0; l < num_flt; l++) {
                        if (iteration++ % 7 == 0) { /* Sample to keep runtime reasonable */
                            int res = test_nan_comparisons(
                                op,
                                test_values_dbl[i],
                                test_values_dbl[k],
                                test_values_flt[j],
                                test_values_flt[l]
                            );
                            checksum ^= res + iteration;
                            use_result(res);
                        }
                    }
                }
            }
        }
    }
    
    /* Additional targeted tests */
    int targeted_results = 0;
    
    /* Direct UNEQ pattern with NaN */
    if (__builtin_isunordered(nan_dbl, normal_dbl) || (nan_dbl == normal_dbl)) {
        targeted_results |= 1;
    }
    
    /* Direct LTGT pattern without NaN */
    if (!__builtin_isunordered(normal_dbl, inf_dbl) && (normal_dbl != inf_dbl)) {
        targeted_results |= 2;
    }
    
    /* Mixed: UNEQ with one NaN */
    if ((isunordered(nan_flt, normal_flt) || (nan_flt == normal_flt)) &&
        !__builtin_isnan(normal_dbl)) {
        targeted_results |= 4;
    }
    
    /* Mixed: LTGT with potential NaN */
    if (!isunordered(zero_flt, inf_flt) && (zero_flt != inf_flt) &&
        __builtin_isnan(sqrt_nan)) {
        targeted_results |= 8;
    }
    
    checksum ^= targeted_results;
    
    printf("Checksum: %d\n", checksum);
    printf("Iterations: %d\n", iteration);
    
    return checksum == 0 ? 0 : 1;
}
