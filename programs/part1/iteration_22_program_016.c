/* test_nan_comparisons.c - Target fold-const.cc lines 6248-6258 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;  /* Side effect to prevent optimization */
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode;
    
    /* Switch-like structure for different comparison types */
    switch (opcode & 3) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins to generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || __builtin_isnan(a) || __builtin_isnan(b)) {
                if (a == b || __builtin_isunordered(a, b)) {
                    result = 1;
                    sink += 123;
                }
            }
            /* Additional UNEQ pattern using macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
                sink -= 456;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using GCC builtins to generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 4;
                sink *= 2;
            }
            /* Additional LTGT pattern */
            if (!isunordered(c, d) && c != d) {
                result |= 8;
                sink /= 3;
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Complex expression that could generate both */
            if ((__builtin_isgreater(a, b) || __builtin_isless(a, b)) && 
                !__builtin_isunordered(a, b)) {
                result = 16;
                sink = sink ^ 0xFF;
            }
            if ((isgreater(c, d) || isless(c, d)) && !isunordered(c, d)) {
                result |= 32;
                sink = ~sink;
            }
            break;
            
        case 3: /* NaN-specific tests */
            /* Direct NaN checks */
            if (__builtin_isnan(a) && __builtin_isnan(b)) {
                if (isunordered(c, d)) {
                    result = 64;
                    sink = 0;
                }
            }
            break;
    }
    
    /* More side effects to prevent dead code elimination */
    volatile int* dummy = &sink;
    *dummy += result;
    
    return result + sink;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("O0"))) 
void dummy_function(int x) {
    volatile static int counter = 0;
    counter += x;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;                    /* Quiet NaN */
    double neg_nan_dbl = -__builtin_nan("");       /* Negative NaN */
    double inf_dbl = __builtin_inf();              /* Positive infinity */
    double neg_inf_dbl = -__builtin_inf();         /* Negative infinity */
    double normal_dbl = 3.141592653589793;
    double zero_dbl = 0.0;
    
    float nan_flt = 0.0f / 0.0f;                   /* Float NaN */
    float neg_nan_flt = -__builtin_nanf("");       /* Float negative NaN */
    float inf_flt = __builtin_inff();              /* Float infinity */
    float neg_inf_flt = -__builtin_inff();         /* Float negative infinity */
    float normal_flt = 2.718281828f;
    float zero_flt = 0.0f;
    
    /* Additional NaN generation methods */
    double sqrt_neg = __builtin_sqrt(-1.0);        /* NaN from sqrt(-1) */
    float log_neg = __builtin_logf(-1.0f);         /* NaN from log(-1) */
    
    /* Test cases designed to trigger UNEQ_EXPR and LTGT_EXPR */
    struct test_case {
        int opcode;
        double a;
        double b;
        float c;
        float d;
    } tests[] = {
        /* UNEQ_EXPR cases - unordered or equal */
        {0, nan_dbl, normal_dbl, nan_flt, normal_flt},
        {0, normal_dbl, nan_dbl, normal_flt, nan_flt},
        {0, nan_dbl, nan_dbl, nan_flt, nan_flt},
        {0, inf_dbl, inf_dbl, inf_flt, inf_flt},
        {0, zero_dbl, -zero_dbl, zero_flt, -zero_flt},
        
        /* LTGT_EXPR cases - ordered and not equal */
        {1, normal_dbl, normal_dbl * 2, normal_flt, normal_flt * 2},
        {1, inf_dbl, neg_inf_dbl, inf_flt, neg_inf_flt},
        {1, zero_dbl, normal_dbl, zero_flt, normal_flt},
        
        /* Mixed cases */
        {2, sqrt_neg, log_neg, nan_flt, neg_nan_flt},
        {2, normal_dbl, inf_dbl, normal_flt, inf_flt},
        
        /* NaN-specific cases */
        {3, neg_nan_dbl, sqrt_neg, neg_nan_flt, log_neg},
        {3, __builtin_nan("0xABC"), __builtin_nan("0xDEF"), 
            __builtin_nanf("0x123"), __builtin_nanf("0x456")},
    };
    
    int checksum = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    /* Execute tests multiple times to ensure coverage */
    for (int iteration = 0; iteration < 3; iteration++) {
        for (int i = 0; i < num_tests; i++) {
            /* Call non-inlineable function with side effects */
            dummy_function(i + iteration);
            
            /* Perform the comparison tests */
            int result = test_nan_comparisons(
                tests[i].opcode + iteration,
                tests[i].a,
                tests[i].b,
                tests[i].c,
                tests[i].d
            );
            
            /* Update checksum deterministically */
            checksum = (checksum * 31 + result) & 0x7FFFFFFF;
            
            /* Additional complex expression to prevent optimization */
            volatile double temp = tests[i].a * tests[i].b;
            if (__builtin_isnan(temp)) {
                checksum ^= 0x5555;
            }
        }
    }
    
    /* Print deterministic result to ensure execution */
    printf("Checksum: %d\n", checksum);
    printf("Test completed with %d iterations\n", num_tests * 3);
    
    /* Final side effect */
    volatile int final_sink = checksum;
    dummy_function(final_sink);
    
    return 0;
}
