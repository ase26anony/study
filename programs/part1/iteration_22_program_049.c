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
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like behavior */
            /* unordered or equal: (a != a) || (b != b) || (a == b) */
            if (__builtin_isunordered(a, b) || (a == b)) {
                result |= 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(b) || (a == b)) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: (a == a) && (b == b) && (a != b) */
            if (!__builtin_isunordered(a, b) && (a != b)) {
                result |= 1;
            }
            if (!isunordered(c, d) && (c != d)) {
                result |= 2;
            }
            /* Ordered comparison using builtins */
            if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate various comparison tree codes */
            if (__builtin_islessequal(a, b)) result |= 1;
            if (isgreater(c, d)) result |= 2;
            if (__builtin_isunordered(a, 0.0)) result |= 4;
            if (!__builtin_isnan(b) && (b != b)) result |= 8; /* Always false but complex */
            break;
            
        default:
            /* Generate tcc_comparison class codes */
            if (a < b) result |= 1;
            if (c > d) result |= 2;
            if (a == b) result |= 4;
            if (a != b) result |= 8;
            break;
    }
    
    /* More side effects to prevent optimization */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result + side_effect;
}

/* Dummy non-inlineable function for additional side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int *p) {
    *p += 1;
    asm volatile("" : : "r"(p) : "memory");
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
    volatile int counter = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, normal_dbl, 
        zero_dbl, sqrt_nan, log_nan, 1.0, -1.0
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt, normal_flt,
        zero_flt, acos_nan, 1.0f, -1.0f, 0.5f
    };
    
    int num_dbl = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    int num_flt = sizeof(test_values_flt) / sizeof(test_values_flt[0]);
    
    /* Loop through different combinations */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_dbl; i++) {
            for (int j = 0; j < num_flt; j++) {
                /* Mix double and float comparisons */
                int res1 = test_nan_comparisons(
                    op, 
                    test_values_dbl[i], 
                    test_values_dbl[(i + 1) % num_dbl],
                    test_values_flt[j],
                    test_values_flt[(j + 1) % num_flt]
                );
                
                checksum ^= res1 + op + i + j;
                counter++;
                
                /* Call dummy function for side effects */
                dummy_side_effect(&checksum);
                
                /* Test with same-type comparisons */
                int res2 = test_nan_comparisons(
                    op,
                    test_values_dbl[i],
                    test_values_dbl[j % num_dbl],
                    test_values_flt[i % num_flt],
                    test_values_flt[j]
                );
                
                checksum ^= res2 * 31;
                counter++;
            }
        }
    }
    
    /* Additional specific tests for UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 100; i++) {
        /* Test UNEQ_EXPR patterns */
        checksum += test_nan_comparisons(
            0, /* UNEQ_EXPR-like */
            (i % 3 == 0) ? nan_dbl : normal_dbl,
            (i % 5 == 0) ? nan_dbl : normal_dbl + i,
            (i % 2 == 0) ? nan_flt : normal_flt,
            (i % 7 == 0) ? nan_flt : normal_flt - i
        );
        
        /* Test LTGT_EXPR patterns */
        checksum ^= test_nan_comparisons(
            1, /* LTGT_EXPR-like */
            (i % 4 == 0) ? sqrt_nan : (double)i,
            (i % 6 == 0) ? log_nan : (double)(i * 2),
            (i % 3 == 0) ? acos_nan : (float)i,
            (i % 5 == 0) ? neg_nan_flt : (float)(i + 1)
        );
        
        dummy_side_effect(&counter);
    }
    
    /* Ensure all code paths are used */
    if (counter > 0) {
        printf("Checksum: %d\n", checksum);
        printf("Iterations: %d\n", counter);
    }
    
    return checksum != 0 ? 0 : 1;
}
