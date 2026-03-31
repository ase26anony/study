/* test_nan_comparisons.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static void __attribute__((noinline)) dummy_side_effect(int x) {
    static volatile int sink;
    sink = x;
}

/* Non-inlineable helper function with complex comparison logic */
static int __attribute__((noinline,noipa))
test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int result = 0;
    
    /* Use different comparison types based on opcode */
    switch (opcode & 3) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                dummy_side_effect(1);
                result |= 1;
            }
            /* Also test with float types */
            if (isunordered(c, d) || c == d) {
                dummy_side_effect(2);
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            if (!__builtin_isunordered(a, b) && a != b) {
                dummy_side_effect(3);
                result |= 4;
            }
            if (!isunordered(c, d) && c != d) {
                dummy_side_effect(4);
                result |= 8;
            }
            break;
            
        case 2: /* Mixed comparisons using builtins */
            if (__builtin_isgreater(a, b) && __builtin_isless(c, d)) {
                dummy_side_effect(5);
                result |= 16;
            }
            /* Create UNEQ_EXPR through complex expression */
            if ((__builtin_isnan(a) || __builtin_isnan(b)) ? 
                (__builtin_isunordered(a, b) || a == b) : (a == b)) {
                dummy_side_effect(6);
                result |= 32;
            }
            break;
            
        case 3: /* More LTGT_EXPR variations */
            if ((!__builtin_isnan(a) && !__builtin_isnan(b)) ? 
                (!__builtin_isunordered(a, b) && a != b) : 0) {
                dummy_side_effect(7);
                result |= 64;
            }
            /* Direct use of comparison macros */
            if (isgreater(a, b) || islessequal(c, d)) {
                dummy_side_effect(8);
                result |= 128;
            }
            break;
    }
    
    /* Additional side effects to prevent optimization */
    volatile double temp = a + b;
    volatile float tempf = c * d;
    
    return result;
}

int main(void) {
    /* Create various floating-point values including NaNs */
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
    double sqrt_nan = __builtin_sqrt(-1.0);
    double log_nan = __builtin_log(-1.0);
    float asin_nan = __builtin_asinf(2.0f);
    
    int checksum = 0;
    int iteration = 0;
    
    /* Test different combinations of values and opcodes */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan, log_nan
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, asin_nan, 1.0f
    };
    
    /* Loop through many combinations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int op = 0; op < 4; op++) {
                int result = test_nan_comparisons(
                    op + iteration,
                    test_values_dbl[i],
                    test_values_dbl[j],
                    test_values_flt[i],
                    test_values_flt[j]
                );
                
                checksum = (checksum * 31 + result) & 0xFFFF;
                iteration = (iteration + 1) & 7;
                
                /* Test with explicit NaN builtins */
                if (i == j) {
                    double explicit_nan = __builtin_nan("");
                    float explicit_nanf = __builtin_nanf("");
                    
                    result = test_nan_comparisons(
                        op ^ iteration,
                        explicit_nan,
                        test_values_dbl[j],
                        explicit_nanf,
                        test_values_flt[j]
                    );
                    
                    checksum = (checksum * 17 + result) & 0xFFFF;
                }
            }
        }
    }
    
    /* Final computation to use all results */
    printf("Checksum: %d\n", checksum);
    
    /* Ensure all code paths are potentially reachable */
    volatile int final_check = 0;
    if (checksum > 1000) {
        dummy_side_effect(checksum);
    }
    
    return checksum != 0 ? 0 : 1;
}
