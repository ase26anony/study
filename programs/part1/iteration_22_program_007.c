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
            /* unordered or equal: a == b OR (a != a && b != b) */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Additional complex expression with volatile */
            side_effect += __builtin_isnan(c) ? 1 : 0;
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* less than or greater than (ordered and not equal) */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 2;
            }
            /* Mix with float comparisons */
            side_effect += __builtin_isnan(d) ? 2 : 0;
            break;
            
        case 2: /* Combined comparisons */
            /* Use GCC builtins that map to UNEQ/LTGT */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 4; /* LTGT semantics */
            }
            if (!__builtin_isunordered(a, b) && a == b) {
                result |= 8; /* Ordered equal */
            }
            /* Force evaluation with volatile */
            volatile float tmp = c + d;
            side_effect += (int)tmp;
            break;
            
        case 3: /* Direct NaN checks */
            if (__builtin_isnan(a) && __builtin_isnan(b)) {
                /* This should generate UNEQ comparisons */
                result |= 16;
            }
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                /* This should generate LTGT comparisons */
                result |= 32;
            }
            /* Complex side effect */
            side_effect += (int)(__builtin_fabs(c) + __builtin_fabs(d));
            break;
    }
    
    /* Ensure side effects aren't optimized away */
    asm volatile("" : "+r" (side_effect));
    
    return result + side_effect;
}

/* Dummy function to prevent optimization */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += 1;
}

int main() {
    /* Test values including NaN, Inf, and normal numbers */
    double nan_dbl = 0.0 / 0.0;
    double inf_dbl = __builtin_inf();
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    float nan_flt = __builtin_nanf("");
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Generate NaN through sqrt(-1) */
    double nan_sqrt = __builtin_sqrt(-1.0);
    float nan_sqrtf = __builtin_sqrtf(-1.0f);
    
    int checksum = 0;
    volatile int prevent_dead_code = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {nan_dbl, inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl, nan_sqrt};
    float test_values_flt[] = {nan_flt, inf_flt, normal_flt, zero_flt, nan_sqrtf};
    
    /* Loop through different opcodes and value combinations */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                for (int k = 0; k < 5; k++) {
                    for (int l = 0; l < 5; l++) {
                        if ((i + j + k + l) % 7 == 0) { /* Sample subset */
                            int res = test_nan_comparisons(
                                op,
                                test_values_dbl[i],
                                test_values_dbl[j],
                                test_values_flt[k],
                                test_values_flt[l]
                            );
                            
                            checksum ^= res + op + i + j + k + l;
                            prevent_dead_code += res;
                            
                            /* Call dummy function to create control flow */
                            dummy_call(&checksum);
                        }
                    }
                }
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    
    /* Test 1: Direct UNEQ pattern - unordered or equal */
    volatile double v1 = nan_dbl;
    volatile double v2 = normal_dbl;
    if (__builtin_isunordered(v1, v2) || v1 == v2) {
        checksum += 1000;
    }
    
    /* Test 2: Direct LTGT pattern - ordered and not equal */
    volatile float f1 = normal_flt;
    volatile float f2 = zero_flt;
    if (!__builtin_isunordered(f1, f2) && f1 != f2) {
        checksum += 2000;
    }
    
    /* Test 3: Mixed types and operations */
    for (int i = 0; i < 10; i++) {
        double a = (i % 2) ? nan_dbl : normal_dbl + i;
        double b = (i % 3) ? inf_dbl : normal_dbl - i;
        
        /* Complex expression that might generate UNEQ_EXPR */
        int cmp1 = (a == b) || (__builtin_isnan(a) && __builtin_isnan(b));
        
        /* Complex expression that might generate LTGT_EXPR */
        int cmp2 = (a < b || a > b) && !__builtin_isunordered(a, b);
        
        checksum += cmp1 * 3 + cmp2 * 7;
        
        /* Use results in volatile operations */
        volatile int vol_cmp = cmp1 + cmp2;
        prevent_dead_code += vol_cmp;
    }
    
    /* Test 4: Using math.h comparison macros */
    {
        double x = nan_dbl;
        double y = 1.0;
        
        /* isgreater/isless generate ordered comparisons */
        if (isgreater(x, y) || isless(x, y)) {
            checksum += 3000;  /* LTGT-like */
        }
        
        /* isunordered generates UNEQ component */
        if (isunordered(x, y) || x == y) {
            checksum += 4000;  /* UNEQ-like */
        }
    }
    
    printf("Final checksum: %d (dead code prevent: %d)\n", 
           checksum, prevent_dead_code);
    
    return checksum != 0 ? 0 : 1;
}
