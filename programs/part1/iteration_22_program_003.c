/* test_nan_folding.c - Target fold-const.cc lines 6248-6258 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int side_effect_counter = 0;
static void __attribute__((noinline)) dummy_side_effect(int x) {
    side_effect_counter += x;
}

/* Helper to create NaN values using different methods */
static double make_nan_double(int method) {
    switch (method) {
        case 0: return 0.0 / 0.0;                    /* Quiet NaN */
        case 1: return __builtin_nan("");            /* Explicit quiet NaN */
        case 2: return -__builtin_nan("");           /* Negative quiet NaN */
        case 3: return __builtin_nan("0xdeadbeef");  /* NaN with payload */
        case 4: return __builtin_nans("");           /* Signaling NaN if supported */
        default: return __builtin_inf() * 0.0;       /* Another NaN source */
    }
}

static float make_nan_float(int method) {
    switch (method) {
        case 0: return 0.0f / 0.0f;
        case 1: return __builtin_nanf("");
        case 2: return -__builtin_nanf("");
        case 3: return __builtin_nanf("0xcafe");
        case 4: return __builtin_nansf("");
        default: return __builtin_inff() * 0.0f;
    }
}

/* Non-inlineable function with complex NaN-aware comparisons */
static int __attribute__((noinline)) 
test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    volatile int result = 0;
    
    /* Use switch-like structure to generate different comparison tree codes */
    if (opcode == 0) {
        /* UNEQ_EXPR generation: unordered or equal */
        /* Using __builtin_isunordered() and equality check */
        if (__builtin_isunordered(d1, d2) || d1 == d2) {
            dummy_side_effect(1);
            result |= 1;
        }
        
        /* Another UNEQ_EXPR pattern with floats */
        if (isunordered(f1, f2) || f1 == f2) {
            dummy_side_effect(2);
            result |= 2;
        }
        
        /* Complex expression mixing builtins and regular operators */
        volatile double tmp = d1;
        if ((__builtin_isnan(tmp) || __builtin_isnan(d2)) || 
            (!__builtin_isnan(tmp) && !__builtin_isnan(d2) && tmp == d2)) {
            dummy_side_effect(3);
            result |= 4;
        }
    }
    else if (opcode == 1) {
        /* LTGT_EXPR generation: ordered and not equal (less than or greater than) */
        /* Using __builtin_isgreater() and __builtin_isless() */
        if ((__builtin_isgreater(d1, d2) || __builtin_isless(d1, d2)) && 
            !__builtin_isunordered(d1, d2)) {
            dummy_side_effect(4);
            result |= 8;
        }
        
        /* Alternative LTGT_EXPR pattern with floats */
        if ((isgreater(f1, f2) || isless(f1, f2)) && !isunordered(f1, f2)) {
            dummy_side_effect(5);
            result |= 16;
        }
        
        /* More complex LTGT_EXPR with side effects in subexpressions */
        volatile float ftmp = f1;
        if (!isunordered(ftmp, f2) && ftmp != f2) {
            dummy_side_effect(6);
            result |= 32;
        }
    }
    else if (opcode == 2) {
        /* Mixed comparisons to generate both tree codes */
        /* First UNEQ_EXPR */
        volatile double dtmp = d1;
        if (__builtin_isunordered(dtmp, d2) || dtmp == d2) {
            dummy_side_effect(7);
            result |= 64;
        }
        
        /* Then LTGT_EXPR */
        if (!__builtin_isunordered(f1, f2) && f1 != f2) {
            dummy_side_effect(8);
            result |= 128;
        }
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Create test values including NaNs, infinities, and normal numbers */
    double d_nan[6];
    float f_nan[6];
    double d_normal[] = {1.0, -1.0, 0.0, -0.0, 2.5, -3.14};
    float f_normal[] = {1.0f, -1.0f, 0.0f, -0.0f, 2.5f, -3.14f};
    double d_inf = __builtin_inf();
    double d_neg_inf = -__builtin_inf();
    float f_inf = __builtin_inff();
    float f_neg_inf = -__builtin_inff();
    
    /* Initialize NaN arrays with different NaN creation methods */
    for (int i = 0; i < 6; i++) {
        d_nan[i] = make_nan_double(i % 5);
        f_nan[i] = make_nan_float(i % 5);
    }
    
    /* Test various combinations to trigger different paths */
    for (int op = 0; op < 3; op++) {
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                /* NaN vs NaN */
                checksum += test_nan_comparisons(op, d_nan[i], d_nan[j], 
                                                f_nan[i], f_nan[j]);
                
                /* NaN vs normal */
                checksum += test_nan_comparisons(op, d_nan[i], d_normal[j], 
                                                f_nan[i], f_normal[j]);
                
                /* Normal vs NaN */
                checksum += test_nan_comparisons(op, d_normal[i], d_nan[j], 
                                                f_normal[i], f_nan[j]);
                
                /* NaN vs infinity */
                checksum += test_nan_comparisons(op, d_nan[i], d_inf, 
                                                f_nan[i], f_inf);
                checksum += test_nan_comparisons(op, d_nan[i], d_neg_inf, 
                                                f_nan[i], f_neg_inf);
                
                /* Infinity vs NaN */
                checksum += test_nan_comparisons(op, d_inf, d_nan[j], 
                                                f_inf, f_nan[j]);
                
                /* Normal vs normal (control cases) */
                checksum += test_nan_comparisons(op, d_normal[i], d_normal[j], 
                                                f_normal[i], f_normal[j]);
                
                /* Infinity vs normal */
                checksum += test_nan_comparisons(op, d_inf, d_normal[j], 
                                                f_inf, f_normal[j]);
            }
        }
    }
    
    /* Additional tests with more complex expressions */
    for (int i = 0; i < 100; i++) {
        double a = (i % 2 == 0) ? make_nan_double(i % 5) : d_normal[i % 6];
        double b = (i % 3 == 0) ? make_nan_double((i + 1) % 5) : d_normal[(i + 1) % 6];
        float c = (i % 5 == 0) ? make_nan_float(i % 5) : f_normal[i % 6];
        float d = (i % 7 == 0) ? make_nan_float((i + 2) % 5) : f_normal[(i + 2) % 6];
        
        checksum += test_nan_comparisons(i % 3, a, b, c, d);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Side effects: %d\n", side_effect_counter);
    
    return 0;
}
