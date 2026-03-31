/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    side_effect += (int)(a * 100.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins to generate UNEQ_EXPR */
            int cmp1 = __builtin_isunordered(a, b) || (a == b);
            int cmp2 = isunordered(c, d) || (c == d);
            result = cmp1 ^ cmp2;
            side_effect += result;
            break;
        }
        case 1: { /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins to generate LTGT_EXPR */
            int cmp1 = !__builtin_isunordered(a, b) && (a != b);
            int cmp2 = !isunordered(c, d) && (c != d);
            result = cmp1 | cmp2;
            side_effect += result;
            break;
        }
        case 2: { /* Mixed comparisons */
            /* Direct NaN checks */
            int cmp1 = __builtin_isnan(a) && __builtin_isgreater(b, 0.0);
            int cmp2 = isnan(c) && isless(d, 0.0f);
            result = cmp1 & cmp2;
            side_effect += result;
            break;
        }
        case 3: { /* Complex NaN-aware expressions */
            /* Using multiple comparison macros */
            int cmp1 = __builtin_islessequal(a, b) && !__builtin_isunordered(a, b);
            int cmp2 = isgreaterequal(c, d) && !isunordered(c, d);
            int cmp3 = __builtin_isnan(a + b) || __builtin_isnan(c * d);
            result = (cmp1 ^ cmp2) | cmp3;
            side_effect += result;
            break;
        }
        default: {
            /* More comparisons using different builtins */
            int cmp1 = __builtin_isless(a, b);
            int cmp2 = isgreater(c, d);
            result = cmp1 != cmp2;
            side_effect += result;
            break;
        }
    }
    
    /* Additional side effect to prevent optimization */
    volatile double dummy = a * b + c * d;
    (void)dummy;
    
    return result + side_effect;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += 1;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;                    /* Quiet NaN */
    double neg_nan_dbl = -__builtin_nan("");       /* Negative quiet NaN */
    double inf_dbl = __builtin_inf();              /* Positive infinity */
    double neg_inf_dbl = -__builtin_inf();         /* Negative infinity */
    double normal_dbl = 3.141592653589793;
    double zero_dbl = 0.0;
    
    float nan_flt = sqrtf(-1.0f);                  /* Signaling NaN */
    float inf_flt = __builtin_inff();              /* Positive infinity (float) */
    float neg_inf_flt = -__builtin_inff();         /* Negative infinity (float) */
    float normal_flt = 2.718281828459045f;
    float zero_flt = 0.0f;
    
    /* Array of test values */
    double dbl_values[] = {nan_dbl, neg_nan_dbl, inf_dbl, neg_inf_dbl, 
                          normal_dbl, zero_dbl, 1.0, -1.0};
    float flt_values[] = {nan_flt, inf_flt, neg_inf_flt, normal_flt, 
                         zero_flt, 1.0f, -1.0f, 0.5f};
    
    int checksum = 0;
    int counter = 0;
    
    /* Test various combinations */
    for (int op = 0; op < 5; op++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                /* Call with different value combinations */
                int res = test_nan_comparisons(
                    op, 
                    dbl_values[i], 
                    dbl_values[(i + j) % 8],
                    flt_values[j],
                    flt_values[(i + j) % 8]
                );
                
                checksum ^= res * (op + 1) * (i + 1) * (j + 1);
                
                /* Add dummy calls to prevent optimization */
                if ((i + j) % 3 == 0) {
                    dummy_call(&counter);
                }
            }
        }
    }
    
    /* Additional tests with computed NaN values */
    for (int i = 0; i < 100; i++) {
        /* Generate NaN through different computations */
        double a = (i % 2 == 0) ? __builtin_nan("123") : (0.0 * inf_dbl);
        float b = (i % 3 == 0) ? __builtin_nanf("456") : (zero_flt / zero_flt);
        
        /* Test UNEQ_EXPR-like comparisons */
        checksum += test_nan_comparisons(0, a, normal_dbl, b, normal_flt);
        
        /* Test LTGT_EXPR-like comparisons */
        checksum -= test_nan_comparisons(1, normal_dbl, a, normal_flt, b);
        
        /* More complex expressions */
        if (i % 5 == 0) {
            checksum ^= test_nan_comparisons(2, a, a, b, b);
        }
        
        dummy_call(&counter);
    }
    
    /* Use volatile to ensure all computations are performed */
    volatile int final_checksum = checksum + counter;
    
    printf("Result: %d (checksum: %d, calls: %d)\n", 
           final_checksum, checksum, counter);
    
    return final_checksum != 0 ? 0 : 1;
}
