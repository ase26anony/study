/* test_nan_comparisons.c - Targeting fold-const.cc lines 6248-6258 */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects to prevent early optimization */
    side_effect += (int)(a * 100.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins to generate UNEQ_EXPR tree nodes */
            int cmp1 = __builtin_isunordered(a, b) || (a == b);
            int cmp2 = __builtin_isunordered(c, d) || (c == d);
            result = cmp1 & cmp2;
            side_effect += result;
            break;
        }
        case 1: { /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using standard math.h macros that generate LTGT_EXPR */
            int cmp1 = islessgreater(a, b);
            int cmp2 = islessgreater(c, d);
            result = cmp1 | cmp2;
            side_effect += result;
            break;
        }
        case 2: { /* Mixed comparisons to increase coverage */
            /* Direct NaN checks combined with ordered comparisons */
            int cmp1 = !__builtin_isnan(a) && !__builtin_isnan(b) && (a != b);
            int cmp2 = __builtin_isgreater(c, d) || __builtin_isless(c, d);
            result = cmp1 ^ cmp2;
            side_effect += result;
            break;
        }
        case 3: { /* Complex expression with volatile side effects */
            volatile float vf = c;
            volatile double vd = b;
            
            /* UNEQ_EXPR pattern using math.h macros */
            int cmp1 = isunordered(a, vd) || (a == vd);
            
            /* LTGT_EXPR pattern using builtins */
            int cmp2 = __builtin_islessgreater(vf, d);
            
            result = (cmp1 != 0) && (cmp2 != 0);
            side_effect += (int)vf + (int)vd;
            break;
        }
        default: {
            /* Fallback with multiple comparison types */
            result = __builtin_isnan(a) || __builtin_isnan(b);
            side_effect += opcode;
            break;
        }
    }
    
    /* Additional side effects to prevent dead code elimination */
    asm volatile("" : "+r"(side_effect));
    return result + side_effect;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("no-tree-vectorize")))
void run_comparison_suite(double* dvals, float* fvals, int n) {
    int checksum = 0;
    
    /* Test all combinations of values with different opcodes */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int op = 0; op < 4; op++) {
                checksum += test_nan_comparisons(
                    op, 
                    dvals[i], 
                    dvals[j], 
                    fvals[i], 
                    fvals[j]
                );
                
                /* Additional test with swapped arguments */
                checksum += test_nan_comparisons(
                    op | 1,  /* Modify opcode slightly */
                    dvals[j], 
                    dvals[i], 
                    fvals[j], 
                    fvals[i]
                );
            }
        }
    }
    
    /* Use checksum to prevent optimization */
    volatile int final_result = checksum;
    printf("Checksum: %d\n", final_result);
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double dvals[6];
    float fvals[6];
    
    /* Normal numbers */
    dvals[0] = 1.0;
    dvals[1] = -3.14159;
    dvals[2] = 0.0;
    fvals[0] = 2.0f;
    fvals[1] = -1.5f;
    fvals[2] = 0.0f;
    
    /* Generate NaNs using different methods */
    dvals[3] = 0.0 / 0.0;                    /* Quiet NaN */
    dvals[4] = __builtin_nan("");            /* Another NaN */
    dvals[5] = __builtin_nan("0xdeadbeef");  /* NaN with payload */
    
    fvals[3] = 0.0f / 0.0f;                  /* Float NaN */
    fvals[4] = __builtin_nanf("");           /* Float NaN */
    fvals[5] = sqrtf(-1.0f);                 /* NaN from invalid operation */
    
    /* Infinities for additional test cases */
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    float finf = __builtin_inff();
    float fneg_inf = -__builtin_inff();
    
    /* Add infinities to test arrays occasionally */
    dvals[1] = (dvals[1] < 0.0) ? neg_inf : dvals[1];
    fvals[4] = (rand() % 2) ? finf : fvals[4];
    
    /* Run the comparison tests multiple times */
    for (int iter = 0; iter < 3; iter++) {
        run_comparison_suite(dvals, fvals, 6);
        
        /* Modify values slightly each iteration */
        dvals[0] += 0.1;
        fvals[0] -= 0.05f;
    }
    
    /* Additional targeted tests for UNEQ_EXPR and LTGT_EXPR */
    volatile double test_nan = __builtin_nan("test");
    volatile double test_num = 42.0;
    volatile float test_fnan = __builtin_nanf("");
    volatile float test_fnum = -7.0f;
    
    /* Direct tests that should generate specific tree codes */
    int uneq_results = 0;
    uneq_results += __builtin_isunordered(test_nan, test_num) || (test_nan == test_num);
    uneq_results += isunordered(test_fnan, test_fnum) || (test_fnan == test_fnum);
    
    int ltgt_results = 0;
    ltgt_results += islessgreater(test_num, test_nan);
    ltgt_results += __builtin_islessgreater(test_fnum, test_fnan);
    ltgt_results += !__builtin_isnan(test_num) && !__builtin_isnan(3.14) && (test_num != 3.14);
    
    printf("Final UNEQ tests: %d, LTGT tests: %d\n", uneq_results, ltgt_results);
    
    return 0;
}
