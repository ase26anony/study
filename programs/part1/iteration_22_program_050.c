/* test_nan_comparisons.c - Targeting GCC fold-const.cc lines 6248-6258 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int dummy_side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter;
}

/* Non-inlineable helper with complex NaN-aware comparisons */
static __attribute__((noinline, optimize("no-tree-pre")))
float test_nan_comparisons(float a, float b, double c, double d, int op) {
    volatile float vf_result = 0.0f;
    volatile double vd_result = 0.0;
    int side_effect = 0;
    
    /* Complex switch-like structure to generate different comparison tree codes */
    switch (op & 7) {
        case 0: /* UNEQ_EXPR generation: unordered or equal */
            /* Using __builtin_isunordered and equality check */
            if (__builtin_isunordered(a, b) || a == b) {
                vf_result = a + b;
                side_effect += 1;
            }
            /* Mixed float/double comparisons */
            if (isunordered((double)a, d) || (double)a == d) {
                vd_result = c * d;
                side_effect += 2;
            }
            /* Direct __builtin_isnan usage */
            if (__builtin_isnan(c) || __builtin_isnan(d) || c == d) {
                vf_result += (float)c;
                side_effect += 4;
            }
            break;
            
        case 1: /* LTGT_EXPR generation: ordered and not equal */
            /* Using ordered comparison macros */
            if (!__builtin_isunordered(a, b) && a != b) {
                vf_result = a - b;
                side_effect += 8;
            }
            if (!isunordered(c, d) && c != d) {
                vd_result = c / (d + 1.0);
                side_effect += 16;
            }
            /* Complex expression with multiple comparisons */
            if ((isgreater(c, d) || isless(c, d)) && !__builtin_isnan(c) && !__builtin_isnan(d)) {
                vf_result *= 2.0f;
                side_effect += 32;
            }
            break;
            
        case 2: /* Mixed comparisons */
            if (__builtin_isgreater(a, b) && !__builtin_isunordered(a, b)) {
                vf_result = b;
                side_effect += 64;
            }
            break;
            
        case 3: /* More UNEQ variants */
            if ((__builtin_isnan(a) || a == b) && !__builtin_signbit(a)) {
                vd_result = c;
                side_effect += 128;
            }
            break;
            
        case 4: /* More LTGT variants */
            if (!isunordered((float)c, (float)d) && (float)c != (float)d) {
                vf_result = (float)c + (float)d;
                side_effect += 256;
            }
            break;
            
        default:
            /* Default case with comparison macros */
            if (islessgreater(a, b)) {
                vf_result = a * b;
                side_effect += 512;
            }
            if (__builtin_isunordered(c, c)) { /* Always false for non-NaN */
                vd_result = -d;
                side_effect += 1024;
            }
            break;
    }
    
    /* Ensure side effects aren't optimized away */
    dummy_side_effect(side_effect);
    
    /* Return value based on comparisons */
    return vf_result + (float)vd_result;
}

int main(void) {
    /* Initialize test values including NaNs */
    float f_nan = 0.0f / 0.0f;
    float f_inf = __builtin_inff();
    float f_neg_inf = -__builtin_inff();
    float f_normal = 3.14159f;
    float f_zero = 0.0f;
    
    double d_nan = __builtin_nan("");
    double d_snan = __builtin_nans("");
    double d_inf = __builtin_inf();
    double d_normal = 2.718281828459045;
    double d_neg = -1.0;
    
    int checksum = 0;
    
    /* Test various combinations to hit different code paths */
    for (int i = 0; i < 32; i++) {
        float result;
        
        /* Different argument combinations */
        switch (i % 8) {
            case 0:
                result = test_nan_comparisons(f_nan, f_normal, d_nan, d_normal, i);
                break;
            case 1:
                result = test_nan_comparisons(f_normal, f_nan, d_normal, d_snan, i);
                break;
            case 2:
                result = test_nan_comparisons(f_inf, f_neg_inf, d_inf, d_neg, i);
                break;
            case 3:
                result = test_nan_comparisons(f_zero, f_zero, 0.0, 0.0, i);
                break;
            case 4:
                result = test_nan_comparisons(f_nan, f_nan, d_nan, d_nan, i);
                break;
            case 5:
                result = test_nan_comparisons(f_normal, f_zero, d_normal, 0.0, i);
                break;
            case 6:
                result = test_nan_comparisons(f_inf, f_normal, d_inf, d_normal, i);
                break;
            case 7:
                result = test_nan_comparisons(f_neg_inf, f_inf, -d_inf, d_inf, i);
                break;
        }
        
        /* Compute deterministic checksum */
        checksum += (int)(result * 1000.0f) + i;
        checksum ^= (int)(__builtin_fabs(result) * 100.0f);
    }
    
    /* Additional tests with sqrt(-1) NaN generation */
    float f_sqrt_nan = __builtin_sqrtf(-1.0f);
    double d_sqrt_nan = __builtin_sqrt(-1.0);
    
    for (int i = 0; i < 16; i++) {
        float r1 = test_nan_comparisons(f_sqrt_nan, f_normal, d_sqrt_nan, d_normal, i + 100);
        float r2 = test_nan_comparisons(f_normal, f_sqrt_nan, d_normal, d_sqrt_nan, i + 200);
        
        checksum += (int)(r1 * 100.0f) + (int)(r2 * 100.0f);
        checksum ^= i * 37;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
