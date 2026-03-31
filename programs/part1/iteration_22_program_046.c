/* test_nan_comparisons.c - Targeting fold-const.cc lines 6248-6258 */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int dummy_volatile = 0;
static int global_counter = 0;

/* Non-inlineable dummy function with side effects */
static void __attribute__((noinline)) 
dummy_side_effect(int x) {
    dummy_volatile = x;
}

/* Helper to create NaN values using different methods */
static double make_nan_double(int method) {
    switch (method) {
        case 0: return 0.0 / 0.0;          /* Division by zero NaN */
        case 1: return __builtin_nan("");  /* Quiet NaN */
        case 2: return -__builtin_nan(""); /* Negative NaN */
        default: return __builtin_nan("0xdead");
    }
}

static float make_nan_float(int method) {
    switch (method) {
        case 0: return 0.0f / 0.0f;
        case 1: return __builtin_nanf("");
        case 2: return -__builtin_nanf("");
        default: return __builtin_nanf("0xbeef");
    }
}

/* Complex comparison function that generates UNEQ_EXPR and LTGT_EXPR */
static int __attribute__((noinline))
test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    int result = 0;
    
    /* Mix volatile operations to prevent optimization */
    volatile double vd1 = d1;
    volatile float vf1 = f1;
    
    /* Complex switch structure to generate different comparison tree codes */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like comparisons (unordered or equal) */
            /* Using __builtin_isunordered() and equality checks */
            if (__builtin_isunordered(vd1, d2) || vd1 == d2) {
                dummy_side_effect(1);
                result |= 1;
            }
            
            /* Using isunordered macro */
            if (isunordered(f1, vf1) || f1 == vf1) {
                dummy_side_effect(2);
                result |= 2;
            }
            
            /* Direct NaN checks with equality */
            if (__builtin_isnan(vd1) || __builtin_isnan(d2) || vd1 == d2) {
                dummy_side_effect(3);
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like comparisons (ordered and not equal) */
            /* Using ordered comparisons: less than OR greater than */
            if ((__builtin_isless(vd1, d2) || __builtin_isgreater(vd1, d2)) 
                && !__builtin_isunordered(vd1, d2)) {
                dummy_side_effect(4);
                result |= 8;
            }
            
            /* Using isless/isgreater macros */
            if ((isless(f1, vf1) || isgreater(f1, vf1)) 
                && !isunordered(f1, vf1)) {
                dummy_side_effect(5);
                result |= 16;
            }
            
            /* Alternative formulation */
            if (!__builtin_isunordered(vd1, d2) && vd1 != d2) {
                dummy_side_effect(6);
                result |= 32;
            }
            break;
            
        case 2: /* Mixed comparisons to generate both tree codes */
            /* First UNEQ-like */
            if (isunordered(vd1, d2) || vd1 == d2) {
                dummy_side_effect(7);
                result |= 64;
            }
            
            /* Then LTGT-like */
            if ((isless(vd1, d2) || isgreater(vd1, d2)) 
                && !isunordered(vd1, d2)) {
                dummy_side_effect(8);
                result |= 128;
            }
            
            /* Float version */
            if ((__builtin_isless(f1, vf1) || __builtin_isgreater(f1, vf1)) 
                && !__builtin_isunordered(f1, vf1)) {
                dummy_side_effect(9);
                result |= 256;
            }
            break;
            
        default:
            /* Fallback with all comparison types */
            int r1 = isunordered(vd1, d2) || vd1 == d2;
            int r2 = (isless(vd1, d2) || isgreater(vd1, d2)) 
                     && !isunordered(vd1, d2);
            int r3 = __builtin_isnan(vd1) || __builtin_isnan(d2) || vd1 == d2;
            
            dummy_side_effect(r1 + r2 + r3);
            result = r1 | (r2 << 1) | (r3 << 2);
            break;
    }
    
    return result;
}

int main(void) {
    /* Test values including normals, infinities, and NaNs */
    double test_doubles[] = {
        1.0,
        -1.0,
        0.0,
        -0.0,
        __builtin_inf(),
        -__builtin_inf(),
        make_nan_double(0),
        make_nan_double(1),
        3.141592653589793,
        2.718281828459045
    };
    
    float test_floats[] = {
        1.0f,
        -1.0f,
        0.0f,
        -0.0f,
        __builtin_inff(),
        -__builtin_inff(),
        make_nan_float(0),
        make_nan_float(1),
        3.14159265f,
        2.71828182f
    };
    
    int checksum = 0;
    int num_doubles = sizeof(test_doubles) / sizeof(test_doubles[0]);
    int num_floats = sizeof(test_floats) / sizeof(test_floats[0]);
    
    /* Loop through different combinations to generate various comparisons */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_doubles; i++) {
            for (int j = 0; j < num_doubles; j++) {
                /* Skip some combinations to keep runtime reasonable */
                if ((i + j) % 3 != 0) continue;
                
                for (int k = 0; k < num_floats; k++) {
                    /* Mix float and double indices */
                    int l = (k + 1) % num_floats;
                    
                    int result = test_nan_comparisons(
                        op, 
                        test_doubles[i], 
                        test_doubles[j],
                        test_floats[k],
                        test_floats[l]
                    );
                    
                    /* Update checksum deterministically */
                    checksum = (checksum * 31 + result) & 0xFFFF;
                    global_counter += (result != 0);
                }
            }
        }
    }
    
    /* Additional targeted tests */
    printf("Targeted NaN comparison tests:\n");
    
    /* Direct UNEQ-like pattern */
    double nan1 = make_nan_double(0);
    double inf1 = __builtin_inf();
    
    if (__builtin_isunordered(nan1, inf1) || nan1 == inf1) {
        checksum += 1000;
        dummy_side_effect(100);
    }
    
    /* Direct LTGT-like pattern */
    double val1 = 42.0;
    double val2 = 17.0;
    
    if ((__builtin_isless(val1, val2) || __builtin_isgreater(val1, val2))
        && !__builtin_isunordered(val1, val2)) {
        checksum += 2000;
        dummy_side_effect(200);
    }
    
    /* Mixed NaN/normal comparisons */
    float nanf = make_nan_float(1);
    float normalf = 7.0f;
    
    volatile float vnanf = nanf;
    if (isunordered(vnanf, normalf) || vnanf == normalf) {
        checksum += 3000;
    }
    
    if ((isless(vnanf, normalf) || isgreater(vnanf, normalf))
        && !isunordered(vnanf, normalf)) {
        checksum += 4000;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Dummy volatile: %d\n", dummy_volatile);
    
    return 0;
}
