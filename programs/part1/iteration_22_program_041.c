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
    sink += (int)(a * 0.5) + (int)(b * 0.3);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins to generate UNEQ_EXPR */
            int cmp1 = (__builtin_isunordered(a, b) || a == b) ? 1 : 0;
            int cmp2 = (isunordered(c, d) || c == d) ? 1 : 0;
            result = cmp1 ^ cmp2;
            sink += result;
            break;
        }
        case 1: { /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using comparison macros to generate LTGT_EXPR */
            int cmp1 = (!__builtin_isunordered(a, b) && a != b) ? 1 : 0;
            int cmp2 = (!isunordered(c, d) && c != d) ? 1 : 0;
            result = cmp1 | cmp2;
            sink += result;
            break;
        }
        case 2: { /* Mixed comparisons to increase coverage */
            /* Direct NaN checks with comparisons */
            int cmp1 = (__builtin_isnan(a) || __builtin_isgreater(a, b)) ? 1 : 0;
            int cmp2 = (isnan(c) || islessequal(c, d)) ? 1 : 0;
            result = cmp1 & cmp2;
            sink += result;
            break;
        }
        case 3: { /* Complex expression with multiple comparisons */
            /* This should generate various comparison tree codes */
            int cmp1 = (__builtin_isunordered(a, b) && !__builtin_isnan(a)) ? 1 : 0;
            int cmp2 = (!isunordered(c, d) && isnan(d)) ? 1 : 0;
            int cmp3 = (a != b && !__builtin_isnan(b)) ? 1 : 0;
            result = cmp1 ^ cmp2 ^ cmp3;
            sink += result;
            break;
        }
        default: {
            /* Fallback with standard comparisons */
            result = (a > b) ? 1 : 0;
            sink += result;
            break;
        }
    }
    
    /* Additional side effect to prevent optimization */
    asm volatile("" : "+r"(sink));
    
    return result + sink;
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline))
void dummy_side_effect(int *counter) {
    *counter += 1;
    volatile int tmp = *counter;
    (void)tmp;
}

int main(void) {
    /* Initialize test values including NaNs */
    double d_nan1 = 0.0 / 0.0;                    /* Quiet NaN */
    double d_nan2 = __builtin_nan("");            /* Another NaN */
    double d_inf = __builtin_inf();               /* Infinity */
    double d_normal = 3.14159;
    double d_zero = 0.0;
    double d_neg = -2.71828;
    
    float f_nan1 = sqrtf(-1.0f);                  /* Signaling NaN */
    float f_nan2 = __builtin_nanf("0xdead");      /* Another NaN */
    float f_inf = __builtin_inff();               /* Float infinity */
    float f_normal = 2.0f;
    float f_zero = -0.0f;
    float f_neg = -1.0f;
    
    /* Array of test value pairs */
    struct {
        double a;
        double b;
        float c;
        float d;
    } test_cases[] = {
        {d_nan1, d_normal, f_nan1, f_normal},
        {d_normal, d_nan2, f_normal, f_nan2},
        {d_nan1, d_nan2, f_nan1, f_nan2},
        {d_inf, d_normal, f_inf, f_normal},
        {d_normal, d_inf, f_normal, f_inf},
        {d_zero, d_neg, f_zero, f_neg},
        {d_neg, d_zero, f_neg, f_zero},
        {d_inf, d_nan1, f_inf, f_nan1},
        {d_nan1, d_inf, f_nan1, f_inf},
    };
    
    int checksum = 0;
    int side_counter = 0;
    
    /* Test all combinations */
    for (int op = 0; op < 4; ++op) {
        for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); ++i) {
            /* Call comparison function */
            int res = test_nan_comparisons(
                op,
                test_cases[i].a,
                test_cases[i].b,
                test_cases[i].c,
                test_cases[i].d
            );
            
            checksum += res;
            checksum ^= (op << 4) | (i & 0xF);
            
            /* Add side effects to prevent dead code elimination */
            dummy_side_effect(&side_counter);
            
            /* Additional volatile store */
            volatile int tmp = checksum;
            (void)tmp;
        }
    }
    
    /* More complex expressions in main to generate additional comparisons */
    volatile double v1 = d_nan1;
    volatile double v2 = d_normal;
    volatile float v3 = f_nan1;
    volatile float v4 = f_normal;
    
    /* These should generate UNEQ_EXPR and LTGT_EXPR nodes */
    int extra1 = (__builtin_isunordered(v1, v2) || v1 == v2) ? 1 : 0;
    int extra2 = (!isunordered(v3, v4) && v3 != v4) ? 1 : 0;
    
    checksum += extra1 + extra2;
    
    /* Print deterministic result */
    printf("Checksum: %d\n", checksum);
    printf("Side effects: %d\n", side_counter);
    
    return 0;
}
