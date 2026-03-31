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
            /* Using builtins that might generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result = 1;
                sink += 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
                sink += 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins that might generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
                sink += 3;
            }
            /* Alternative formulation */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 2;
                sink += 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Complex expression that might generate both tree codes */
            if ((__builtin_isnan(a) && isunordered(b, c)) || 
                (!__builtin_isnan(d) && __builtin_islessequal(a, b))) {
                result = 1;
                sink += 5;
            }
            break;
            
        case 3: /* Direct NaN checks with comparisons */
            if ((a != a) || (b != b) || /* NaN checks */
                (__builtin_isnan(c) && !__builtin_isnan(d))) {
                result = 1;
                sink += 6;
            }
            /* Ordered comparison that ignores NaNs */
            if (__builtin_islessgreater(a, b)) {
                result |= 2;
                sink += 7;
            }
            break;
    }
    
    /* Use sink to prevent optimization */
    return result + (sink & 1);
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline))
void dummy_side_effect(int *counter) {
    *counter += 1;
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
    double sqrt_nan = __builtin_sqrt(-1.0);
    double log_nan = __builtin_log(-1.0);
    float acos_nan = __builtin_acosf(2.0f);
    
    int checksum = 0;
    int side_counter = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan, log_nan
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, acos_nan
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through many combinations */
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_dbl; j++) {
                for (int k = 0; k < num_flt; k++) {
                    for (int l = 0; l < num_flt; l++) {
                        if ((i + j + k + l) % 7 == 0) { /* Sample subset */
                            int res = test_nan_comparisons(
                                op + i + j, 
                                test_values_dbl[i],
                                test_values_dbl[j],
                                test_values_flt[k],
                                test_values_flt[l]
                            );
                            checksum += res;
                            dummy_side_effect(&side_counter);
                        }
                    }
                }
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    volatile double v1 = __builtin_nan("");
    volatile double v2 = __builtin_nan("0x1234");
    volatile float v3 = __builtin_nanf("");
    volatile float v4 = __builtin_nanf("0xabcd");
    
    /* Direct comparisons that should generate specific tree codes */
    for (int i = 0; i < 100; i++) {
        /* UNEQ_EXPR patterns */
        if (__builtin_isunordered(v1, v2) || v1 == v2) {
            checksum += i;
        }
        
        /* LTGT_EXPR patterns */
        if (!__builtin_isunordered(v3, v4) && v3 != v4) {
            checksum -= i;
        }
        
        /* Using islessgreater macro */
        if (islessgreater(v1 + i, v2 - i)) {
            checksum += 2 * i;
        }
        
        dummy_side_effect(&side_counter);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Side effects: %d\n", side_counter);
    
    return 0;
}
