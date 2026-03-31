/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Non-inlineable helper functions to prevent optimization */
static volatile int side_effect_counter = 0;
static void __attribute__((noinline)) dummy_side_effect(int x) {
    side_effect_counter += x;
}

/* Helper to create NaN values in different ways */
static double __attribute__((noinline)) get_nan_double(int method) {
    switch (method) {
        case 0: return 0.0 / 0.0;           /* Division by zero */
        case 1: return __builtin_nan("");   /* Quiet NaN */
        case 2: return sqrt(-1.0);          /* sqrt of negative */
        case 3: return __builtin_nanf("");  /* Float NaN promoted */
        default: return __builtin_inf() * 0.0; /* Infinity times zero */
    }
}

static float __attribute__((noinline)) get_nan_float(int method) {
    switch (method) {
        case 0: return 0.0f / 0.0f;
        case 1: return __builtin_nanf("");
        case 2: return sqrtf(-1.0f);
        case 3: return (float)__builtin_nan("");
        default: return __builtin_inff() * 0.0f;
    }
}

/* Complex comparison function with side effects */
static int __attribute__((noinline))
test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int result = 0;
    volatile double vd1, vd2;
    volatile float vf1, vf2;
    
    /* Store to volatile variables to prevent optimization */
    vd1 = a;
    vd2 = b;
    vf1 = c;
    vf2 = d;
    
    /* Complex switch-like structure with side effects */
    switch (opcode & 7) {
        case 0: /* UNEQ_EXPR-like comparisons using builtins */
            if (__builtin_isunordered(vd1, vd2) || vd1 == vd2) {
                dummy_side_effect(1);
                result |= 1;
            }
            /* Float version */
            if (isunordered(vf1, vf2) || vf1 == vf2) {
                dummy_side_effect(2);
                result |= 2;
            }
            /* Mixed float/double with isnan check */
            if (__builtin_isnan(vd1) || __builtin_isnan((double)vf1)) {
                dummy_side_effect(4);
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like comparisons */
            if (!__builtin_isunordered(vd1, vd2) && vd1 != vd2) {
                dummy_side_effect(8);
                result |= 8;
            }
            if (!isunordered(vf1, vf2) && vf1 != vf2) {
                dummy_side_effect(16);
                result |= 16;
            }
            /* Check ordered comparison with isnan */
            if (!__builtin_isnan(vd1) && !__builtin_isnan(vd2) && 
                (__builtin_isgreater(vd1, vd2) || __builtin_isless(vd1, vd2))) {
                dummy_side_effect(32);
                result |= 32;
            }
            break;
            
        case 2: /* Mixed comparisons that might generate both tree codes */
            /* First an UNEQ-like expression */
            if ((isunordered(vf1, vf2) || vf1 == vf2) && 
                !__builtin_isnan((double)vf1)) {
                dummy_side_effect(64);
                result |= 64;
            }
            /* Then an LTGT-like expression */
            if ((!isunordered(vf1, vf2) && vf1 != vf2) ||
                __builtin_isnan((double)vf2)) {
                dummy_side_effect(128);
                result |= 128;
            }
            break;
            
        case 3: /* Complex expression combining both */
            result = (__builtin_isunordered(vd1, vd2) || vd1 == vd2) ? 
                     ((!__builtin_isunordered(vd1, vd2) && vd1 != vd2) ? 256 : 512) : 1024;
            dummy_side_effect(result);
            break;
            
        case 4: /* Using comparison macros directly */
            if (isgreater(vd1, vd2) || isless(vd1, vd2)) {
                if (!isunordered(vd1, vd2)) {
                    dummy_side_effect(2048);
                    result |= 2048;
                }
            }
            if (isunordered(vf1, vf2) && __builtin_isnanf(vf1)) {
                dummy_side_effect(4096);
                result |= 4096;
            }
            break;
            
        default:
            /* More complex nested comparisons */
            int temp = 0;
            if ((__builtin_isnan(vd1) || vd1 == vd2) && 
                !__builtin_isunordered(vd1, vd2)) {
                temp = 8192;
            }
            if ((!__builtin_isnan(vd2) && vd1 != vd2) ||
                __builtin_isunordered(vd1, vd2)) {
                temp |= 16384;
            }
            result = temp;
            dummy_side_effect(temp);
            break;
    }
    
    /* Additional side effect to ensure expression isn't optimized away */
    volatile int final_check = (result > 0) ? 1 : 0;
    dummy_side_effect(final_check);
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Test values including normals, infinities, and NaNs */
    double test_doubles[] = {
        1.0, -1.0, 0.0, -0.0,
        __builtin_inf(), -__builtin_inf(),
        get_nan_double(0), get_nan_double(1),
        3.14159, -2.71828
    };
    
    float test_floats[] = {
        2.0f, -2.0f, 0.0f, -0.0f,
        __builtin_inff(), -__builtin_inff(),
        get_nan_float(0), get_nan_float(1),
        1.5f, -2.5f
    };
    
    int num_doubles = sizeof(test_doubles) / sizeof(test_doubles[0]);
    int num_floats = sizeof(test_floats) / sizeof(test_floats[0]);
    
    /* Test many combinations to increase coverage */
    for (int i = 0; i < num_doubles; i++) {
        for (int j = 0; j < num_floats; j++) {
            for (int op = 0; op < 8; op++) {
                /* Mix indices to create various NaN/normal combinations */
                int idx1 = (i + j) % num_doubles;
                int idx2 = (i * j) % num_floats;
                int idx3 = (i + op) % num_floats;
                int idx4 = (j + op) % num_doubles;
                
                checksum += test_nan_comparisons(
                    op,
                    test_doubles[idx1],
                    test_doubles[idx4],
                    test_floats[idx2],
                    test_floats[idx3]
                );
                
                /* Also test with same values */
                checksum += test_nan_comparisons(
                    op | 0x10,
                    test_doubles[i],
                    test_doubles[i],  /* Same value */
                    test_floats[j],
                    test_floats[j]    /* Same value */
                );
            }
        }
    }
    
    /* Additional specific tests for edge cases */
    checksum += test_nan_comparisons(0, 
        __builtin_nan(""), __builtin_nan(""), 
        __builtin_nanf(""), __builtin_nanf(""));
    
    checksum += test_nan_comparisons(1,
        0.0/0.0, sqrt(-1.0),
        0.0f/0.0f, sqrtf(-1.0f));
    
    checksum += test_nan_comparisons(2,
        __builtin_inf(), -__builtin_inf(),
        __builtin_inff(), -__builtin_inff());
    
    printf("Final checksum: %d\n", checksum);
    printf("Side effects triggered: %d\n", side_effect_counter);
    
    return 0;
}
