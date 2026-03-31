/* test_nan_comparisons.c - Targeting fold-const.cc lines 6248-6258 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects to prevent early optimization */
    side_effect += (int)((uintptr_t)&side_effect) & 1;
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR (unordered or equal) */
            /* Using GCC builtins to generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
                result |= 1;
            }
            /* Alternative using standard macros */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            /* Direct NaN check */
            if (__builtin_isnan(a) || __builtin_isnan(c) || a == b || c == d) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR (ordered and not equal) */
            /* Using GCC builtins to generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && __builtin_islessgreater(a, b)) {
                result |= 1;
            }
            /* Alternative formulation */
            if (islessgreater(c, d) && !isunordered(c, d)) {
                result |= 2;
            }
            /* Direct comparison avoiding unordered */
            if (a < b || a > b) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isgreater(a, b) && __builtin_isless(c, d)) {
                result |= 1;
            }
            if (islessequal(a, b) || isgreaterequal(c, d)) {
                result |= 2;
            }
            if (__builtin_isnan(a) && !__builtin_isnan(b)) {
                result |= 4;
            }
            break;
            
        default:
            /* Generate other comparison tree codes */
            if (a == b) result |= 1;
            if (c != d) result |= 2;
            if (a < b) result |= 4;
            if (c > d) result |= 8;
            break;
    }
    
    /* Additional side effect to prevent dead code elimination */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += 1;
    asm volatile("" : : : "memory");
}

int main(void) {
    /* Initialize test values including NaNs */
    double nan_dbl = 0.0 / 0.0;          /* Generate quiet NaN */
    double neg_nan_dbl = -nan_dbl;       /* Negative NaN */
    double inf_dbl = __builtin_inf();    /* Positive infinity */
    double neg_inf_dbl = -inf_dbl;       /* Negative infinity */
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = __builtin_nanf("");  /* Another way to create NaN */
    float neg_nan_flt = -nan_flt;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN generation methods */
    double sqrt_nan = __builtin_sqrt(-1.0);
    float log_nan = __builtin_logf(-1.0f);
    
    int checksum = 0;
    volatile int guard = 0;
    
    /* Test different combinations to hit various code paths */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan, 1.0/zero_dbl
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, log_nan, 1.0f/zero_flt
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations and opcodes */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_flt; j++) {
                /* Vary the operand combinations */
                double a = test_values_dbl[i];
                double b = test_values_dbl[(i + 1) % num_dbl];
                float c = test_values_flt[j];
                float d = test_values_flt[(j + 1) % num_flt];
                
                /* Call with side effects */
                dummy_call(&guard);
                int res = test_nan_comparisons(op, a, b, c, d);
                
                /* Update checksum deterministically */
                checksum = (checksum * 31 + res) & 0xFFFF;
                checksum ^= (i * 17 + j * 13 + op * 11);
                
                /* Mix in some conditional operations */
                if (res & 1) checksum += 1;
                if (res & 2) checksum += 2;
                if (res & 4) checksum += 4;
                if (res & 8) checksum += 8;
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 100; i++) {
        /* Test UNEQ_EXPR patterns */
        checksum += test_nan_comparisons(0, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(0, normal_dbl, nan_dbl, normal_flt, nan_flt);
        checksum += test_nan_comparisons(0, nan_dbl, nan_dbl, nan_flt, nan_flt);
        checksum += test_nan_comparisons(0, normal_dbl, normal_dbl, normal_flt, normal_flt);
        
        /* Test LTGT_EXPR patterns */
        checksum += test_nan_comparisons(1, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(1, normal_dbl, nan_dbl, normal_flt, nan_flt);
        checksum += test_nan_comparisons(1, normal_dbl, normal_dbl * 2, normal_flt, normal_flt * 2);
        checksum += test_nan_comparisons(1, -normal_dbl, normal_dbl, -normal_flt, normal_flt);
        
        /* Test with infinities */
        checksum += test_nan_comparisons(0, inf_dbl, nan_dbl, inf_flt, nan_flt);
        checksum += test_nan_comparisons(1, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt);
        
        /* Prevent loop unrolling from eliminating comparisons */
        dummy_call(&guard);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Guard value: %d\n", guard);
    
    return checksum == 0 ? 0 : 1;
}
