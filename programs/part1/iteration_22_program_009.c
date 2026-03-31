/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex side-effectful expressions with NaN-aware comparisons */
    switch (opcode) {
        case 0: /* UNEQ_EXPR: unordered or equal */
            sink += __builtin_isunordered(a, b) || (a == b);
            result = sink ? 1 : 0;
            break;
            
        case 1: /* LTGT_EXPR: less than or greater than (ordered and not equal) */
            sink += __builtin_isless(c, d) || __builtin_isgreater(c, d);
            result = sink ? 2 : 0;
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            sink += __builtin_isnan(a) ? 3 : 0;
            sink += isgreater(b, 1.0) ? 4 : 0;
            sink += islessequal(c, d) ? 5 : 0;
            result = sink;
            break;
            
        case 3: /* Direct unordered comparisons */
            sink += __builtin_isunordered(a, c) ? 6 : 0;
            sink += !__builtin_isunordered(b, d) ? 7 : 0;
            result = sink;
            break;
            
        default:
            result = -1;
    }
    
    /* Additional side effects to prevent optimization */
    static volatile int counter = 0;
    counter += result;
    
    return result;
}

/* Dummy function with side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int val) {
    static volatile int storage = 0;
    storage += val;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan_dbl = -nan_dbl;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -inf_dbl;
    double normal_dbl = 3.14159;
    
    float nan_flt = sqrtf(-1.0f);
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    float normal_flt = 2.71828f;
    
    /* Additional NaN sources */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    int checksum = 0;
    
    /* Test different combinations to generate UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            double a, b;
            float c, d;
            
            /* Vary the operand combinations */
            switch (j % 5) {
                case 0: a = nan_dbl; b = normal_dbl; c = nan_flt; d = normal_flt; break;
                case 1: a = normal_dbl; b = builtin_nan; c = normal_flt; d = builtin_nanf; break;
                case 2: a = inf_dbl; b = neg_inf_dbl; c = inf_flt; d = neg_inf_flt; break;
                case 3: a = nan_dbl; b = nan_dbl; c = nan_flt; d = nan_flt; break;
                case 4: a = normal_dbl; b = normal_dbl * 2.0; c = normal_flt; d = normal_flt * 2.0f; break;
            }
            
            int res = test_nan_comparisons(i, a, b, c, d);
            checksum += res;
            
            /* Add side effects */
            dummy_side_effect(res);
        }
    }
    
    /* Also test with compile-time constants */
    checksum += test_nan_comparisons(0, 1.0/0.0, 1.0/0.0, 0.0f/0.0f, 0.0f/0.0f);
    checksum += test_nan_comparisons(1, 1.0, 2.0, 3.0f, 4.0f);
    
    /* Test with signaling NaN if supported */
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    double snan = __builtin_nans("");
    float snanf = __builtin_nansf("");
    checksum += test_nan_comparisons(2, snan, normal_dbl, snanf, normal_flt);
    #endif
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to affect return value (prevent dead code elimination) */
    return checksum == 0 ? 0 : 1;
}
