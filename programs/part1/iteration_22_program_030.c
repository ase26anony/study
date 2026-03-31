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
    side_effect += (int)((uintptr_t)&side_effect) % 256;
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins that may generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Alternative using standard macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            /* Direct NaN check combination */
            if (__builtin_isnan(a) || __builtin_isnan(b) || a == b) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using GCC builtins that may generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 1;
            }
            /* Alternative using standard macros */
            if (!isunordered(c, d) && c != d) {
                result |= 2;
            }
            /* Direct ordered comparison */
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 4;
            }
            /* Mixed comparisons to increase coverage */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 8;
            }
            break;
            
        case 2: /* Mixed comparisons for robustness */
            if (__builtin_islessgreater(a, b)) {
                result |= 1;
            }
            if (islessgreater(c, d)) {
                result |= 2;
            }
            /* Complex expression with volatile */
            volatile double tmp = a + b;
            if ((!__builtin_isunordered(a, b) && a != b) || 
                (__builtin_isunordered(c, d) || c == d)) {
                result |= 4;
                side_effect += (int)tmp;
            }
            break;
            
        default:
            /* Fallback comparisons */
            result = (a != b) ? 1 : 0;
            break;
    }
    
    /* Additional side effect to prevent optimization */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result + side_effect;
}

/* Non-inlineable helper with switch-like structure */
static __attribute__((noinline, optimize("no-tree-vectorize")))
int perform_comparisons(double x, double y, float fx, float fy, int mode) {
    int checksum = 0;
    
    /* Multiple calls with different opcodes */
    checksum ^= test_nan_comparisons(0, x, y, fx, fy);
    checksum ^= test_nan_comparisons(1, y, x, fy, fx);
    checksum ^= test_nan_comparisons(2, x, x, fx, fx);
    checksum ^= test_nan_comparisons(mode, y, y, fy, fy);
    
    /* Cross-type comparisons */
    checksum ^= test_nan_comparisons(0, x, (double)fx, fy, (float)y);
    checksum ^= test_nan_comparisons(1, (double)fy, y, (float)x, fx);
    
    return checksum;
}

int main(void) {
    /* Create various NaN values using different methods */
    double nan_dbl_1 = 0.0 / 0.0;
    double nan_dbl_2 = __builtin_nan("");
    double nan_dbl_3 = -0.0 / 0.0;
    double nan_dbl_4 = sqrt(-1.0);
    
    float nan_flt_1 = 0.0f / 0.0f;
    float nan_flt_2 = __builtin_nanf("");
    float nan_flt_3 = -0.0f / 0.0f;
    
    /* Normal numbers and infinities */
    double normal_dbl = 3.14159;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    
    float normal_flt = 2.71828f;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -__builtin_inff();
    
    int checksum = 0;
    
    /* Test various combinations in a loop */
    for (int i = 0; i < 10; i++) {
        /* NaN vs NaN */
        checksum += perform_comparisons(
            nan_dbl_1, nan_dbl_2, nan_flt_1, nan_flt_2, i % 3);
        
        /* NaN vs normal */
        checksum += perform_comparisons(
            nan_dbl_3, normal_dbl, nan_flt_3, normal_flt, i % 3);
        
        /* NaN vs infinity */
        checksum += perform_comparisons(
            nan_dbl_4, inf_dbl, nan_flt_2, inf_flt, i % 3);
        
        /* Normal vs infinity */
        checksum += perform_comparisons(
            normal_dbl, inf_dbl, normal_flt, inf_flt, i % 3);
        
        /* Infinity vs negative infinity */
        checksum += perform_comparisons(
            inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt, i % 3);
        
        /* Same value comparisons */
        checksum += perform_comparisons(
            normal_dbl, normal_dbl, normal_flt, normal_flt, i % 3);
        
        /* Zero vs negative zero */
        checksum += perform_comparisons(
            0.0, -0.0, 0.0f, -0.0f, i % 3);
    }
    
    /* Use checksum to prevent dead code elimination */
    volatile int final_result = checksum;
    printf("Result checksum: %d\n", final_result % 1000000);
    
    return (final_result != 0) ? 0 : 1;
}
