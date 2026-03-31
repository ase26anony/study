/* test_nan_folding.c - Target fold-const.cc lines 6248-6258 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int global_counter = 0;
static int __attribute__((noinline)) dummy_side_effect(int x) {
    global_counter += x;
    return x;
}

/* Generate various NaN values */
static double get_nan_double(void) {
    return 0.0 / 0.0;  /* Produces quiet NaN */
}

static float get_nan_float(void) {
    float zero = 0.0f;
    return zero / zero;  /* Produces quiet NaN */
}

static double get_snan_double(void) {
    /* Signaling NaN via builtin */
    return __builtin_nans("");
}

static float get_snan_float(void) {
    /* Signaling NaN via builtin */
    return __builtin_nansf("");
}

/* Complex comparison function with side effects */
static int __attribute__((noinline))
test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Force evaluation with side effects */
    side_effect += dummy_side_effect(1);
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR - unordered or equal */
            /* Using GCC builtins that generate UNEQ_EXPR */
            if (__builtin_isunordered(d1, d2) || d1 == d2) {
                result |= 1;
                side_effect += dummy_side_effect(2);
            }
            /* Float version */
            if (isunordered(f1, f2) || f1 == f2) {
                result |= 2;
                side_effect += dummy_side_effect(3);
            }
            break;
            
        case 1: /* LTGT_EXPR - less than or greater than (ordered and not equal) */
            /* Using comparison macros that generate LTGT_EXPR */
            if (__builtin_islessgreater(d1, d2)) {
                result |= 4;
                side_effect += dummy_side_effect(4);
            }
            /* Alternative formulation */
            if ((d1 < d2 || d1 > d2) && !__builtin_isunordered(d1, d2)) {
                result |= 8;
                side_effect += dummy_side_effect(5);
            }
            /* Float version with isnan check */
            if ((isless(f1, f2) || isgreater(f1, f2)) && !isnan(f1 + f2)) {
                result |= 16;
                side_effect += dummy_side_effect(6);
            }
            break;
            
        case 2: /* Mixed comparisons to generate both tree codes */
            /* Generate UNEQ_EXPR */
            if (!__builtin_isnan(d1) && (__builtin_isunordered(d1, d2) || d1 == d2)) {
                result |= 32;
                side_effect += dummy_side_effect(7);
            }
            /* Generate LTGT_EXPR */
            if (__builtin_islessgreater(f1, f2) && !__builtin_isnan(f1)) {
                result |= 64;
                side_effect += dummy_side_effect(8);
            }
            break;
            
        default:
            /* Generate various comparison tree codes */
            if (__builtin_isgreater(d1, d2)) result |= 128;
            if (__builtin_isless(d1, d2)) result |= 256;
            if (__builtin_isunordered(f1, f2)) result |= 512;
            side_effect += dummy_side_effect(9);
            break;
    }
    
    /* Use side_effect to prevent dead code elimination */
    return result + side_effect;
}

int main(void) {
    /* Test values including NaNs, infinities, and normal numbers */
    double d_nan = get_nan_double();
    double d_snan = get_snan_double();
    double d_inf = __builtin_inf();
    double d_neg_inf = -__builtin_inf();
    double d_normal = 3.14159;
    double d_zero = 0.0;
    
    float f_nan = get_nan_float();
    float f_snan = get_snan_float();
    float f_inf = __builtin_inff();
    float f_neg_inf = -__builtin_inff();
    float f_normal = 2.71828f;
    float f_zero = 0.0f;
    
    /* Test cases designed to trigger specific fold-const transformations */
    struct test_case {
        int opcode;
        double d1, d2;
        float f1, f2;
        const char *desc;
    } test_cases[] = {
        /* Cases that should generate UNEQ_EXPR */
        {0, d_nan, d_nan, f_nan, f_nan, "NaN vs NaN (UNEQ)"},
        {0, d_nan, d_normal, f_nan, f_normal, "NaN vs normal (UNEQ)"},
        {0, d_normal, d_nan, f_normal, f_nan, "normal vs NaN (UNEQ)"},
        {0, d_inf, d_inf, f_inf, f_inf, "inf vs inf (UNEQ)"},
        {0, d_zero, d_zero, f_zero, f_zero, "zero vs zero (UNEQ)"},
        
        /* Cases that should generate LTGT_EXPR */
        {1, d_normal, d_inf, f_normal, f_inf, "normal vs inf (LTGT)"},
        {1, d_neg_inf, d_normal, f_neg_inf, f_normal, "-inf vs normal (LTGT)"},
        {1, d_inf, d_neg_inf, f_inf, f_neg_inf, "inf vs -inf (LTGT)"},
        {1, 1.0, 2.0, 1.0f, 2.0f, "1.0 vs 2.0 (LTGT)"},
        
        /* Mixed cases */
        {2, d_snan, d_normal, f_snan, f_normal, "sNaN vs normal (mixed)"},
        {2, d_normal, d_snan, f_normal, f_snan, "normal vs sNaN (mixed)"},
        {2, d_inf, d_nan, f_inf, f_nan, "inf vs NaN (mixed)"},
        
        /* Edge cases */
        {3, d_nan, d_snan, f_nan, f_snan, "NaN vs sNaN (other)"},
        {3, d_inf, d_zero, f_inf, f_zero, "inf vs zero (other)"},
    };
    
    int checksum = 0;
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Execute all test cases multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        for (int i = 0; i < num_cases; i++) {
            struct test_case tc = test_cases[i];
            
            /* Call comparison function - compiler cannot optimize away due to side effects */
            int result = test_nan_comparisons(tc.opcode, tc.d1, tc.d2, tc.f1, tc.f2);
            
            /* Update checksum deterministically */
            checksum = (checksum * 31 + result) ^ (i + iteration * 100);
            
            /* Force evaluation with printf to prevent optimization */
            if (iteration == 0 && i == 0) {
                printf("Starting NaN comparison tests...\n");
            }
        }
    }
    
    /* Print final checksum to ensure all code executes */
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
