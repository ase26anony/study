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
                result |= 1;
            }
            /* Using standard macros */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(b) || (a == b)) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* less than or greater than: ordered and not equal */
            if (!__builtin_isunordered(a, b) && (a != b)) {
                result |= 1;
            }
            if (!isunordered(c, d) && (c != d)) {
                result |= 2;
            }
            /* Alternative formulation */
            if ((a < b) || (a > b)) {  /* This becomes LTGT_EXPR */
                result |= 4;
            }
            if ((c < d) || (c > d)) {
                result |= 8;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ_EXPR and LTGT_EXPR in same basic block */
            if (__builtin_isgreater(a, b) && !__builtin_isnan(a) && !__builtin_isnan(b)) {
                result |= 1;
            }
            if (islessequal(c, d) || isunordered(c, d)) {
                result |= 2;
            }
            /* Direct comparisons that might fold */
            if ((a != a) || (b != b) || (a == b)) {  /* Potential UNEQ_EXPR */
                result |= 4;
            }
            if ((a < b) || (a > b)) {  /* Potential LTGT_EXPR */
                result |= 8;
            }
            break;
    }
    
    /* More side effects to prevent optimization */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result + side_effect;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += 1;
    asm volatile("" : : : "memory");
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
    double sqrt_nan_dbl = __builtin_sqrt(-1.0);
    float sqrt_nan_flt = __builtin_sqrtf(-1.0f);
    double builtin_nan_dbl = __builtin_nan("");
    float builtin_nan_flt = __builtin_nanf("");
    
    int checksum = 0;
    volatile int guard = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl,
        sqrt_nan_dbl, builtin_nan_dbl
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt, normal_flt, zero_flt,
        sqrt_nan_flt, builtin_nan_flt
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations */
    for (int op = 0; op < 3; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_dbl; j++) {
                for (int k = 0; k < num_flt; k++) {
                    for (int l = 0; l < num_flt; l++) {
                        if ((i + j + k + l) % 7 == 0) {  /* Sample subset */
                            int res = test_nan_comparisons(
                                op,
                                test_values_dbl[i],
                                test_values_dbl[j],
                                test_values_flt[k],
                                test_values_flt[l]
                            );
                            checksum ^= res + op + i + j + k + l;
                            dummy_call(&guard);
                        }
                    }
                }
            }
        }
    }
    
    /* Use results to compute deterministic checksum */
    checksum += guard;
    
    /* Print checksum to ensure code isn't dead */
    printf("Result checksum: %d\n", checksum);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile int final_result = checksum;
    
    return final_result != 0 ? 0 : 1;
}
