/* test_nan_comparisons.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    switch (opcode % 4) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins that may generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Another variant using macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            sink += result;
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins that may generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 4;
            }
            /* Another variant */
            if (!isunordered(c, d) && c != d) {
                result |= 8;
            }
            sink += result * 2;
            break;
            
        case 2: /* Mixed comparisons */
            /* Complex expression that might fold to UNEQ_EXPR */
            if ((__builtin_isnan(a) || __builtin_isnan(b)) ? 
                (__builtin_isnan(a) && __builtin_isnan(b)) : (a == b)) {
                result |= 16;
            }
            sink += result / 2;
            break;
            
        case 3: /* More mixed comparisons */
            /* Expression that might fold to LTGT_EXPR */
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 32;
            }
            /* Use comparison macros */
            if (isgreater(c, d) || isless(c, d)) {
                result |= 64;
            }
            sink += result % 7;
            break;
    }
    
    /* Additional side effect to prevent optimization */
    static volatile int counter = 0;
    counter += sink;
    
    return result;
}

/* Dummy function with side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int x) {
    volatile static int dummy = 0;
    dummy += x;
}

int main(void) {
    /* Create various floating-point values */
    double nan_dbl = 0.0 / 0.0;          /* Generate NaN */
    double inf_dbl = __builtin_inf();    /* Infinity */
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Alternative NaN generation methods */
    double nan_dbl2 = __builtin_nan("");
    double nan_dbl3 = sqrt(-1.0);
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    int checksum = 0;
    int iteration = 0;
    
    /* Test different combinations */
    double test_values_dbl[] = {
        nan_dbl, inf_dbl, neg_inf_dbl, normal_dbl, zero_dbl,
        nan_dbl2, nan_dbl3, 1.0, -1.0, 100.0
    };
    
    float test_values_flt[] = {
        nan_flt, inf_flt, normal_flt, zero_flt,
        0.0f, -0.0f, 42.0f, -42.0f
    };
    
    /* Loop through many combinations */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 8; k++) {
                for (int l = 0; l < 8; l++) {
                    for (int op = 0; op < 8; op++) {
                        /* Call with different operation codes and values */
                        int res = test_nan_comparisons(
                            op + iteration,
                            test_values_dbl[i],
                            test_values_dbl[j],
                            test_values_flt[k],
                            test_values_flt[l]
                        );
                        
                        checksum ^= res * (i + j + k + l + op);
                        checksum += iteration;
                        
                        /* Add side effects */
                        dummy_side_effect(res);
                        
                        iteration++;
                        if (iteration > 50) iteration = 0;
                    }
                }
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    for (int i = 0; i < 100; i++) {
        /* Direct use of comparison macros in complex expressions */
        volatile double x = (i % 2) ? nan_dbl : normal_dbl;
        volatile double y = (i % 3) ? nan_dbl2 : normal_dbl;
        
        /* Expression that should generate UNEQ_EXPR */
        int r1 = (__builtin_isunordered(x, y) || x == y) ? 1 : 0;
        
        /* Expression that should generate LTGT_EXPR */
        int r2 = (!__builtin_isunordered(x, y) && x != y) ? 2 : 0;
        
        /* Use results to prevent dead code elimination */
        checksum += r1 + r2;
        
        /* Test with floats using standard macros */
        volatile float f1 = (i % 5) ? nan_flt : normal_flt;
        volatile float f2 = (i % 7) ? nan_flt : inf_flt;
        
        int r3 = (isunordered(f1, f2) || f1 == f2) ? 4 : 0;
        int r4 = (!isunordered(f1, f2) && f1 != f2) ? 8 : 0;
        
        checksum ^= r3 | r4;
        
        /* Call helper function with specific patterns */
        test_nan_comparisons(0, x, y, f1, f2);  /* UNEQ_EXPR focus */
        test_nan_comparisons(1, x, y, f1, f2);  /* LTGT_EXPR focus */
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
