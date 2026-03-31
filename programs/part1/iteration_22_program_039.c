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
                result = 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: (a == a) && (b == b) && (a != b) */
            if (!__builtin_isunordered(a, b) && (a != b)) {
                result = 1;
            }
            /* Using comparison macros */
            if (isgreater(c, d) || isless(c, d)) {
                result |= 2;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isnan(a) && !__builtin_isnan(b)) {
                result = 3;
            }
            if (isunordered(c, d) && isless(c, d)) {
                result |= 4;
            }
            break;
            
        case 3: /* Direct use of builtins */
            if (__builtin_isgreater(a, b) && __builtin_islessequal(a, b + 1.0)) {
                result = 5;
            }
            if (__builtin_isunordered(c, d) || __builtin_isless(c, d)) {
                result |= 6;
            }
            break;
    }
    
    /* More side effects to prevent optimization */
    side_effect += result;
    return result + side_effect;
}

/* Another non-inlineable function with different comparison patterns */
static __attribute__((noinline))
int complex_nan_checks(double x, double y, float p, float q) {
    volatile int counter = 0;
    int checksum = 0;
    
    /* Generate UNEQ_EXPR patterns */
    checksum += (__builtin_isunordered(x, y) || (x == y)) ? 1 : 0;
    counter++;
    
    checksum += (isunordered(p, q) || (p == q)) ? 2 : 0;
    counter++;
    
    /* Generate LTGT_EXPR patterns */
    checksum += (!__builtin_isunordered(x, y) && (x != y)) ? 4 : 0;
    counter++;
    
    checksum += (isgreater(p, q) || isless(p, q)) ? 8 : 0;
    counter++;
    
    /* Complex expression that might fold differently */
    checksum += (__builtin_isnan(x) && !__builtin_isnan(y) && 
                 (isgreater(p, q) || islessequal(p, q))) ? 16 : 0;
    counter++;
    
    return checksum + counter;
}

int main(void) {
    /* Create various floating-point values */
    double nan_dbl = 0.0 / 0.0;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    double neg_zero_dbl = -0.0;
    
    /* Additional NaN sources */
    double sqrt_neg = __builtin_sqrt(-1.0);
    double explicit_nan = __builtin_nan("");
    double nan_payload = __builtin_nan("0xDEAD");
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    int checksum = 0;
    
    /* Test different combinations */
    for (int i = 0; i < 4; i++) {
        /* Test with NaN values */
        checksum += test_nan_comparisons(i, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(i, normal_dbl, nan_dbl, normal_flt, nan_flt);
        checksum += test_nan_comparisons(i, nan_dbl, nan_dbl, nan_flt, nan_flt);
        
        /* Test with infinity */
        checksum += test_nan_comparisons(i, inf_dbl, normal_dbl, inf_flt, normal_flt);
        checksum += test_nan_comparisons(i, neg_inf_dbl, inf_dbl, -inf_flt, inf_flt);
        
        /* Test with normal numbers */
        checksum += test_nan_comparisons(i, normal_dbl, zero_dbl, normal_flt, zero_flt);
        checksum += test_nan_comparisons(i, zero_dbl, neg_zero_dbl, zero_flt, -zero_flt);
        
        /* Test with different NaN sources */
        checksum += test_nan_comparisons(i, sqrt_neg, explicit_nan, nan_flt, normal_flt);
        checksum += test_nan_comparisons(i, nan_payload, inf_dbl, normal_flt, inf_flt);
    }
    
    /* Additional complex checks */
    checksum += complex_nan_checks(nan_dbl, normal_dbl, nan_flt, normal_flt);
    checksum += complex_nan_checks(explicit_nan, sqrt_neg, inf_flt, -inf_flt);
    checksum += complex_nan_checks(zero_dbl, neg_zero_dbl, zero_flt, normal_flt);
    checksum += complex_nan_checks(inf_dbl, neg_inf_dbl, nan_flt, nan_flt);
    
    /* Force evaluation of all comparisons */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
