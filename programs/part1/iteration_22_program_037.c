/* test_nan_folding.c - Target specific uncovered lines in fold-const.cc */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int dummy_sink;
static void use_result(int x) { dummy_sink = x; }

/* Non-inlineable helper to force side effects */
static __attribute__((noinline)) 
int dummy_function(int x) {
    volatile int y = x;
    return y + 1;
}

/* Generate various floating-point values */
static double get_nan_double(void) {
    return 0.0 / 0.0;  /* Produces NaN */
}

static float get_nan_float(void) {
    float zero = 0.0f;
    return zero / zero;  /* Produces NaN */
}

static double get_inf_double(void) {
    return __builtin_inf();
}

static float get_inf_float(void) {
    return __builtin_inff();
}

/* Complex comparison function with side effects */
static __attribute__((noinline))
int test_nan_comparisons(int opcode, double d1, double d2, float f1, float f2) {
    int result = 0;
    
    /* Force side effects before comparisons */
    int side_effect = dummy_function(opcode);
    
    switch (opcode) {
        case 0: { /* UNEQ_EXPR (unordered or equal) */
            /* Using __builtin_isunordered() and == */
            int unordered = __builtin_isunordered(d1, d2);
            int equal = (d1 == d2);
            int uneq_result = unordered || equal;
            
            /* Also test with floats */
            int funordered = __builtin_isunordered(f1, f2);
            int fequal = (f1 == f2);
            int funeq_result = funordered || fequal;
            
            result = uneq_result + funeq_result + side_effect;
            break;
        }
        
        case 1: { /* LTGT_EXPR (ordered and not equal) */
            /* Using !__builtin_isunordered() and != */
            int ordered = !__builtin_isunordered(d1, d2);
            int not_equal = (d1 != d2);
            int ltgt_result = ordered && not_equal;
            
            /* Also test with floats */
            int fordered = !__builtin_isunordered(f1, f2);
            int fnot_equal = (f1 != f2);
            int fltgt_result = fordered && fnot_equal;
            
            result = ltgt_result + fltgt_result + side_effect;
            break;
        }
        
        case 2: { /* Mixed comparisons to generate various tree codes */
            /* Use GCC built-in comparison macros */
            int isgreater = __builtin_isgreater(d1, d2);
            int isless = __builtin_isless(f1, f2);
            int isunordered = __builtin_isunordered(d1, f2);
            
            result = isgreater + isless + isunordered + side_effect;
            break;
        }
        
        case 3: { /* Direct NaN checks */
            int isnan_d1 = __builtin_isnan(d1);
            int isnan_f1 = __builtin_isnan(f1);
            int isnan_d2 = __builtin_isnan(d2);
            int isnan_f2 = __builtin_isnan(f2);
            
            /* Complex expression that might fold to UNEQ/LTGT */
            int complex_comp = (isnan_d1 || isnan_f1) && 
                              (!isnan_d2 && !isnan_f2) &&
                              (d1 != d2) && (f1 != f2);
            
            result = complex_comp + side_effect;
            break;
        }
        
        default: {
            /* Default case with all comparison types */
            int cmp1 = (d1 < d2);
            int cmp2 = (f1 > f2);
            int cmp3 = (d1 == d2);
            int cmp4 = (f1 != f2);
            int unordered_check = __builtin_isunordered(d1, d2);
            
            result = cmp1 + cmp2 + cmp3 + cmp4 + unordered_check + side_effect;
            break;
        }
    }
    
    /* Use result to prevent dead code elimination */
    use_result(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Generate test values */
    double nan_d = get_nan_double();
    float nan_f = get_nan_float();
    double inf_d = get_inf_double();
    float inf_f = get_inf_float();
    double normal_d = 3.14159;
    float normal_f = 2.71828f;
    double zero_d = 0.0;
    float zero_f = 0.0f;
    double neg_d = -1.5;
    float neg_f = -2.5f;
    
    /* Test different combinations */
    double test_values_d[] = {nan_d, inf_d, normal_d, zero_d, neg_d};
    float test_values_f[] = {nan_f, inf_f, normal_f, zero_f, neg_f};
    
    int num_d_vals = sizeof(test_values_d) / sizeof(test_values_d[0]);
    int num_f_vals = sizeof(test_values_f) / sizeof(test_values_f[0]);
    
    /* Loop through different opcodes and value combinations */
    for (int op = 0; op < 5; op++) {
        for (int i = 0; i < num_d_vals; i++) {
            for (int j = 0; j < num_f_vals; j++) {
                /* Mix different types to create conversion nodes */
                double d1 = test_values_d[i];
                double d2 = test_values_d[(i + 1) % num_d_vals];
                float f1 = test_values_f[j];
                float f2 = test_values_f[(j + 1) % num_f_vals];
                
                /* Call comparison function */
                int result = test_nan_comparisons(op, d1, d2, f1, f2);
                
                /* Update checksum deterministically */
                checksum = (checksum * 31 + result) & 0xFFFF;
                
                /* Additional direct comparisons in main to increase coverage */
                if (op == 0) {
                    /* UNEQ-like expression */
                    volatile int uneq_test = __builtin_isunordered(d1, f2) || (d1 == f2);
                    checksum += uneq_test;
                } else if (op == 1) {
                    /* LTGT-like expression */
                    volatile int ltgt_test = !__builtin_isunordered(f1, d2) && (f1 != d2);
                    checksum += ltgt_test;
                }
            }
        }
    }
    
    /* Test with __builtin_nan directly */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    for (int i = 0; i < 10; i++) {
        int r1 = test_nan_comparisons(i % 4, builtin_nan, normal_d, builtin_nanf, normal_f);
        int r2 = test_nan_comparisons((i + 1) % 4, inf_d, builtin_nan, inf_f, builtin_nanf);
        checksum = (checksum * 17 + r1 + r2) & 0xFFFF;
    }
    
    /* Print deterministic result */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
