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
            /* Using GCC builtins to generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || __builtin_isgreater(a, b) || 
                __builtin_isless(a, b) || a == b) {
                result = 1;
            }
            /* Additional UNEQ_EXPR variants */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using GCC builtins to generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
            }
            /* Alternative formulation */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 2;
            }
            /* Float version */
            if (islessgreater(c, d)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isnan(a) && __builtin_isnan(b)) {
                result = 1;
            }
            if (!__builtin_isnan(c) && !__builtin_isnan(d)) {
                if (c > d || c < d) {
                    result |= 2;
                }
            }
            break;
            
        default:
            /* Generate other comparison codes */
            if (a == b) result = 1;
            if (a != b) result |= 2;
            if (a > b) result |= 4;
            if (a < b) result |= 8;
            break;
    }
    
    /* More side effects to prevent optimization */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result;
}

/* Dummy non-inlineable function for additional side effects */
static __attribute__((noinline, used))
void dummy_side_effect(int *p) {
    *p += 1;
}

int main(void) {
    /* Initialize test values including NaNs */
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
    float acos_nan = acosf(2.0f);
    
    int checksum = 0;
    volatile int counter = 0;
    
    /* Test various combinations */
    double test_pairs_dbl[][2] = {
        {nan_dbl, normal_dbl},
        {normal_dbl, nan_dbl},
        {nan_dbl, nan_dbl},
        {inf_dbl, nan_dbl},
        {normal_dbl, inf_dbl},
        {zero_dbl, zero_dbl},
        {neg_inf_dbl, inf_dbl},
        {sqrt_nan, log_nan},
        {normal_dbl, zero_dbl}
    };
    
    float test_pairs_flt[][2] = {
        {nan_flt, normal_flt},
        {normal_flt, nan_flt},
        {nan_flt, nan_flt},
        {inf_flt, nan_flt},
        {normal_flt, inf_flt},
        {zero_flt, zero_flt},
        {neg_inf_flt, inf_flt},
        {acos_nan, nan_flt},
        {normal_flt, zero_flt}
    };
    
    /* Loop through test cases */
    for (int op = 0; op < 4; op++) {
        for (size_t i = 0; i < sizeof(test_pairs_dbl)/sizeof(test_pairs_dbl[0]); i++) {
            for (size_t j = 0; j < sizeof(test_pairs_flt)/sizeof(test_pairs_flt[0]); j++) {
                int res = test_nan_comparisons(
                    op,
                    test_pairs_dbl[i][0],
                    test_pairs_dbl[i][1],
                    test_pairs_flt[j][0],
                    test_pairs_flt[j][1]
                );
                
                checksum ^= (res << (op * 4)) ^ (i * 17) ^ (j * 23);
                counter++;
                
                /* Call dummy function for side effects */
                dummy_side_effect(&checksum);
            }
        }
    }
    
    /* Force use of all variables */
    asm volatile("" : : "r"(nan_dbl), "r"(neg_nan_dbl), "r"(inf_dbl), 
                   "r"(neg_inf_dbl), "r"(normal_dbl), "r"(zero_dbl),
                   "r"(nan_flt), "r"(neg_nan_flt), "r"(inf_flt),
                   "r"(neg_inf_flt), "r"(normal_flt), "r"(zero_flt),
                   "r"(sqrt_nan), "r"(log_nan), "r"(acos_nan) : "memory");
    
    printf("Checksum: %d\n", checksum);
    printf("Iterations: %d\n", counter);
    
    return 0;
}
