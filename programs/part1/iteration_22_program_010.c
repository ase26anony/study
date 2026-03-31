/* test_nan_comparisons.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int dummy_volatile;
static int __attribute__((noinline)) dummy_func(int x) {
    dummy_volatile = x;
    return x + 1;
}

/* Helper to create various NaN values */
static double make_nan_double(void) {
    return 0.0 / 0.0;
}

static float make_nan_float(void) {
    float zero = 0.0f;
    return zero / zero;
}

static double make_snan(void) {
    return __builtin_nan("0x1");
}

static double make_qnan(void) {
    return __builtin_nan("");
}

/* Non-inlineable function with complex NaN-aware comparisons */
static int __attribute__((noinline,noipa))
test_nan_comparisons(int opcode, double a, double b, float fa, float fb) {
    int result = 0;
    
    /* Side effects to prevent early optimization */
    int side_effect = dummy_func(opcode);
    
    /* Use volatile variables to force memory operations */
    volatile double va = a;
    volatile double vb = b;
    volatile float vfa = fa;
    volatile float vfb = fb;
    
    /* Complex switch-like structure for different comparison types */
    switch (opcode & 0x3) {
        case 0: { /* UNEQ_EXPR-like comparisons (unordered or equal) */
            /* Using __builtin_isunordered() and equality checks */
            int unord_eq_double = (__builtin_isunordered(va, vb) || va == vb);
            int unord_eq_float = (__builtin_isunordered(vfa, vfb) || vfa == vfb);
            
            /* Mix with isnan checks */
            int isnan_a = __builtin_isnan(va);
            int isnan_b = __builtin_isnan(vb);
            
            /* Complex expression with side effects */
            result = (unord_eq_double && !isnan_a) || 
                     (unord_eq_float && !isnan_b) ||
                     (side_effect > 0);
            
            /* More UNEQ-like expressions using macros */
            result |= (isunordered(va, vb) || (va == vb)) ? 1 : 0;
            result |= (isunordered(vfa, vfb) || (vfa == vfb)) ? 1 : 0;
            break;
        }
        
        case 1: { /* LTGT_EXPR-like comparisons (ordered and not equal) */
            /* Ordered and not equal: !(unordered) && (a != b) */
            int ltgt_double = (!__builtin_isunordered(va, vb) && va != vb);
            int ltgt_float = (!__builtin_isunordered(vfa, vfb) && vfa != vfb);
            
            /* Alternative using comparison macros */
            int ltgt_alt = (isgreater(va, vb) || isless(va, vb));
            
            /* Combine with isnan checks */
            int both_nan = __builtin_isnan(va) && __builtin_isnan(vb);
            
            result = (ltgt_double && !both_nan) ||
                     (ltgt_float && !both_nan) ||
                     (ltgt_alt && side_effect > 0);
            
            /* More LTGT-like expressions */
            result |= (!isunordered(va, vb) && (va != vb)) ? 2 : 0;
            result |= (!isunordered(vfa, vfb) && (vfa != vfb)) ? 2 : 0;
            break;
        }
        
        case 2: { /* Mixed comparisons to generate both tree codes */
            /* Generate UNEQ_EXPR */
            int unord_eq_mixed = (__builtin_isunordered(va, vfb) || va == vfb);
            
            /* Generate LTGT_EXPR */
            int ltgt_mixed = (!__builtin_isunordered(fa, vb) && fa != vb);
            
            /* Complex conditional with side effects */
            if (unord_eq_mixed && (side_effect % 2)) {
                result = 1;
            } else if (ltgt_mixed && (side_effect % 3)) {
                result = 2;
            } else {
                result = 3;
            }
            
            /* Additional comparisons using builtins */
            result += (__builtin_isgreater(va, vb) ? 1 : 0);
            result += (__builtin_isless(vfa, vfb) ? 1 : 0);
            result += (__builtin_isunordered(va + 1.0, vb - 1.0) ? 1 : 0);
            break;
        }
        
        default: {
            /* More complex expressions that might generate both codes */
            double temp1 = va * 2.0;
            float temp2 = vfb / 2.0f;
            
            /* UNEQ_EXPR pattern */
            int cond1 = (isunordered(temp1, vb) || temp1 == vb);
            
            /* LTGT_EXPR pattern */
            int cond2 = (!isunordered(va, temp2) && va != temp2);
            
            /* Nested conditional to force evaluation */
            result = cond1 ? (cond2 ? 4 : 5) : (cond2 ? 6 : 7);
            
            /* Use the side effect variable */
            result += side_effect;
            break;
        }
    }
    
    /* Final side effect to prevent dead code elimination */
    dummy_volatile = result;
    
    return result;
}

