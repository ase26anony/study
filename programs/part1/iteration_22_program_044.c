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
    side_effect += (int)(a * 1000.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like behavior */
            /* unordered or equal: a == b OR (a != a AND b != b) */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Using macros for UNEQ semantics */
            if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
                result |= 2;
            }
            side_effect++;
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: a != b AND !(a != a OR b != b) */
            if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
                result |= 4;
            }
            /* Alternative formulation */
            if (a != b && !__builtin_isunordered(a, b)) {
                result |= 8;
            }
            side_effect += 2;
            break;
            
        case 2: /* Mixed comparisons with floats */
            if (__builtin_isunordered(c, d)) {
                result |= 16;
            }
            if (c == d || (__builtin_isnan(c) && __builtin_isnan(d))) {
                result |= 32;
            }
            side_effect += 3;
            break;
            
        case 3: /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(b)) {
                result |= 64;
            }
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 128;
            }
            side_effect += 4;
            break;
            
        default:
            /* Use comparison macros from math.h */
            result |= (isgreater(a, b) ? 256 : 0);
            result |= (isless(a, b) ? 512 : 0);
            result |= (isunordered(a, b) ? 1024 : 0);
            side_effect += 5;
    }
    
    /* Additional side effect to prevent optimization */
    volatile double dummy = a + b + c + d + side_effect;
    (void)dummy;
    
    return result;
}

/* Non-inlineable helper with volatile arguments */
static __attribute__((noinline, optimize("O0")))
int perform_comparisons(volatile double x, volatile double y, 
                        volatile float u, volatile float v, int mode) {
    int checksum = 0;
    
    /* Force evaluation of volatile values */
    double a = x;
    double b = y;
    float c = u;
    float d = v;
    
    /* Test all opcodes with different combinations */
    for (int op = 0; op < 5; op++) {
        checksum += test_nan_comparisons(op, a, b, c, d);
        checksum += test_nan_comparisons(op, b, a, d, c); /* swapped */
    }
    
    /* Cross-type comparisons */
    checksum += (__builtin_isnan(a) ? 1 : 0);
    checksum += (__builtin_isnan(c) ? 2 : 0);
    checksum += (isunordered(a, (double)c) ? 4 : 0);
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    /* Create various NaN values using different methods */
    double nan1 = 0.0 / 0.0;                    /* quiet NaN */
    double nan2 = __builtin_nan("");            /* another quiet NaN */
    double nan3 = -__builtin_nan("0x1234");     /* NaN with payload */
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    /* Float versions */
    float nanf1 = 0.0f / 0.0f;
    float nanf2 = __builtin_nanf("");
    float inf_f = __builtin_inff();
    
    /* Normal numbers */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    float normalf1 = 1.414f;
    float normalf2 = 1.732f;
    
    /* Zero values (can create NaN when divided by zero) */
    double zero = 0.0;
    double neg_zero = -0.0;
    
    /* Test combinations that should generate UNEQ_EXPR and LTGT_EXPR */
    struct {
        double a, b;
        float c, d;
        int mode;
    } test_cases[] = {
        /* NaN vs NaN (should trigger unordered comparisons) */
        {nan1, nan2, nanf1, nanf2, 0},
        /* NaN vs normal */
        {nan1, normal1, nanf1, normalf1, 1},
        /* normal vs NaN */
        {normal2, nan3, normalf2, nanf2, 2},
        /* Infinity comparisons */
        {inf, neg_inf, inf_f, -inf_f, 3},
        /* Normal comparisons */
        {normal1, normal2, normalf1, normalf2, 4},
        /* Zero/NaN edge cases */
        {zero, nan1, 0.0f, nanf1, 5},
        {neg_zero, zero, -0.0f, 0.0f, 6},
        /* Division that could produce NaN depending on flags */
        {1.0/zero, -1.0/zero, 1.0f/0.0f, -1.0f/0.0f, 7},
    };
    
    /* Run all test cases multiple times */
    for (unsigned i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        for (int repeat = 0; repeat < 3; repeat++) {
            total_checksum += perform_comparisons(
                test_cases[i].a,
                test_cases[i].b,
                test_cases[i].c,
                test_cases[i].d,
                test_cases[i].mode
            );
            
            /* Modify values slightly to create variation */
            test_cases[i].a += 0.1;
            test_cases[i].c += 0.1f;
        }
    }
    
    /* Additional tests with computed NaN values */
    volatile double computed_nan = sqrt(-1.0);
    volatile float computed_nanf = sqrtf(-1.0f);
    
    total_checksum += perform_comparisons(
        computed_nan, normal1, computed_nanf, normalf1, 8
    );
    
    /* Print deterministic checksum */
    printf("Checksum: %d\n", total_checksum);
    
    return 0;
}
