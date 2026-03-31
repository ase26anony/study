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
    side_effect += (int)(a * 1000.0);
    
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
            /* less than or greater than (ordered and not equal) */
            if (__builtin_islessgreater(a, b)) {
                result |= 1;
            }
            /* Alternative using individual comparisons */
            if ((a < b) || (a > b)) {
                result |= 2;
            }
            /* With explicit NaN checks */
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && (a != b)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            if (__builtin_isgreater(a, b)) {
                result |= 1;
            }
            if (__builtin_isless(c, d)) {
                result |= 2;
            }
            if (isunordered(a, b) && !__builtin_isnan(c)) {
                result |= 4;
            }
            break;
            
        default:
            /* More complex nested comparisons */
            if ((__builtin_isunordered(a, b) ? (c == d) : (a != b))) {
                result |= 1;
            }
            if ((!__builtin_isnan(a) && !__builtin_isnan(b)) && (a != b)) {
                result |= 2;
            }
            break;
    }
    
    /* Additional side effects to prevent optimization */
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
    /* Create various floating-point values */
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
    double sqrt_nan_dbl = sqrt(-1.0);
    float sqrt_nan_flt = sqrtf(-1.0f);
    double builtin_nan_dbl = __builtin_nan("");
    float builtin_nan_flt = __builtin_nanf("");
    
    int checksum = 0;
    volatile int prevent_opt = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan_dbl, builtin_nan_dbl
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, sqrt_nan_flt, builtin_nan_flt
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_dbl; j++) {
                for (int k = 0; k < num_flt; k++) {
                    for (int l = 0; l < num_flt; l++) {
                        if ((i + j + k + l) % 3 == 0) { /* Skip some to limit runtime */
                            int res = test_nan_comparisons(
                                op, 
                                test_values_dbl[i], 
                                test_values_dbl[j],
                                test_values_flt[k],
                                test_values_flt[l]
                            );
                            checksum ^= (res << ((i + j + k + l) % 16));
                            dummy_call(&prevent_opt);
                        }
                    }
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
        checksum += test_nan_comparisons(1, nan_dbl, normal_dbl, nan_flt, normal_flt);
        checksum += test_nan_comparisons(1, normal_dbl, nan_dbl, normal_flt, nan_flt);
        checksum += test_nan_comparisons(1, normal_dbl, normal_dbl * 2.0, normal_flt, normal_flt * 2.0f);
        checksum += test_nan_comparisons(1, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt);
        
        dummy_call(&prevent_opt);
    }
    
    /* Complex nested expression that might generate UNEQ_EXPR/LTGT_EXPR */
    for (int i = 0; i < 50; i++) {
        double a = (i % 2) ? nan_dbl : normal_dbl + i;
        double b = (i % 3) ? sqrt_nan_dbl : normal_dbl - i;
        float c = (i % 4) ? nan_flt : normal_flt + i;
        float d = (i % 5) ? builtin_nan_flt : normal_flt - i;
        
        /* This complex condition may generate various comparison tree codes */
        int r1 = test_nan_comparisons(i % 4, a, b, c, d);
        int r2 = test_nan_comparisons((i + 1) % 4, b, a, d, c);
        
        checksum = (checksum * 31 + r1) ^ (r2 * 17);
        prevent_opt += r1 + r2;
    }
    
    printf("Final checksum: %d (prevent_opt: %d)\n", checksum, prevent_opt);
    return checksum != 0 ? 0 : 1;
}
