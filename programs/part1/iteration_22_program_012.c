/* test_nan_comparisons.c - Targeting fold-const.cc lines 6248-6258 */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int dummy_sink;
static void __attribute__((noinline)) use_result(int cond) {
    dummy_sink = cond; /* Side effect to prevent optimization */
}

/* Non-inlineable helper with complex comparison logic */
static int __attribute__((noinline,noipa))
test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int result = 0;
    
    /* Multiple comparison types in switch-like structure */
    switch (opcode) {
        case 0: /* UNEQ_EXPR path - unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Complex expression with side effects */
            use_result(__builtin_isunordered(c, d) || c == d);
            break;
            
        case 1: /* LTGT_EXPR path - ordered and not equal */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 2;
            }
            /* Embedded in larger expression */
            dummy_sink = !__builtin_isunordered(c, d) && c != d;
            break;
            
        case 2: /* Mixed comparisons */
            if (__builtin_isgreater(a, b) && __builtin_isless(c, d)) {
                result |= 4;
            }
            /* Use isnan() directly */
            if (__builtin_isnan(a) || __builtin_isnan(c)) {
                result |= 8;
            }
            break;
            
        case 3: /* More complex UNEQ/LTGT combinations */
            {
                int uneq = (__builtin_isunordered(a, b) || a == b);
                int ltgt = (!__builtin_isunordered(a, b) && a != b);
                result |= (uneq ? 16 : 0) | (ltgt ? 32 : 0);
                
                /* Force evaluation with volatile */
                volatile int tmp = uneq + ltgt;
                dummy_sink = tmp;
            }
            break;
    }
    
    return result;
}

/* Another helper with different comparison patterns */
static int __attribute__((noinline,noipa))
compare_with_nan_handling(int mode, double x, double y) {
    int res = 0;
    
    if (mode == 0) {
        /* UNEQ_EXPR pattern using standard macros */
        res = isunordered(x, y) || x == y;
        /* Side effect */
        volatile double tmp = x + y;
        (void)tmp;
    } else if (mode == 1) {
        /* LTGT_EXPR pattern */
        res = !isunordered(x, y) && x != y;
        /* Complex expression preventing early folding */
        res += __builtin_isnan(x) ? 0 : 1;
    } else {
        /* Both comparisons in one expression */
        res = (isunordered(x, y) || x == y) ? 100 : 
              (!isunordered(x, y) && x != y) ? 200 : 0;
    }
    
    return res;
}

int main(void) {
    /* Initialize test values including NaNs */
    double nan_dbl = 0.0 / 0.0;          /* Quiet NaN */
    double neg_nan = -__builtin_nan(""); /* Negative NaN */
    double inf_pos = __builtin_inf();
    double inf_neg = -__builtin_inf();
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt1 = 1.414f;
    float normal_flt2 = 1.732f;
    
    /* Additional NaN sources */
    double sqrt_neg = sqrt(-1.0);        /* May produce NaN */
    double log_neg = log(-1.0);          /* May produce NaN */
    
    uint64_t checksum = 0;
    int iterations = 0;
    
    /* Test various combinations */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            double a, b;
            float c, d;
            
            /* Select different value combinations */
            switch (j) {
                case 0: a = nan_dbl; b = normal1; c = nan_flt; d = normal_flt1; break;
                case 1: a = normal1; b = nan_dbl; c = normal_flt1; d = nan_flt; break;
                case 2: a = nan_dbl; b = nan_dbl; c = nan_flt; d = nan_flt; break;
                case 3: a = normal1; b = normal2; c = normal_flt1; d = normal_flt2; break;
                case 4: a = inf_pos; b = nan_dbl; c = inf_flt; d = nan_flt; break;
                case 5: a = sqrt_neg; b = log_neg; c = nan_flt; d = normal_flt1; break;
            }
            
            /* Call helper functions multiple times */
            int r1 = test_nan_comparisons(i, a, b, c, d);
            int r2 = compare_with_nan_handling(i % 3, a, b);
            
            checksum += (r1 * 31 + r2 * 17) * (i + 1) * (j + 1);
            iterations++;
            
            /* Additional direct comparisons to generate tree nodes */
            volatile int cmp1 = __builtin_isunordered(a, b) || a == b;  /* UNEQ */
            volatile int cmp2 = !__builtin_isunordered(a, b) && a != b; /* LTGT */
            checksum += cmp1 + cmp2 * 3;
        }
    }
    
    /* Test with constants mixed with NaN values */
    for (int k = 0; k < 10; k++) {
        double val = (k % 2) ? nan_dbl : (k * 1.5);
        float fval = (k % 3) ? nan_flt : (k * 0.7f);
        
        /* Generate both UNEQ and LTGT expressions */
        int uneq_expr = isunordered(val, 42.0) || val == 42.0;
        int ltgt_expr = !isunordered(fval, 3.14f) && fval != 3.14f;
        
        checksum += uneq_expr * 7 + ltgt_expr * 11;
        
        /* Complex expression with side effect */
        volatile double tmp = uneq_expr ? val : fval;
        (void)tmp;
    }
    
    printf("Checksum: %lu (iterations: %d)\n", 
           (unsigned long)checksum, iterations);
    
    /* Use checksum to affect return value (prevent dead code) */
    return (checksum & 255) == 0 ? 0 : 1;
}
