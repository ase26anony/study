/* test_nan_folding.c - Target fold-const.cc lines 6248-6258 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static volatile int side_effect_counter = 0;
static void __attribute__((noinline)) dummy_side_effect(int x) {
    side_effect_counter += x;
}

/* Helper to create NaN values using different methods */
static double make_nan_double(void) {
    return 0.0 / 0.0;  /* Will generate quiet NaN */
}

static float make_nan_float(void) {
    float zero = 0.0f;
    return zero / zero;
}

static double make_snan_double(void) {
    /* Try to create signaling NaN if supported */
    return __builtin_nans("");
}

/* Non-inlineable function with complex NaN-aware comparisons */
static int __attribute__((noinline,optimize("no-tree-vectorize")))
test_nan_comparisons(int opcode, double a, double b, float fa, float fb) {
    volatile int result = 0;
    volatile double vd;
    volatile float vf;
    
    /* Use switch-like structure to generate different comparison types */
    switch (opcode & 3) {
        case 0: /* UNEQ_EXPR scenarios */
            /* Double precision UNEQ-like comparisons */
            if (!__builtin_isunordered(a, b) || a == b) {
                dummy_side_effect(1);
                vd = a + b;  /* Side effect */
                result += 1;
            }
            
            /* Float precision with isnan check */
            if (__builtin_isnan(fa) || __builtin_isnan(fb)) {
                if (!__builtin_isunordered(fa, fb) || fa == fb) {
                    dummy_side_effect(2);
                    vf = fa * fb;
                    result += 2;
                }
            }
            
            /* Mixed float/double using comparison macros */
            if (isunordered((double)fa, b) || (double)fa == b) {
                dummy_side_effect(3);
                result += 4;
            }
            break;
            
        case 1: /* LTGT_EXPR scenarios */
            /* Ordered and not equal comparisons */
            if (!__builtin_isunordered(a, b) && a != b) {
                dummy_side_effect(4);
                vd = a - b;
                result += 8;
            }
            
            /* Float version with explicit checks */
            if (!isunordered(fa, fb) && fa != fb) {
                dummy_side_effect(5);
                vf = fa / (fb + 1.0f);
                result += 16;
            }
            
            /* Using comparison macros for LTGT semantics */
            if (isgreater(a, b) || isless(a, b)) {
                dummy_side_effect(6);
                result += 32;
            }
            break;
            
        case 2: /* Mixed comparisons to generate both tree codes */
            /* First UNEQ-like */
            if ((!__builtin_isunordered(a, a) || a == a) && __builtin_isnan(a)) {
                dummy_side_effect(7);
                result += 64;
            }
            
            /* Then LTGT-like */
            if (!__builtin_isunordered(b, b) && b != b && __builtin_isnan(b)) {
                dummy_effect(8);
                result += 128;
            }
            
            /* Complex expression that could fold to either */
            if ((!isunordered(fa, fb) && fa != fb) || 
                (isunordered(fa, fb) || fa == fb)) {
                dummy_side_effect(9);
                vf = fa + fb;
                result += 256;
            }
            break;
            
        case 3: /* Direct builtin usage */
            /* Generate UNEQ_EXPR via builtins */
            int unord_eq = __builtin_isunordered(a, b) ? 0 : (a == b);
            if (unord_eq) {
                dummy_side_effect(10);
                result += 512;
            }
            
            /* Generate LTGT_EXPR via builtins */
            int ord_neq = !__builtin_isunordered(fa, fb) && (fa != fb);
            if (ord_neq) {
                dummy_side_effect(11);
                result += 1024;
            }
            break;
    }
    
    /* Additional complex expression that might fold */
    volatile double complex_expr = 
        (__builtin_isnan(a) ? 0 : (!__builtin_isunordered(a, b) || a == b)) +
        (__builtin_isnan(b) ? 0 : (!__builtin_isunordered(a, b) && a != b));
    
    dummy_side_effect((int)complex_expr);
    
    return result;
}

int main(void) {
    /* Initialize test values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double inf_pos = __builtin_inf();
    double inf_neg = -__builtin_inf();
    double nan_dbl = make_nan_double();
    double snan_dbl = make_snan_double();
    double zero = 0.0;
    
    float normal_f1 = 1.41421f;
    float normal_f2 = 1.61803f;
    float nan_float = make_nan_float();
    float inf_pos_f = __builtin_inff();
    
    /* Test cases designed to generate UNEQ_EXPR and LTGT_EXPR */
    struct {
        int opcode;
        double a;
        double b;
        float fa;
        float fb;
    } test_cases[] = {
        /* Normal numbers - should generate comparisons */
        {0, normal1, normal2, normal_f1, normal_f2},
        /* NaN with normal - should trigger UNEQ/LTGT */
        {1, nan_dbl, normal1, nan_float, normal_f1},
        /* NaN with NaN */
        {2, nan_dbl, nan_dbl, nan_float, nan_float},
        /* Infinity cases */
        {3, inf_pos, inf_neg, inf_pos_f, -inf_pos_f},
        /* Zero/NaN mixtures */
        {0, zero, nan_dbl, 0.0f, nan_float},
        /* Signaling NaN */
        {1, snan_dbl, normal1, nan_float, normal_f2},
        /* Same value comparisons */
        {2, normal1, normal1, normal_f1, normal_f1},
        /* Infinity with NaN */
        {3, inf_pos, nan_dbl, inf_pos_f, nan_float},
    };
    
    int checksum = 0;
    const int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Loop through test cases multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        for (int i = 0; i < num_cases; i++) {
            /* Vary opcodes to hit different paths */
            int opcode = (test_cases[i].opcode + iteration) % 4;
            
            int result = test_nan_comparisons(
                opcode,
                test_cases[i].a,
                test_cases[i].b,
                test_cases[i].fa,
                test_cases[i].fb
            );
            
            checksum = (checksum * 31 + result) & 0xFFFF;
            
            /* Force evaluation with volatile store */
            volatile int temp = result;
            dummy_side_effect(temp);
        }
    }
    
    /* Additional direct comparisons to ensure tree node generation */
    for (int i = 0; i < 10; i++) {
        /* These should generate UNEQ_EXPR nodes */
        volatile int r1 = !__builtin_isunordered(nan_dbl, normal1) || nan_dbl == normal1;
        volatile int r2 = isunordered(nan_float, normal_f1) || nan_float == normal_f1;
        
        /* These should generate LTGT_EXPR nodes */
        volatile int r3 = !__builtin_isunordered(normal1, normal2) && normal1 != normal2;
        volatile int r4 = !isunordered(normal_f1, normal_f2) && normal_f1 != normal_f2;
        
        checksum += r1 + r2 + r3 + r4;
        dummy_side_effect(checksum);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Side effects: %d\n", side_effect_counter);
    
    return checksum == 0 ? 0 : 1;
}
