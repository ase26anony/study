/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0; /* Side effect variable */
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode;
    
    /* Switch-like structure for different comparison types */
    switch (opcode & 3) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using GCC builtins to generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || a == b) {
                result |= 1;
            }
            /* Additional complex expression */
            sink += (__builtin_isnan(c) ? 1 : 0);
            if (!__builtin_islessgreater(c, d) || __builtin_isunordered(c, d)) {
                result |= 2;
            }
            break;
            
        case 1: /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using comparison macros to generate LTGT_EXPR */
            if (islessgreater(a, b)) {
                result |= 4;
            }
            /* Mixed float/double comparisons */
            sink += (__builtin_isnan((double)c) ? 2 : 0);
            if (__builtin_isgreater(a, (double)d) || __builtin_isless(a, (double)d)) {
                if (!__builtin_isunordered(a, (double)d)) {
                    result |= 8;
                }
            }
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ_EXPR and LTGT_EXPR in same basic block */
            if (__builtin_isunordered(a, b) || a == b) {
                sink += 4;
            }
            if (islessgreater(c, d)) {
                sink += 8;
            }
            /* Force evaluation with volatile */
            result = sink & 15;
            break;
            
        default:
            /* More complex NaN-aware comparisons */
            int temp = 0;
            temp += __builtin_isnan(a) ? 16 : 0;
            temp += __builtin_isnan(b) ? 32 : 0;
            temp += isgreater(c, d) ? 64 : 0;
            temp += isless(c, d) ? 128 : 0;
            
            /* Conditional that might fold to UNEQ_EXPR/LTGT_EXPR */
            if ((__builtin_isunordered(a, b) || a == b) && 
                !__builtin_isnan(c) && !__builtin_isnan(d)) {
                result = temp;
            }
            sink = temp;
            break;
    }
    
    /* Ensure side effects aren't optimized away */
    asm volatile("" : "+r" (sink));
    return result + sink;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("no-tree-pre")))
int compute_checksum(int iterations) {
    int checksum = 0;
    
    /* Create NaN values using different methods */
    double nan_dbl = 0.0 / 0.0;
    double neg_nan = -nan_dbl;
    double sqrt_nan = __builtin_sqrt(-1.0);
    double builtin_nan = __builtin_nan("");
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float neg_inf_flt = -inf_flt;
    
    /* Normal numbers */
    double normal_dbl = 3.14159;
    float normal_flt = 2.71828f;
    
    /* Array of test values */
    double dbl_vals[] = {nan_dbl, neg_nan, sqrt_nan, builtin_nan, normal_dbl, 
                         __builtin_inf(), -__builtin_inf(), 0.0};
    float flt_vals[] = {nan_flt, inf_flt, neg_inf_flt, normal_flt, 0.0f};
    
    int dbl_count = sizeof(dbl_vals) / sizeof(dbl_vals[0]);
    int flt_count = sizeof(flt_vals) / sizeof(flt_vals[0]);
    
    /* Perform many comparisons with different combinations */
    for (int i = 0; i < iterations; i++) {
        for (int op = 0; op < 4; op++) {
            for (int j = 0; j < dbl_count; j++) {
                for (int k = 0; k < flt_count; k++) {
                    /* Mix different types and NaN sources */
                    double a = dbl_vals[j];
                    double b = dbl_vals[(j + op) % dbl_count];
                    float c = flt_vals[k];
                    float d = flt_vals[(k + op + 1) % flt_count];
                    
                    /* Call comparison function - cannot be inlined */
                    int res = test_nan_comparisons(op + i, a, b, c, d);
                    
                    /* Update checksum deterministically */
                    checksum = (checksum * 31 + res) & 0x7FFFFFFF;
                }
            }
        }
    }
    
    return checksum;
}

int main(void) {
    /* Compute checksum to ensure all code executes */
    int checksum = compute_checksum(3);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile store to prevent optimization */
    volatile int dummy = checksum;
    
    return 0;
}
