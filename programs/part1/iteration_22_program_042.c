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
    side_effect += (int)((uintptr_t)&side_effect) % 256;
    
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins that may generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Alternative using math.h macros */
            if (isunordered(c, d) || c == d) {
                result |= 2;
            }
            /* Direct NaN check combination */
            if (__builtin_isnan(a) || __builtin_isnan(b) || a == b) {
                result |= 4;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using GCC builtins that may generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && a != b) {
                result |= 1;
            }
            /* Alternative using math.h macros */
            if (!isunordered(c, d) && c != d) {
                result |= 2;
            }
            /* Direct ordered comparison */
            if (!__builtin_isnan(a) && !__builtin_isnan(b) && a != b) {
                result |= 4;
            }
            break;
            
        case 2: /* Mixed comparisons to increase coverage */
            /* Complex expression that might fold to UNEQ_EXPR */
            if ((__builtin_isgreater(a, b) || __builtin_isless(a, b) || 
                 __builtin_isunordered(a, b) || a == b)) {
                result |= 1;
            }
            /* Another complex expression */
            if ((!isunordered(c, d) && c != d) || (isunordered(c, d) && c == d)) {
                result |= 2;
            }
            break;
            
        default:
            /* More comparison variants */
            if (__builtin_islessgreater(a, b)) {
                result |= 1;
            }
            if (islessgreater(c, d)) {
                result |= 2;
            }
            break;
    }
    
    /* Additional side effects to prevent optimization */
    side_effect += result;
    asm volatile("" : "+r" (side_effect) : : "memory");
    
    return result + side_effect;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
    *counter += (*counter * 13 + 7) % 19;
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double nan_dbl = 0.0 / 0.0;          /* Quiet NaN */
    double neg_nan_dbl = -nan_dbl;       /* Negative NaN */
    double inf_dbl = __builtin_inf();    /* Positive infinity */
    double neg_inf_dbl = -inf_dbl;       /* Negative infinity */
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Create NaN via sqrt(-1) */
    double sqrt_nan_dbl = __builtin_sqrt(-1.0);
    
    /* Float versions */
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    /* Using __builtin_nan directly */
    double builtin_nan = __builtin_nan("");
    float builtin_nanf = __builtin_nanf("");
    
    int checksum = 0;
    volatile int loop_counter = 0;
    
    /* Test different combinations of values and opcodes */
    for (int i = 0; i < 100; i++) {
        int op = i % 4;
        
        /* Vary the arguments based on loop iteration */
        double a, b;
        float c, d;
        
        switch (i % 7) {
            case 0: a = nan_dbl; b = normal_dbl; break;
            case 1: a = normal_dbl; b = nan_dbl; break;
            case 2: a = nan_dbl; b = nan_dbl; break;
            case 3: a = inf_dbl; b = normal_dbl; break;
            case 4: a = normal_dbl; b = inf_dbl; break;
            case 5: a = sqrt_nan_dbl; b = builtin_nan; break;
            case 6: a = zero_dbl; b = normal_dbl; break;
        }
        
        switch (i % 5) {
            case 0: c = nan_flt; d = normal_flt; break;
            case 1: c = normal_flt; d = nan_flt; break;
            case 2: c = nan_flt; d = nan_flt; break;
            case 3: c = inf_flt; d = normal_flt; break;
            case 4: c = normal_flt; d = zero_flt; break;
        }
        
        /* Call the comparison function */
        int res = test_nan_comparisons(op, a, b, c, d);
        checksum = (checksum * 31 + res) % 1000000;
        
        /* Add dummy calls to create control flow complexity */
        if (i % 13 == 0) {
            dummy_call(&checksum);
        }
        
        loop_counter++;
    }
    
    /* Additional tests with specific value combinations */
    struct {
        double a, b;
        float c, d;
    } special_cases[] = {
        {__builtin_nan("0xdead"), __builtin_nan("0xbeef"), 
         __builtin_nanf("0xdead"), __builtin_nanf("0xbeef")},
        {inf_dbl, neg_inf_dbl, inf_flt, -inf_flt},
        {normal_dbl, normal_dbl, normal_flt, normal_flt},
        {nan_dbl, inf_dbl, nan_flt, inf_flt},
        {0.0, -0.0, 0.0f, -0.0f},
    };
    
    for (size_t j = 0; j < sizeof(special_cases)/sizeof(special_cases[0]); j++) {
        for (int op = 0; op < 4; op++) {
            int res = test_nan_comparisons(op, 
                special_cases[j].a, special_cases[j].b,
                special_cases[j].c, special_cases[j].d);
            checksum = (checksum * 17 + res) % 1000000;
        }
    }
    
    /* Print deterministic checksum */
    printf("Checksum: %d\n", checksum);
    
    /* Use results to affect return value */
    return (checksum == 0) ? 1 : 0;
}
