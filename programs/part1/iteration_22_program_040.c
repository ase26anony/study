#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Non-inlineable function to prevent early optimization */
static void __attribute__((noinline)) dummy_side_effect(int x) {
    static volatile int sink;
    sink = x;
}

/* Helper to create NaN values using different methods */
static double make_nan_double(int method) {
    switch (method) {
        case 0: return 0.0 / 0.0;                    /* Division by zero */
        case 1: return __builtin_nan("");           /* Quiet NaN */
        case 2: return -__builtin_nan("0x1");       /* Different payload */
        case 3: return __builtin_sqrt(-1.0);        /* sqrt(-1) */
        default: return __builtin_nanf("");         /* Float NaN promoted */
    }
}

static float make_nan_float(int method) {
    switch (method) {
        case 0: return 0.0f / 0.0f;
        case 1: return __builtin_nanf("");
        case 2: return -__builtin_nanf("0x2");
        case 3: return __builtin_sqrtf(-1.0f);
        default: return __builtin_nanf("");
    }
}

/* Non-inlineable function performing NaN-aware comparisons */
static int __attribute__((noinline))
test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    volatile int result = 0;
    
    /* Complex expression with side effects to prevent optimization */
    switch (opcode) {
        case 0: /* UNEQ_EXPR (unordered or equal) */
            dummy_side_effect(__LINE__);
            if (__builtin_isunordered(d1, d2) || d1 == d2) {
                result |= 1;
            }
            if (isunordered(f1, f2) || f1 == f2) {
                result |= 2;
            }
            /* Additional UNEQ-like expressions */
            if (!__builtin_isgreater(d1, d2) && !__builtin_isless(d1, d2)) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR (ordered and not equal) */
            dummy_side_effect(__LINE__);
            if (!__builtin_isunordered(d1, d2) && d1 != d2) {
                result |= 1;
            }
            if (!isunordered(f1, f2) && f1 != f2) {
                result |= 2;
            }
            /* Additional LTGT-like expressions */
            if (__builtin_isgreater(d1, d2) || __builtin_isless(d1, d2)) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons */
            dummy_side_effect(__LINE__);
            /* Generate both UNEQ and LTGT in same basic block */
            if ((__builtin_isunordered(d1, d2) || d1 == d2) &&
                (!__builtin_isunordered(f1, f2) && f1 != f2)) {
                result |= 8;
            }
            /* Direct isnan checks */
            if (__builtin_isnan(d1) && !__builtin_isnan(d2)) {
                result |= 16;
            }
            break;
            
        default:
            dummy_side_effect(__LINE__);
            /* More complex nested comparisons */
            int temp = (isgreater(d1, d2) ? 1 : 0) +
                      (isless(d1, d2) ? 2 : 0) +
                      (isunordered(d1, d2) ? 4 : 0);
            result = temp;
            break;
    }
    
    return result;
}

int main(void) {
    /* Test values including normals, infinities, and NaNs */
    double d_vals[] = {
        1.0, -1.0, 0.0, -0.0,
        __builtin_inf(), -__builtin_inf(),
        make_nan_double(0), make_nan_double(1),
        make_nan_double(2), make_nan_double(3)
    };
    
    float f_vals[] = {
        1.0f, -1.0f, 0.0f, -0.0f,
        __builtin_inff(), -__builtin_inff(),
        make_nan_float(0), make_nan_float(1),
        make_nan_float(2), make_nan_float(3)
    };
    
    int checksum = 0;
    int iteration = 0;
    
    /* Test various combinations to generate different tree codes */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int op = 0; op < 3; op++) {
                /* Mix float and double comparisons */
                int res1 = test_nan_comparisons(op, 
                    d_vals[i], d_vals[j],
                    f_vals[i], f_vals[j]);
                
                /* Test with swapped arguments */
                int res2 = test_nan_comparisons(op,
                    d_vals[j], d_vals[i],
                    f_vals[j], f_vals[i]);
                
                /* Test with same values */
                int res3 = test_nan_comparisons(op,
                    d_vals[i], d_vals[i],
                    f_vals[j], f_vals[j]);
                
                checksum += res1 + res2 + res3 + iteration;
                iteration++;
                
                /* Force evaluation with volatile */
                volatile int dummy = res1 | res2 | res3;
                (void)dummy;
            }
        }
    }
    
    /* Also test special NaN propagation cases */
    double nan1 = make_nan_double(0);
    double nan2 = make_nan_double(1);
    float nanf1 = make_nan_float(0);
    float nanf2 = make_nan_float(1);
    
    /* These should generate UNEQ_EXPR and LTGT_EXPR nodes */
    checksum += test_nan_comparisons(0, nan1, nan2, nanf1, nanf2);
    checksum += test_nan_comparisons(1, nan1, 1.0, nanf1, 1.0f);
    checksum += test_nan_comparisons(0, 1.0, nan1, 1.0f, nanf1);
    checksum += test_nan_comparisons(1, nan1, nan1, nanf1, nanf1);
    
    /* Mixed ordered/unordered comparisons */
    if (!__builtin_isunordered(nan1, 1.0) && nan1 != 1.0) {
        checksum += 1000;  /* LTGT_EXPR with NaN and normal */
    }
    
    if (__builtin_isunordered(nan1, nan2) || nan1 == nan2) {
        checksum += 2000;  /* UNEQ_EXPR with two NaNs */
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
