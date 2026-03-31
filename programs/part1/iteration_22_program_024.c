/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    switch (opcode % 4) {
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using __builtin_isunordered and direct equality */
            int unord = __builtin_isunordered(a, b);
            int eq = (a == b);
            if (unord || eq) {
                result = 1;
                sink += 100;
            }
            /* Alternative using macro */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
                sink += 200;
            }
            break;
        }
        
        case 1: { /* LTGT_EXPR-like: ordered and not equal */
            /* Ordered: !unordered AND not equal */
            int ord = !__builtin_isunordered(a, b);
            int ne = (a != b);
            if (ord && ne) {
                result = 3;
                sink += 300;
            }
            /* Using comparison macros */
            if (!isunordered(c, d) && (c != d)) {
                result |= 4;
                sink += 400;
            }
            break;
        }
        
        case 2: { /* Mixed comparisons that might generate UNEQ/LTGT */
            /* Complex expression that could fold to UNEQ_EXPR */
            if ((__builtin_isnan(a) || __builtin_isnan(b)) || (a == b)) {
                result = 5;
                sink += 500;
            }
            /* Expression that could fold to LTGT_EXPR */
            if (!(__builtin_isnan(c) || __builtin_isnan(d)) && (c != d)) {
                result |= 6;
                sink += 600;
            }
            break;
        }
        
        case 3: { /* More complex nested comparisons */
            /* Using builtin comparison functions */
            int cmp1 = __builtin_isgreater(a, b);
            int cmp2 = __builtin_isless(a, b);
            int cmp3 = __builtin_isunordered(a, b);
            
            /* This might generate LTGT_EXPR: (a > b || a < b) && !unordered */
            if ((cmp1 || cmp2) && !cmp3) {
                result = 7;
                sink += 700;
            }
            
            /* This might generate UNEQ_EXPR: unordered || (c == d) */
            if (isunordered(c, d) || !(c != d)) {
                result |= 8;
                sink += 800;
            }
            break;
        }
    }
    
    /* Use sink to prevent dead code elimination */
    return result + (sink & 1);
}

/* Dummy function to create side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int *p) {
    *p += 1;
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
    volatile int global_sink = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
        normal_dbl, zero_dbl, sqrt_nan_dbl, builtin_nan_dbl
    };
    
    float test_values_flt[] = {
        nan_flt, neg_nan_flt, inf_flt, neg_inf_flt,
        normal_flt, zero_flt, sqrt_nan_flt, builtin_nan_flt
    };
    
    int num_tests = sizeof(test_values_dbl) / sizeof(test_values_dbl[0]);
    
    /* Loop through different combinations */
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            for (int op = 0; op < 8; op++) {
                /* Call test function with side effects */
                int res = test_nan_comparisons(
                    op, 
                    test_values_dbl[i], 
                    test_values_dbl[j],
                    test_values_flt[i % 4],
                    test_values_flt[j % 4]
                );
                
                checksum += res;
                checksum ^= (i * 31 + j * 17 + op * 13);
                
                /* Create additional side effects */
                dummy_side_effect(&global_sink);
                
                /* Volatile access to prevent optimization */
                global_sink += res;
            }
        }
    }
    
    /* Also test specific edge cases */
    for (int op = 0; op < 4; op++) {
        /* NaN vs NaN */
        checksum += test_nan_comparisons(op, nan_dbl, nan_dbl, nan_flt, nan_flt);
        
        /* NaN vs normal */
        checksum += test_nan_comparisons(op, nan_dbl, normal_dbl, nan_flt, normal_flt);
        
        /* Inf vs Inf */
        checksum += test_nan_comparisons(op, inf_dbl, inf_dbl, inf_flt, inf_flt);
        
        /* Zero vs Zero */
        checksum += test_nan_comparisons(op, zero_dbl, zero_dbl, zero_flt, zero_flt);
        
        /* Ordered comparison: normal vs normal */
        checksum += test_nan_comparisons(op, normal_dbl, 2.0 * normal_dbl, normal_flt, 2.0f * normal_flt);
    }
    
    printf("Checksum: %d (global_sink: %d)\n", checksum, global_sink);
    
    /* Use checksum to affect return value */
    return (checksum & 255) == 0 ? 0 : 1;
}
