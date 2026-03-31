/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    switch (opcode % 4) {
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins for unordered comparisons */
            int cmp1 = __builtin_isunordered(a, b) || (a == b);
            int cmp2 = isunordered(c, d) || (c == d);
            result = cmp1 && cmp2;
            sink += result;
            break;
        }
        case 1: { /* LTGT_EXPR-like: ordered and not equal */
            /* Ordered comparison: !(unordered) && (a != b) */
            int cmp1 = !__builtin_isunordered(a, b) && (a != b);
            int cmp2 = !isunordered(c, d) && (c != d);
            result = cmp1 || cmp2;
            sink += result;
            break;
        }
        case 2: { /* Mixed comparisons using builtin macros */
            /* Using math.h comparison macros */
            int cmp1 = !__builtin_isnan(a) && !__builtin_isnan(b) && 
                      (__builtin_isgreater(a, b) || __builtin_isless(a, b));
            int cmp2 = !isnan(c) && !isnan(d) && 
                      (isgreater(c, d) || isless(c, d));
            result = cmp1 ^ cmp2;
            sink += result;
            break;
        }
        case 3: { /* Direct NaN checks with comparisons */
            int cmp1 = (__builtin_isnan(a) || __builtin_isnan(b)) ? 
                      (a != a || b != b) : (a == b);
            int cmp2 = (isnan(c) || isnan(d)) ? 
                      (c != c || d != d) : (c == d);
            result = cmp1 & cmp2;
            sink += result;
            break;
        }
    }
    
    /* Additional side effect to prevent optimization */
    static volatile int counter = 0;
    counter += sink;
    
    return result;
}

/* Dummy function with side effects */
static __attribute__((noinline)) 
void dummy_side_effect(double x, float y) {
    volatile double tmp = x;
    volatile float tmp2 = y;
    (void)tmp;
    (void)tmp2;
}

int main(void) {
    /* Create various floating-point values including NaN */
    double nan_dbl = 0.0 / 0.0;          /* Quiet NaN */
    double neg_nan_dbl = -nan_dbl;       /* Negative NaN */
    double inf_dbl = __builtin_inf();    /* Positive infinity */
    double neg_inf_dbl = -inf_dbl;       /* Negative infinity */
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Create NaN using sqrt(-1) */
    double nan_sqrt = __builtin_sqrt(-1.0);
    
    /* Create NaN using builtin */
    double nan_builtin = __builtin_nan("");
    
    float nan_flt = 0.0f / 0.0f;         /* Float NaN */
    float inf_flt = __builtin_inff();    /* Float infinity */
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Array of test values */
    double dbl_vals[] = {nan_dbl, neg_nan_dbl, nan_sqrt, nan_builtin, 
                        inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl};
    float flt_vals[] = {nan_flt, -nan_flt, inf_flt, -inf_flt, 
                       normal_flt, zero_flt};
    
    int dbl_count = sizeof(dbl_vals) / sizeof(dbl_vals[0]);
    int flt_count = sizeof(flt_vals) / sizeof(flt_vals[0]);
    
    int checksum = 0;
    
    /* Test various combinations */
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < dbl_count; i++) {
            for (int j = 0; j < dbl_count; j++) {
                for (int k = 0; k < flt_count; k++) {
                    for (int l = 0; l < flt_count; l++) {
                        /* Call comparison function */
                        int res = test_nan_comparisons(
                            op, 
                            dbl_vals[i], 
                            dbl_vals[j],
                            flt_vals[k],
                            flt_vals[l]
                        );
                        
                        /* Update checksum deterministically */
                        checksum = (checksum * 31 + res) & 0xFFFF;
                        
                        /* Call dummy function to add side effects */
                        dummy_side_effect(dbl_vals[i], flt_vals[k]);
                    }
                }
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 100; i++) {
        /* Direct UNEQ_EXPR pattern: unordered or equal */
        double a = (i % 3 == 0) ? nan_dbl : normal_dbl;
        double b = (i % 5 == 0) ? nan_sqrt : normal_dbl + i;
        float c = (i % 2 == 0) ? nan_flt : normal_flt;
        float d = (i % 7 == 0) ? -nan_flt : normal_flt + i;
        
        /* Pattern that should generate UNEQ_EXPR */
        int uneq_like = __builtin_isunordered(a, b) || (a == b);
        checksum += uneq_like;
        
        /* Pattern that should generate LTGT_EXPR */
        int ltgt_like = !__builtin_isunordered(a, b) && (a != b);
        checksum += ltgt_like;
        
        /* Using math.h macros */
        int uneq_macro = isunordered(c, d) || (c == d);
        checksum += uneq_macro;
        
        int ltgt_macro = !isunordered(c, d) && (c != d);
        checksum += ltgt_macro;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
