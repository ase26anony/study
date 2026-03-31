/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects to prevent early optimization */
    sink += (int)(a * 0.5) + (int)(c * 0.3);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins to generate UNEQ_EXPR tree code */
            if (__builtin_isunordered(a, b) || __builtin_islessgreater(a, b) == 0) {
                result |= 1;
            }
            /* Alternative using standard macros */
            if (isunordered(c, d) || !islessgreater(c, d)) {
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using builtins to generate LTGT_EXPR tree code */
            if (__builtin_islessgreater(a, b) && !__builtin_isunordered(a, b)) {
                result |= 4;
            }
            /* Alternative using standard macros */
            if (islessgreater(c, d) && !isunordered(c, d)) {
                result |= 8;
            }
            break;
            
        case 2: /* Mixed comparisons to increase coverage */
            /* Direct NaN checks combined with comparisons */
            if (__builtin_isnan(a) || __builtin_isnan(b)) {
                if (isunordered(c, d)) {
                    result |= 16;
                }
            } else if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                if (islessgreater(c, d)) {
                    result |= 32;
                }
            }
            break;
    }
    
    /* More side effects */
    sink += result;
    return result + sink;
}

/* Dummy function to prevent optimization */
static __attribute__((noinline)) 
void use_result(int val) {
    volatile static int storage = 0;
    storage += val;
}

int main(void) {
    /* Initialize test values including NaNs */
    double nan_dbl = 0.0 / 0.0;           /* Generate NaN */
    double inf_dbl = __builtin_inf();     /* Infinity */
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Alternative NaN generation methods */
    double nan_dbl2 = __builtin_nan("");
    double nan_dbl3 = sqrt(-1.0);
    
    /* Float versions */
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    int checksum = 0;
    
    /* Test different combinations in a loop */
    for (int i = 0; i < 3; i++) {  /* opcodes 0, 1, 2 */
        /* Test with NaN vs normal */
        checksum += test_nan_comparisons(i, nan_dbl, normal_dbl, nan_flt, normal_flt);
        
        /* Test with NaN vs NaN */
        checksum += test_nan_comparisons(i, nan_dbl, nan_dbl2, nan_flt, nan_flt);
        
        /* Test with normal vs normal */
        checksum += test_nan_comparisons(i, normal_dbl, zero_dbl, normal_flt, zero_flt);
        
        /* Test with infinity vs NaN */
        checksum += test_nan_comparisons(i, inf_dbl, nan_dbl3, inf_flt, nan_flt);
        
        /* Test with negative infinity vs normal */
        checksum += test_nan_comparisons(i, neg_inf_dbl, normal_dbl, -inf_flt, normal_flt);
        
        /* Test with zero vs NaN from sqrt(-1) */
        checksum += test_nan_comparisons(i, zero_dbl, nan_dbl3, zero_flt, nan_flt);
    }
    
    /* Use results to prevent dead code elimination */
    use_result(checksum);
    
    /* Print deterministic checksum */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
