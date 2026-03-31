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
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins that might generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result = 1;
                sink += 1;
            }
            /* Alternative using comparison macros */
            if (!__builtin_isgreater(c, d) && !__builtin_isless(c, d)) {
                result |= 2;
                sink += 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins that might generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
                sink += 3;
            }
            /* Alternative formulation */
            if (__builtin_islessgreater(c, d)) {
                result |= 2;
                sink += 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Complex expression combining both */
            if ((__builtin_isnan(a) || __builtin_isnan(b)) ? 
                (__builtin_isunordered(a, b) || a == b) : 
                (!__builtin_isunordered(a, b) && a != b)) {
                result = 1;
                sink += 5;
            }
            break;
            
        case 3: /* Direct isnan checks */
            if (__builtin_isnan(c) || __builtin_isnan(d)) {
                if (!__builtin_isless(c, d) && !__builtin_isgreater(c, d)) {
                    result = 1;
                    sink += 6;
                }
            } else {
                if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) {
                    result |= 2;
                    sink += 7;
                }
            }
            break;
    }
    
    /* Use sink to prevent optimization */
    return result + (sink & 1);
}

/* Dummy function with side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
}

int main(void) {
    /* Create various NaN values using different methods */
    double nan_dbl_1 = 0.0 / 0.0;           /* Quiet NaN */
    double nan_dbl_2 = __builtin_nan("");   /* Another quiet NaN */
    double nan_dbl_3 = -__builtin_nanf(""); /* Negative quiet NaN */
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Float versions */
    float nan_flt_1 = 0.0f / 0.0f;
    float nan_flt_2 = __builtin_nanf("");
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -__builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN via sqrt(-1) */
    double nan_dbl_4 = __builtin_sqrt(-1.0);
    float nan_flt_3 = __builtin_sqrtf(-1.0f);
    
    /* Test values array */
    double dbl_vals[] = {
        nan_dbl_1, nan_dbl_2, nan_dbl_3, nan_dbl_4,
        inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl
    };
    
    float flt_vals[] = {
        nan_flt_1, nan_flt_2, nan_flt_3,
        inf_flt, neg_inf_flt, normal_flt, zero_flt
    };
    
    int dbl_count = sizeof(dbl_vals) / sizeof(dbl_vals[0]);
    int flt_count = sizeof(flt_vals) / sizeof(flt_vals[0]);
    
    int checksum = 0;
    
    /* Test many combinations to increase coverage */
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < dbl_count; i++) {
            for (int j = 0; j < dbl_count; j++) {
                for (int k = 0; k < flt_count; k++) {
                    for (int l = 0; l < flt_count; l++) {
                        /* Skip some combinations to keep runtime reasonable */
                        if ((i + j + k + l) % 3 != 0) continue;
                        
                        int res = test_nan_comparisons(
                            op + i + j, 
                            dbl_vals[i], 
                            dbl_vals[j],
                            flt_vals[k],
                            flt_vals[l]
                        );
                        
                        checksum = (checksum * 31 + res) & 0xFFFF;
                        
                        /* Call dummy function with side effect */
                        dummy_side_effect(res);
                    }
                }
            }
        }
    }
    
    /* Additional focused tests for edge cases */
    for (int i = 0; i < 100; i++) {
        /* Test UNEQ_EXPR patterns */
        checksum += test_nan_comparisons(0, nan_dbl_1, normal_dbl, nan_flt_1, normal_flt);
        checksum += test_nan_comparisons(0, normal_dbl, nan_dbl_2, normal_flt, nan_flt_2);
        checksum += test_nan_comparisons(0, nan_dbl_1, nan_dbl_2, nan_flt_1, nan_flt_2);
        checksum += test_nan_comparisons(0, normal_dbl, normal_dbl, normal_flt, normal_flt);
        
        /* Test LTGT_EXPR patterns */
        checksum += test_nan_comparisons(1, normal_dbl, normal_dbl + 1.0, normal_flt, normal_flt + 1.0f);
        checksum += test_nan_comparisons(1, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt);
        checksum += test_nan_comparisons(1, nan_dbl_1, normal_dbl, nan_flt_1, normal_flt);
        checksum += test_nan_comparisons(1, normal_dbl, nan_dbl_2, normal_flt, nan_flt_2);
        
        dummy_side_effect(i);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
