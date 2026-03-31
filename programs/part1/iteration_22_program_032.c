/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
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
            /* Using __builtin_isunordered and direct equality */
            if (__builtin_isunordered(a, b) || (a == b)) {
                result = 1;
                sink += 123;
            }
            /* Also test with float types */
            if (isunordered(c, d) || (c == d)) {
                result |= 2;
                sink += 456;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using ordered comparison macros */
            if (!__builtin_isunordered(a, b) && (a != b)) {
                result = 3;
                sink += 789;
            }
            if (!isunordered(c, d) && (c != d)) {
                result |= 4;
                sink += 1011;
            }
            break;
            
        case 2: /* Mixed comparisons using builtins */
            /* Generate UNEQ_EXPR via __builtin_isgreater */
            if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) {
                result = 5;
                sink += 1213;
            }
            /* Generate LTGT_EXPR via __builtin_isless */
            if (__builtin_isless(c, d) && !__builtin_isunordered(c, d)) {
                result |= 6;
                sink += 1415;
            }
            break;
            
        case 3: /* Direct NaN checks with comparisons */
            /* UNEQ_EXPR pattern */
            if (__builtin_isnan(a) || __builtin_isnan(b) || (a == b)) {
                result = 7;
                sink += 1617;
            }
            /* LTGT_EXPR pattern */
            if (!__builtin_isnan(c) && !__builtin_isnan(d) && (c != d)) {
                result |= 8;
                sink += 1819;
            }
            break;
    }
    
    /* Ensure sink is used */
    return result + (sink & 1);
}

/* Dummy function to prevent optimization */
static __attribute__((noinline)) 
void use_result(int res) {
    volatile static int storage = 0;
    storage += res;
}

int main(void) {
    /* Create various floating-point values */
    double d_nan = 0.0 / 0.0;                    /* Quiet NaN */
    double d_nan2 = __builtin_nan("0xdead");     /* Another NaN */
    double d_inf = __builtin_inf();              /* Positive infinity */
    double d_neg_inf = -__builtin_inf();         /* Negative infinity */
    double d_zero = 0.0;
    double d_one = 1.0;
    double d_neg = -1.0;
    
    /* Float versions */
    float f_nan = 0.0f / 0.0f;
    float f_nan2 = __builtin_nanf("");
    float f_inf = __builtin_inff();
    float f_zero = 0.0f;
    float f_one = 1.0f;
    
    /* Additional NaN-producing computations */
    double d_sqrt_nan = sqrt(-1.0);
    float f_sqrt_nan = sqrtf(-1.0f);
    double d_log_nan = log(-1.0);
    
    int checksum = 0;
    
    /* Test different combinations */
    const int num_tests = 8;
    double test_values_d[] = {d_nan, d_inf, d_zero, d_one, d_neg_inf, d_nan2, d_sqrt_nan, d_log_nan};
    float test_values_f[] = {f_nan, f_inf, f_zero, f_one, f_nan2, f_sqrt_nan, 2.0f, -2.0f};
    
    /* Loop through various combinations */
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            for (int op = 0; op < 8; op++) {
                int res = test_nan_comparisons(
                    op, 
                    test_values_d[i], 
                    test_values_d[j],
                    test_values_f[i % 8],
                    test_values_f[j % 8]
                );
                checksum += res;
                use_result(res);
            }
        }
    }
    
    /* Additional specific tests targeting UNEQ_EXPR and LTGT_EXPR */
    
    /* Test 1: NaN vs NaN (should be unordered) */
    checksum += test_nan_comparisons(0, d_nan, d_nan2, f_nan, f_nan2);
    
    /* Test 2: NaN vs normal number */
    checksum += test_nan_comparisons(1, d_nan, d_one, f_nan, f_one);
    
    /* Test 3: Infinity comparisons */
    checksum += test_nan_comparisons(2, d_inf, d_neg_inf, f_inf, -f_inf);
    
    /* Test 4: Equal values */
    checksum += test_nan_comparisons(3, d_zero, -d_zero, f_zero, -f_zero);
    
    /* Test 5: Using isgreater/isless macros explicitly */
    volatile double v1 = d_nan;
    volatile double v2 = d_one;
    volatile float v3 = f_nan;
    volatile float v4 = f_one;
    
    /* These should generate UNEQ_EXPR/LTGT_EXPR nodes */
    if (isgreater(v1, v2) || isunordered(v1, v2)) {
        checksum += 1000;
    }
    
    if (isless(v3, v4) && !isunordered(v3, v4)) {
        checksum += 2000;
    }
    
    /* Complex expression mixing types and operations */
    checksum += (isunordered(d_sqrt_nan, d_log_nan) || (d_sqrt_nan == d_log_nan)) ? 3000 : 0;
    checksum += (!isunordered(f_sqrt_nan, f_nan) && (f_sqrt_nan != f_nan)) ? 4000 : 0;
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
