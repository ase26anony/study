/* test_nan_comparisons.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    switch (opcode % 4) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins that may generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result = 1;
            }
            /* Additional UNEQ pattern using macros */
            if (!__builtin_isgreater(c, d) && !__builtin_isless(c, d)) {
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using GCC builtins that may generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
            }
            /* Additional LTGT pattern */
            if (__builtin_islessgreater(c, d)) {
                result |= 2;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Complex expression that might fold to UNEQ/LTGT */
            if ((__builtin_isnan(a) || a == b) && 
                (__builtin_isnan(b) || b == a)) {
                result = 1;
            }
            if (!__builtin_isnan(c) && !__builtin_isnan(d) && c != d) {
                result |= 2;
            }
            break;
            
        case 3: /* Direct builtin usage */
            /* These may generate the target tree codes */
            result = __builtin_islessgreater(a, b) ? 1 : 0;
            result |= isunordered(c, d) ? 2 : 0;
            break;
    }
    
    /* More side effects to prevent optimization */
    sink += result;
    asm volatile("" : "+r" (sink) : : "memory");
    
    return result;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_side_effect(int *p) {
    *p += 1;
    asm volatile("" : : "r" (p) : "memory");
}

int main(void) {
    /* Create various floating-point values */
    double nan_dbl = 0.0 / 0.0;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = __builtin_nanf("");
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -__builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN sources */
    double sqrt_neg = __builtin_sqrt(-1.0);
    float log_neg = __builtin_logf(-1.0f);
    
    int checksum = 0;
    volatile int counter = 0;
    
    /* Test many combinations to increase coverage */
    double test_values_dbl[] = {
        nan_dbl, inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl, sqrt_neg
    };
    
    float test_values_flt[] = {
        nan_flt, inf_flt, neg_inf_flt, normal_flt, zero_flt, log_neg
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_dbl; j++) {
                for (int k = 0; k < num_flt; k++) {
                    for (int l = 0; l < num_flt; l++) {
                        /* Call with different operation codes */
                        int res = test_nan_comparisons(
                            op + i + j,  /* Varying opcode */
                            test_values_dbl[i],
                            test_values_dbl[j],
                            test_values_flt[k],
                            test_values_flt[l]
                        );
                        
                        checksum ^= (res << (op & 3));
                        checksum += res;
                        
                        /* Create side effects */
                        dummy_side_effect(&counter);
                    }
                }
            }
        }
    }
    
    /* Use results to compute deterministic output */
    printf("Checksum: %d\n", checksum);
    printf("Counter: %d\n", counter);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile int final_check = checksum + counter;
    
    return (final_check != 0) ? 0 : 1;
}
