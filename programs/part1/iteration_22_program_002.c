/* test_nan_comparisons.c - Target fold-const.cc lines 6248-6258 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int dummy_sink;
static void __attribute__((noinline)) use_result(int r) {
    dummy_sink = r;
}

/* Non-inlineable helper to force side effects */
static int __attribute__((noinline,noipa)) 
dummy_side_effect(float f) {
    volatile static float sink;
    sink = f;
    return sink != 0.0f;
}

/* Helper function implementing NaN-aware comparisons */
static int __attribute__((noinline))
test_nan_comparisons(float a, float b, double c, double d, int opcode) {
    int result = 0;
    volatile int local_sink = 0;
    
    /* Complex expression with side effects */
    local_sink += dummy_side_effect(a) + dummy_side_effect(b);
    
    switch (opcode) {
        case 0: { /* UNEQ_EXPR: unordered or equal */
            /* Using GCC builtins that may generate UNEQ_EXPR */
            int r1 = __builtin_isunordered(a, b) || (a == b);
            int r2 = !__builtin_isgreater(c, d) && !__builtin_isless(c, d);
            result = r1 | r2;
            
            /* Additional complex expression */
            float temp = a + b;
            local_sink += __builtin_isnan(temp) ? 1 : 0;
            break;
        }
        
        case 1: { /* LTGT_EXPR: ordered and not equal */
            /* Using comparisons that may generate LTGT_EXPR */
            int r1 = (a < b) || (a > b);
            int r2 = __builtin_islessgreater(c, d);
            result = r1 & r2;
            
            /* Force evaluation with volatile */
            volatile double v = c;
            local_sink += (v == d) ? 0 : 1;
            break;
        }
        
        case 2: { /* Mixed comparisons */
            /* Generate both UNEQ and LTGT patterns */
            int uneq_test = !__builtin_isless(a, b) && !__builtin_isgreater(a, b);
            int ltgt_test = __builtin_isless(c, d) || __builtin_isgreater(c, d);
            result = uneq_test ^ ltgt_test;
            
            /* Complex NaN check */
            if (__builtin_isnan(a) || __builtin_isnan(b)) {
                local_sink += 2;
            }
            break;
        }
        
        case 3: { /* Direct NaN comparisons */
            /* Explicit NaN checks that may fold */
            int nan_a = __builtin_isnan(a);
            int nan_b = __builtin_isnan(b);
            int nan_c = __builtin_isnan(c);
            int nan_d = __builtin_isnan(d);
            
            /* UNEQ-like: equal OR both NaN */
            result = (a == b) || (nan_a && nan_b);
            
            /* LTGT-like: ordered and not equal */
            result |= (!nan_c && !nan_d && c != d);
            
            /* Force side effect */
            local_sink += nan_a + nan_b + nan_c + nan_d;
            break;
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(result + local_sink);
    return result;
}

/* Main test driver */
int main(void) {
    /* Create various floating-point values */
    float f_nan = 0.0f / 0.0f;
    float f_inf = __builtin_inff();
    float f_neg_inf = -__builtin_inff();
    float f_zero = 0.0f;
    float f_one = 1.0f;
    float f_neg = -1.0f;
    
    /* sqrt(-1) produces NaN */
    float f_sqrt_nan = __builtin_sqrtf(-1.0f);
    
    double d_nan = __builtin_nan("");
    double d_snan = __builtin_nans("");
    double d_inf = __builtin_inf();
    double d_neg_inf = -__builtin_inf();
    double d_zero = 0.0;
    double d_one = 1.0;
    double d_half = 0.5;
    
    /* Test combinations */
    struct {
        float a, b;
        double c, d;
    } test_cases[] = {
        {f_nan, f_one, d_nan, d_one},
        {f_zero, f_nan, d_zero, d_nan},
        {f_inf, f_neg_inf, d_inf, d_neg_inf},
        {f_one, f_one, d_one, d_one},
        {f_sqrt_nan, f_zero, d_snan, d_zero},
        {f_neg, f_one, d_neg_inf, d_half},
        {f_nan, f_nan, d_nan, d_nan},
        {f_inf, f_inf, d_inf, d_inf},
    };
    
    int checksum = 0;
    const int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Loop through test cases with different opcodes */
    for (int op = 0; op < 4; op++) {
        for (int i = 0; i < num_cases; i++) {
            checksum += test_nan_comparisons(
                test_cases[i].a,
                test_cases[i].b,
                test_cases[i].c,
                test_cases[i].d,
                op
            );
            
            /* Also test with swapped arguments */
            checksum += test_nan_comparisons(
                test_cases[i].b,
                test_cases[i].a,
                test_cases[i].d,
                test_cases[i].c,
                op
            );
        }
    }
    
    /* Additional tests with computation in arguments */
    for (int i = 0; i < 100; i++) {
        float dynamic_a = (i % 2) ? f_nan : (float)i;
        double dynamic_c = (i % 3) ? d_nan : (double)(i * 0.5);
        
        checksum += test_nan_comparisons(
            dynamic_a,
            f_one + (i % 5),
            dynamic_c,
            d_one * i,
            i % 4
        );
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Dummy sink: %d\n", dummy_sink);
    
    return 0;
}
