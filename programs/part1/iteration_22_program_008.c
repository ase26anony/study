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
    switch (opcode) {
        case 0: /* UNEQ_EXPR: unordered or equal */
            /* Using __builtin_isunordered and equality check */
            if (__builtin_isunordered(a, b) || a == b) {
                side_effect = 1;
                result |= 1;
            }
            /* Additional float comparison to generate more tree nodes */
            if (__builtin_isunordered(c, d) || c == d) {
                side_effect += 2;
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR: ordered and not equal (less than or greater than) */
            /* Using ordered comparison macros */
            if (!__builtin_isunordered(a, b) && a != b) {
                side_effect = 3;
                result |= 4;
            }
            /* Alternative using comparison macros */
            if (islessgreater(c, d)) {
                side_effect += 4;
                result |= 8;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isnan(a) && __builtin_isgreater(b, 0.0)) {
                side_effect = 5;
                result |= 16;
            }
            if (isless(c, d) && !__builtin_isnan(c)) {
                side_effect += 6;
                result |= 32;
            }
            break;
            
        default:
            /* Generate UNEQ and LTGT with different operand orders */
            if (__builtin_isunordered(b, a) || b == a) {
                side_effect = 7;
                result |= 64;
            }
            if (islessgreater(d, c)) {
                side_effect += 8;
                result |= 128;
            }
            break;
    }
    
    /* Use side_effect to prevent dead code elimination */
    asm volatile("" : "+r" (side_effect));
    return result + side_effect;
}

/* Non-inlineable dummy function for additional side effects */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += 1;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;          /* Generate quiet NaN */
    double neg_nan_dbl = -nan_dbl;       /* Negative NaN */
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -inf_dbl;
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Float versions */
    float nan_flt = 0.0f / 0.0f;
    float neg_nan_flt = -nan_flt;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN sources */
    double sqrt_nan_dbl = __builtin_sqrt(-1.0);
    float sqrt_nan_flt = __builtin_sqrtf(-1.0f);
    
    int checksum = 0;
    volatile int global_counter = 0;
    
    /* Test different combinations to generate UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            double a, b;
            float c, d;
            
            /* Select different operand combinations */
            switch (j) {
                case 0: a = nan_dbl; b = normal_dbl; c = nan_flt; d = normal_flt; break;
                case 1: a = normal_dbl; b = nan_dbl; c = normal_flt; d = nan_flt; break;
                case 2: a = nan_dbl; b = nan_dbl; c = nan_flt; d = nan_flt; break;
                case 3: a = inf_dbl; b = normal_dbl; c = inf_flt; d = normal_flt; break;
                case 4: a = normal_dbl; b = inf_dbl; c = normal_flt; d = inf_flt; break;
                case 5: a = sqrt_nan_dbl; b = neg_nan_dbl; c = sqrt_nan_flt; d = neg_nan_flt; break;
                default: a = zero_dbl; b = inf_dbl; c = zero_flt; d = inf_flt; break;
            }
            
            /* Call with different opcodes */
            int res = test_nan_comparisons(i, a, b, c, d);
            checksum ^= res * (i + 1) * (j + 1);
            
            /* Add side effect to prevent optimization */
            dummy_call(&global_counter);
        }
    }
    
    /* Additional specific tests for UNEQ and LTGT */
    int specific_results = 0;
    
    /* Direct UNEQ-like expressions */
    if (__builtin_isunordered(nan_dbl, normal_dbl) || nan_dbl == normal_dbl) {
        specific_results |= 1;
    }
    
    /* Direct LTGT-like expressions */
    if (!__builtin_isunordered(normal_dbl, inf_dbl) && normal_dbl != inf_dbl) {
        specific_results |= 2;
    }
    
    /* Using islessgreater macro */
    if (islessgreater(normal_flt, zero_flt)) {
        specific_results |= 4;
    }
    
    /* Complex expression mixing NaN checks */
    if ((__builtin_isnan(sqrt_nan_dbl) && isgreater(inf_dbl, zero_dbl)) ||
        (isless(neg_inf_flt, normal_flt) && !__builtin_isnan(normal_flt))) {
        specific_results |= 8;
    }
    
    checksum ^= specific_results;
    
    /* Use global_counter to prevent dead code elimination */
    asm volatile("" : "+r" (global_counter));
    
    printf("Checksum: %d (Counter: %d)\n", checksum, global_counter);
    return checksum != 0 ? 0 : 1;
}
