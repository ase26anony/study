/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects to prevent early optimization */
    side_effect += (int)(a * 1000.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like behavior */
            /* unordered or equal: isnan(a) || isnan(b) || a == b */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(b) || a == b) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: !isnan(a) && !isnan(b) && a != b */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 1;
            }
            if (!isunordered(c, d) && c != d) {
                result |= 2;
            }
            /* Alternative formulation */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ and LTGT patterns */
            if ((__builtin_isunordered(a, b) || a == b) && 
                (!__builtin_isunordered(c, d) && c != d)) {
                result |= 8;
            }
            break;
            
        default:
            /* Generate standard comparisons that might become UNEQ/LTGT */
            if (a < b) result |= 16;
            if (c > d) result |= 32;
            if (a == b) result |= 64;
            if (c != d) result |= 128;
            break;
    }
    
    /* More side effects to prevent dead code elimination */
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
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan_dbl = -0.0 / 0.0;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = 0.0f / 0.0f;
    float sqrt_nan_flt = sqrtf(-1.0f);
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Explicit NaN using builtin */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    int checksum = 0;
    volatile int dummy = 0;
    
    /* Test different combinations to generate various comparison patterns */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, builtin_nan, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, 1.0, -1.0, 100.0
    };
    
    float test_values_flt[] = {
        nan_flt, sqrt_nan_flt, builtin_nanf, inf_flt, normal_flt,
        zero_flt, 1.0f, -1.0f, 100.0f
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_flt; j++) {
                for (int k = 0; k < num_dbl; k++) {
                    if (k % 3 == 0) continue; /* Skip some to reduce runtime */
                    
                    /* Call the comparison function */
                    int res = test_nan_comparisons(
                        op,
                        test_values_dbl[i],
                        test_values_dbl[k],
                        test_values_flt[j],
                        test_values_flt[(j + 1) % num_flt]
                    );
                    
                    checksum ^= (res * 31) + op + i + j + k;
                    
                    /* Add side effects */
                    dummy += res;
                    dummy_call(&dummy);
                }
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 100; i++) {
        /* UNEQ_EXPR patterns */
        checksum += test_nan_comparisons(0, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(0, normal_dbl, nan_dbl, normal_flt, nan_flt);
        checksum += test_nan_comparisons(0, nan_dbl, nan_dbl, nan_flt, nan_flt);
        checksum += test_nan_comparisons(0, normal_dbl, normal_dbl, normal_flt, normal_flt);
        
        /* LTGT_EXPR patterns */
        checksum += test_nan_comparisons(1, normal_dbl, normal_dbl * 2.0, normal_flt, normal_flt * 2.0f);
        checksum += test_nan_comparisons(1, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(1, inf_dbl, neg_inf_dbl, inf_flt, -inf_flt);
        
        /* Mixed with side effects */
        dummy += test_nan_comparisons(2, 
            i % 2 ? nan_dbl : normal_dbl,
            i % 3 ? builtin_nan : inf_dbl,
            i % 5 ? nan_flt : normal_flt,
            i % 7 ? builtin_nanf : inf_flt
        );
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Dummy: %d\n", dummy);
    
    return checksum != 0 ? 0 : 1;
}