int main(void) {
    /* Initialize test values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double inf_pos = __builtin_inf();
    double inf_neg = -__builtin_inf();
    double nan_d = make_nan_double();
    double snan = make_snan();
    double qnan = make_qnan();
    
    float f_normal1 = 1.41421f;
    float f_normal2 = 1.73205f;
    float f_inf = __builtin_inff();
    float f_nan = make_nan_float();
    
    /* Array of test values */
    double d_vals[] = {normal1, normal2, inf_pos, inf_neg, nan_d, snan, qnan, 0.0, -0.0};
    float f_vals[] = {f_normal1, f_normal2, f_inf, f_nan, 0.0f, -0.0f};
    
    int d_count = sizeof(d_vals) / sizeof(d_vals[0]);
    int f_count = sizeof(f_vals) / sizeof(f_vals[0]);
    
    int checksum = 0;
    
    /* Test various combinations */
    for (int op = 0; op < 8; op++) {
        for (int i = 0; i < d_count; i++) {
            for (int j = 0; j < d_count; j++) {
                for (int k = 0; k < f_count; k++) {
                    for (int l = 0; l < f_count; l++) {
                        /* Call the comparison function with different opcodes */
                        int res = test_nan_comparisons(
                            op + i + j,  /* Varying opcode */
                            d_vals[i],   /* double a */
                            d_vals[j],   /* double b */
                            f_vals[k],   /* float fa */
                            f_vals[l]    /* float fb */
                        );
                        
                        /* Update checksum deterministically */
                        checksum = (checksum * 31 + res) & 0xFFFF;
                        
                        /* Occasionally mix in direct comparisons */
                        if ((i + j + k + l) % 7 == 0) {
                            /* Direct UNEQ-like expression */
                            if (__builtin_isunordered(d_vals[i], d_vals[j]) || 
                                d_vals[i] == d_vals[j]) {
                                checksum += 1;
                            }
                            
                            /* Direct LTGT-like expression */
                            if (!__builtin_isunordered(f_vals[k], f_vals[l]) && 
                                f_vals[k] != f_vals[l]) {
                                checksum += 2;
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* Print deterministic result */
    printf("Checksum: %d\n", checksum);
    
    /* Additional test cases focused on NaN comparisons */
    printf("Additional tests:\n");
    
    /* Explicit UNEQ_EXPR patterns */
    double test_nan = 0.0/0.0;
    double test_inf = 1.0/0.0;
    
    volatile int test1 = (__builtin_isunordered(test_nan, normal1) || test_nan == normal1);
    volatile int test2 = (isunordered(test_nan, test_inf) || test_nan == test_inf);
    volatile int test3 = (isunordered(f_nan, f_normal1) || f_nan == f_normal1);
    
    /* Explicit LTGT_EXPR patterns */
    volatile int test4 = (!__builtin_isunordered(normal1, normal2) && normal1 != normal2);
    volatile int test5 = (!isunordered(test_inf, inf_neg) && test_inf != inf_neg);
    volatile int test6 = (!isunordered(f_normal1, f_normal2) && f_normal1 != f_normal2);
    
    printf("Test results: %d %d %d %d %d %d\n", 
           test1, test2, test3, test4, test5, test6);
    
    return 0;
}
