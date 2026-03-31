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
            /* Using builtins that may generate UNEQ_EXPR */
            int cmp1 = __builtin_isunordered(a, b) || (a == b);
            int cmp2 = isunordered(c, d) || (c == d);
            int cmp3 = !__builtin_isnan(a) && !__builtin_isnan(b) && (a == b);
            
            result = cmp1 + cmp2 * 2 + cmp3 * 4;
            sink += result;
            break;
        }
        
        case 1: { /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins that may generate LTGT_EXPR */
            int cmp1 = !__builtin_isunordered(a, b) && (a != b);
            int cmp2 = !isunordered(c, d) && (c != d);
            int cmp3 = __builtin_isgreater(a, b) || __builtin_isless(a, b);
            
            result = cmp1 + cmp2 * 2 + cmp3 * 4;
            sink += result * 3;
            break;
        }
        
        case 2: { /* Mixed comparisons */
            /* Complex expression combining multiple comparisons */
            int cmp1 = (__builtin_isnan(a) || __builtin_isnan(b)) ? 
                       (a == b) : (a != b);
            int cmp2 = (isnan(c) || isnan(d)) ? 
                       __builtin_islessequal(c, d) : __builtin_isgreaterequal(c, d);
            
            result = cmp1 ^ cmp2;
            sink += result * 5;
            break;
        }
        
        case 3: { /* Direct NaN checks with comparisons */
            int cmp1 = (a == a) && (b == b) && (a != b);
            int cmp2 = (c == c) && (d == d) && (c == d);
            int cmp3 = __builtin_isnan(a) || __builtin_isnan(b);
            
            result = cmp1 + cmp2 - cmp3;
            sink += result * 7;
            break;
        }
    }
    
    /* Additional side effect to prevent optimization */
    static volatile int counter = 0;
    counter += sink;
    
    return result;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("O0"))) 
void dummy_side_effect(int x) {
    volatile static int dummy = 0;
    dummy += x;
}

int main(void) {
    /* Create various floating-point values */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan_dbl = -nan_dbl;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -inf_dbl;
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = sqrtf(-1.0f);
    float neg_nan_flt = -nan_flt;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN generation methods */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    /* Test values array */
    double dbl_vals[] = {nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
                         normal_dbl, zero_dbl, builtin_nan, 1.0/zero_dbl};
    float flt_vals[] = {nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
                       normal_flt, zero_flt, builtin_nanf, 1.0f/zero_flt};
    
    int dbl_count = sizeof(dbl_vals)/sizeof(dbl_vals[0]);
    int flt_count = sizeof(flt_vals)/sizeof(flt_vals[0]);
    
    int checksum = 0;
    
    /* Test many combinations to increase coverage */
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < dbl_count; i++) {
            for (int j = 0; j < dbl_count; j++) {
                for (int k = 0; k < flt_count; k++) {
                    for (int l = 0; l < flt_count; l++) {
                        /* Only test some combinations to keep runtime reasonable */
                        if ((i + j + k + l) % 3 == 0) {
                            int res = test_nan_comparisons(
                                op + i + j, 
                                dbl_vals[i], 
                                dbl_vals[j],
                                flt_vals[k],
                                flt_vals[l]
                            );
                            checksum += res;
                            
                            /* Call dummy function for additional side effects */
                            dummy_side_effect(res);
                        }
                    }
                }
            }
        }
    }
    
    /* Also test specific edge cases */
    checksum += test_nan_comparisons(0, nan_dbl, nan_dbl, nan_flt, nan_flt);
    checksum += test_nan_comparisons(1, normal_dbl, nan_dbl, normal_flt, nan_flt);
    checksum += test_nan_comparisons(2, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt);
    checksum += test_nan_comparisons(3, zero_dbl, -zero_dbl, zero_flt, -zero_flt);
    
    /* Complex expression that might generate UNEQ_EXPR/LTGT_EXPR */
    volatile double v1 = nan_dbl;
    volatile double v2 = normal_dbl;
    volatile float v3 = nan_flt;
    volatile float v4 = normal_flt;
    
    for (int i = 0; i < 10; i++) {
        int r1 = __builtin_isunordered(v1, v2) || (v1 == v2);
        int r2 = !__builtin_isunordered(v3, v4) && (v3 != v4);
        int r3 = (v1 != v1) ? (v2 == v2) : (v1 > v2);
        int r4 = __builtin_isnan(v3) ? __builtin_isless(v4, v3) : __builtin_isgreater(v4, v3);
        
        checksum += r1 + r2 + r3 + r4;
        dummy_side_effect(r1 + r2 + r3 + r4);
        
        /* Modify values slightly */
        v1 += 0.1;
        v2 += 0.2;
        v3 += 0.1f;
        v4 += 0.2f;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
