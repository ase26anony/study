/* test_nan_folding.c - Target specific uncovered lines in fold-const.cc */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int global_counter = 0;
static int __attribute__((noinline)) dummy_side_effect(int x) {
    global_counter += x;
    return global_counter;
}

/* Helper to create NaN values in different ways */
static double make_nan_double(int method) {
    switch (method) {
        case 0: return 0.0 / 0.0;          /* Division by zero NaN */
        case 1: return __builtin_nan("");  /* Quiet NaN */
        case 2: return -__builtin_nan(""); /* Negative NaN */
        case 3: {
            double x = -1.0;
            return sqrt(x);                /* sqrt(-1) produces NaN */
        }
        default: return __builtin_nan("");
    }
}

static float make_nan_float(int method) {
    switch (method) {
        case 0: return 0.0f / 0.0f;
        case 1: return __builtin_nanf("");
        case 2: return -__builtin_nanf("");
        case 3: {
            float x = -1.0f;
            return sqrtf(x);
        }
        default: return __builtin_nanf("");
    }
}

/* Non-inlineable function with complex NaN-aware comparisons */
static int __attribute__((noinline)) 
test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    volatile int result = 0;
    
    /* Use different comparison types based on opcode */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like comparisons (unordered or equal) */
            /* Double comparisons using builtins */
            if (__builtin_isunordered(d1, d2) || d1 == d2) {
                result += dummy_side_effect(1);
            }
            /* Float comparisons using macros */
            if (isunordered(f1, f2) || f1 == f2) {
                result += dummy_side_effect(2);
            }
            /* Mixed type comparisons */
            if (__builtin_isunordered(d1, (double)f2) || d1 == (double)f2) {
                result += dummy_side_effect(3);
            }
            break;
            
        case 1: /* LTGT_EXPR-like comparisons (ordered and not equal) */
            /* Double: ordered and (less than or greater than) */
            if (!__builtin_isunordered(d1, d2) && (d1 < d2 || d1 > d2)) {
                result += dummy_side_effect(4);
            }
            /* Float: using isless/greater macros */
            if (!isunordered(f1, f2) && (isless(f1, f2) || isgreater(f1, f2))) {
                result += dummy_side_effect(5);
            }
            /* Alternative formulation */
            if (__builtin_isless(d1, d2) || __builtin_isgreater(d1, d2)) {
                result += dummy_side_effect(6);
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            /* Direct isnan checks combined with comparisons */
            if (__builtin_isnan(d1) || __builtin_isnan(d2)) {
                if (d1 != d2) {
                    result += dummy_side_effect(7);
                }
            }
            if (!__builtin_isnan(f1) && !__builtin_isnan(f2)) {
                if (f1 < f2 || f1 > f2) {
                    result += dummy_side_effect(8);
                }
            }
            break;
            
        default:
            /* Complex expression that might generate UNEQ_EXPR/LTGT_EXPR */
            int temp = (__builtin_isunordered(d1, d2) ? 1 : 0) +
                      (isunordered(f1, f2) ? 2 : 0) +
                      ((d1 == d2) ? 4 : 0) +
                      ((f1 == f2) ? 8 : 0);
            result += dummy_side_effect(temp);
            break;
    }
    
    /* Additional side effects to prevent optimization */
    volatile double check = d1 + d2;
    volatile float fcheck = f1 + f2;
    (void)check;
    (void)fcheck;
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Test values including normals, infinities, and NaNs */
    double test_doubles[] = {
        1.0, -1.0, 0.0, -0.0,
        __builtin_inf(), -__builtin_inf(),
        make_nan_double(0), make_nan_double(1),
        2.5, -3.7, 1e308, -1e308
    };
    
    float test_floats[] = {
        1.0f, -1.0f, 0.0f, -0.0f,
        __builtin_inff(), -__builtin_inff(),
        make_nan_float(0), make_nan_float(1),
        2.5f, -3.7f, 1e20f, -1e20f
    };
    
    int num_doubles = sizeof(test_doubles) / sizeof(test_doubles[0]);
    int num_floats = sizeof(test_floats) / sizeof(test_floats[0]);
    
    /* Test different combinations and opcodes */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_doubles; i++) {
            for (int j = 0; j < num_floats; j++) {
                /* Vary the combinations to test different scenarios */
                int idx1 = (i + op) % num_doubles;
                int idx2 = (j + op) % num_floats;
                int idx3 = (i + j) % num_doubles;
                int idx4 = (i * j) % num_floats;
                
                checksum += test_nan_comparisons(
                    op,
                    test_doubles[idx1],
                    test_doubles[idx3],
                    test_floats[idx2],
                    test_floats[idx4]
                );
                
                /* Test with same values */
                checksum += test_nan_comparisons(
                    op,
                    test_doubles[i],
                    test_doubles[i],
                    test_floats[j],
                    test_floats[j]
                );
                
                /* Test with NaN vs non-NaN */
                if (i >= 6) { /* NaN indices */
                    checksum += test_nan_comparisons(
                        op,
                        test_doubles[i],
                        test_doubles[0], /* non-NaN */
                        test_floats[j],
                        test_floats[0]   /* non-NaN */
                    );
                }
            }
        }
    }
    
    /* Additional specific tests for edge cases */
    double zero = 0.0;
    double neg_zero = -0.0;
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double nan1 = make_nan_double(0);
    double nan2 = make_nan_double(1);
    
    /* These should generate UNEQ_EXPR and LTGT_EXPR nodes */
    checksum += test_nan_comparisons(0, zero, neg_zero, 0.0f, -0.0f);
    checksum += test_nan_comparisons(1, inf, neg_inf, __builtin_inff(), -__builtin_inff());
    checksum += test_nan_comparisons(0, nan1, 1.0, make_nan_float(0), 1.0f);
    checksum += test_nan_comparisons(1, 1.0, nan1, 1.0f, make_nan_float(0));
    checksum += test_nan_comparisons(2, nan1, nan2, make_nan_float(0), make_nan_float(1));
    
    /* Complex expression that might fold differently */
    volatile double complex_expr = (nan1 == nan2) ? 1.0 : 2.0;
    volatile float complex_expr_f = (make_nan_float(0) == make_nan_float(1)) ? 1.0f : 2.0f;
    (void)complex_expr;
    (void)complex_expr_f;
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
