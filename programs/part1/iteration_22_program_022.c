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
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins that might generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Alternative using macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            /* Direct NaN check */
            if (__builtin_isnan(a) || __builtin_isnan(b) || a == b) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins that might generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 1;
            }
            /* Alternative using comparison macros */
            if (isgreater(a, b) || isless(a, b)) {
                result |= 2;
            }
            /* Combined check */
            if (!isunordered(c, d) && c != d) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isgreater(a, b)) result |= 1;
            if (__builtin_isless(c, d)) result |= 2;
            if (isunordered(a + b, c - d)) result |= 4;
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) result |= 8;
            break;
            
        default:
            /* More complex expressions with volatile side effects */
            volatile double tmp = a * b - c / d;
            side_effect += (int)tmp;
            if ((__builtin_isunordered(a, b) || a == b) && 
                (!__builtin_isunordered(c, d) && c != d)) {
                result = side_effect;
            }
            break;
    }
    
    /* Additional side effect to prevent optimization */
    volatile int dummy = side_effect + result;
    (void)dummy;
    
    return result;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("no-tree-pre")))
void run_comparisons(double *dvals, float *fvals, int n, int *checksum) {
    for (int i = 0; i < n - 1; i++) {
        for (int op = 0; op < 4; op++) {
            *checksum += test_nan_comparisons(
                op, 
                dvals[i], 
                dvals[i + 1],
                fvals[i % 4],
                fvals[(i + 1) % 4]
            );
            
            /* Cross-type comparisons */
            *checksum += test_nan_comparisons(
                (op + 1) % 3,
                dvals[i] + fvals[i % 4],
                dvals[i + 1] - fvals[(i + 1) % 4],
                (float)dvals[i],
                (float)dvals[i + 1]
            );
        }
    }
}

int main(void) {
    /* Create various NaN and special values */
    double zero = 0.0;
    double neg_zero = -0.0;
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    /* Different ways to generate NaN */
    double nan1 = 0.0 / zero;           /* 0.0/0.0 */
    double nan2 = __builtin_nan("");    /* Explicit NaN */
    double nan3 = inf - inf;            /* inf - inf */
    double nan4 = sqrt(-1.0);           /* sqrt(-1) */
    
    /* Normal numbers */
    double normal1 = 3.14159;
    double normal2 = -2.71828;
    double normal3 = 42.0;
    double normal4 = -99.9;
    
    /* Float versions */
    float fnan1 = 0.0f / 0.0f;
    float fnan2 = __builtin_nanf("");
    float fnormal1 = 1.5f;
    float fnormal2 = -3.7f;
    
    /* Arrays of test values */
    double dvals[] = {
        nan1, normal1, nan2, normal2, 
        inf, neg_inf, nan3, normal3,
        nan4, normal4, zero, neg_zero
    };
    
    float fvals[] = {
        fnan1, fnormal1, fnan2, fnormal2,
        0.0f, -0.0f, __builtin_inff(), -__builtin_inff()
    };
    
    int checksum = 0;
    
    /* Run multiple iterations with different value combinations */
    for (int iter = 0; iter < 3; iter++) {
        run_comparisons(dvals, fvals, 
                       sizeof(dvals)/sizeof(dvals[0]),
                       &checksum);
        
        /* Modify values slightly each iteration */
        dvals[1] += 1.0;
        fvals[1] += 0.5f;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int final_result = checksum;
    return final_result % 256;
}
