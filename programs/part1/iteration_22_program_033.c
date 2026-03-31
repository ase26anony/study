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
            /* unordered or equal: a == b OR (a != a && b != b) */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Additional complex expression with volatile */
            side_effect += __builtin_isnan(c) ? 1 : 0;
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: a != b AND !(a != a || b != b) */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 2;
            }
            /* Mix with float comparisons */
            side_effect += isgreater(c, d) ? 2 : 0;
            break;
            
        case 2: /* Combined comparisons */
            /* Use GCC builtins that may generate UNEQ_EXPR/LTGT_EXPR */
            if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) {
                result |= 4;
            }
            if (!__builtin_islessequal(c, d) && !__builtin_isunordered(c, d)) {
                result |= 8;
            }
            side_effect += (int)c + (int)d;
            break;
            
        case 3: /* Direct NaN checks with comparisons */
            if ((__builtin_isnan(a) && __builtin_isnan(b)) || a == b) {
                result |= 16;
            }
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 32;
            }
            /* Force side effect with volatile */
            *(volatile int*)&side_effect = side_effect + 1;
            break;
    }
    
    /* More side effects to prevent optimization */
    if (side_effect > 1000) {
        /* This should never happen, but prevents dead code elimination */
        printf("Impossible!\n");
    }
    
    return result;
}

/* Another non-inlineable function with different comparison patterns */
static __attribute__((noinline, optimize("no-tree-vectorize")))
int test_mixed_comparisons(double x, double y, float f1, float f2) {
    volatile double v = 0.0;
    int checksum = 0;
    
    /* Generate various comparison expressions */
    checksum += isunordered(x, y) ? 1 : 0;
    v = x + y;
    
    checksum += (!isunordered(f1, f2) && f1 != f2) ? 2 : 0;
    v += f1 * f2;
    
    checksum += ((x != x && y != y) || x == y) ? 4 : 0;
    *(volatile double*)&v = v;
    
    checksum += (!(x != x || y != y) && x != y) ? 8 : 0;
    
    /* Use math.h macros that map to GCC builtins */
    checksum += isgreater(x, y) ? 16 : 0;
    checksum += isless(f1, f2) ? 32 : 0;
    
    return checksum;
}

int main(void) {
    /* Initialize test values including NaNs */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan = -nan_dbl;
    double inf = __builtin_inf();
    double neg_inf = -inf;
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    
    float nan_flt = 0.0f / 0.0f;
    float sqrt_neg = sqrtf(-1.0f);
    float normal_flt1 = 1.41421f;
    float normal_flt2 = 1.73205f;
    
    /* Explicit NaN using builtin */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    int total_checksum = 0;
    
    /* Test different combinations in a loop */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            double a, b;
            float c, d;
            
            /* Select different value combinations */
            switch (j) {
                case 0: a = nan_dbl; b = normal1; c = nan_flt; d = normal_flt1; break;
                case 1: a = normal2; b = builtin_nan; c = normal_flt2; d = builtin_nanf; break;
                case 2: a = nan_dbl; b = builtin_nan; c = nan_flt; d = sqrt_neg; break;
                case 3: a = inf; b = neg_inf; c = 1.0f; d = -1.0f; break;
                case 4: a = normal1; b = normal2; c = normal_flt1; d = normal_flt2; break;
                case 5: a = neg_nan; b = inf; c = builtin_nanf; d = 0.0f; break;
            }
            
            /* Call test functions with side effects */
            total_checksum += test_nan_comparisons(i, a, b, c, d);
            total_checksum += test_mixed_comparisons(a, b, c, d);
            
            /* Mix arguments to create more patterns */
            total_checksum += test_nan_comparisons((i + 1) % 4, b, a, d, c);
        }
    }
    
    /* Use results to compute deterministic checksum */
    printf("Result checksum: %d\n", total_checksum % 1000);
    
    return 0;
}
