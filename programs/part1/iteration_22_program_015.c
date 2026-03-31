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
        case 0: { /* UNEQ_EXPR-like: unordered or equal */
            /* Using __builtin_isunordered and direct equality */
            if (__builtin_isunordered(a, b) || (a == b)) {
                result = 1;
            }
            /* Additional check with float types */
            if (__builtin_isunordered(c, d) || (c == d)) {
                result |= 2;
            }
            sink += result;
            break;
        }
        
        case 1: { /* LTGT_EXPR-like: less than or greater than (ordered and not equal) */
            /* Using ordered comparison macros */
            if ((__builtin_isgreater(a, b) || __builtin_isless(a, b)) && !__builtin_isunordered(a, b)) {
                result = 1;
            }
            /* Alternative formulation */
            if (!__builtin_isunordered(c, d) && (c != d)) {
                result |= 2;
            }
            sink += result * 3;
            break;
        }
        
        case 2: { /* Mixed comparisons that may generate both tree codes */
            /* Complex expression that could fold to UNEQ_EXPR */
            int r1 = !(__builtin_isnan(a) || __builtin_isnan(b)) && (a == b);
            int r2 = __builtin_isunordered(a, b);
            if (r1 || r2) {
                result = 1;
            }
            
            /* Expression that could fold to LTGT_EXPR */
            int r3 = !__builtin_isunordered(c, d) && (__builtin_isless(c, d) || __builtin_isgreater(c, d));
            if (r3) {
                result |= 4;
            }
            sink += r1 + r2 + r3;
            break;
        }
        
        case 3: { /* Direct use of comparison functions */
            /* These may generate UNEQ_EXPR/LTGT_EXPR during folding */
            result = isunordered(a, b) ? 1 : (a == b ? 2 : 0);
            if (!isunordered(c, d) && islessgreater(c, d)) {
                result |= 4;
            }
            sink += result * 5;
            break;
        }
    }
    
    /* Ensure sink is used to prevent optimization */
    asm volatile("" : "+r" (sink));
    return result;
}

/* Non-inlineable helper to ensure code generation */
static __attribute__((noinline, optimize("O0"))) 
void run_comparisons(double* dvals, float* fvals, int n, int* checksum) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int op = 0; op < 8; op++) {
                *checksum += test_nan_comparisons(
                    op, 
                    dvals[i], 
                    dvals[j], 
                    fvals[i], 
                    fvals[j]
                );
            }
        }
    }
}

int main(void) {
    /* Create various floating-point values including NaNs */
    double dvals[6];
    float fvals[6];
    
    /* Normal numbers */
    dvals[0] = 1.0;
    fvals[0] = 1.0f;
    dvals[1] = -3.14;
    fvals[1] = -3.14f;
    dvals[2] = 0.0;
    fvals[2] = 0.0f;
    
    /* Generate NaNs using different methods */
    dvals[3] = 0.0 / 0.0;                    /* Quiet NaN */
    fvals[3] = 0.0f / 0.0f;                  /* Quiet NaN (float) */
    dvals[4] = __builtin_nan("");            /* Another NaN */
    fvals[4] = __builtin_nanf("");           /* Float NaN */
    dvals[5] = __builtin_sqrt(-1.0);         /* Signaling NaN candidate */
    fvals[5] = __builtin_sqrtf(-1.0f);       /* Float signaling NaN candidate */
    
    /* Add infinities to the mix */
    double inf = __builtin_inf();
    float finf = __builtin_inff();
    
    /* Test with infinities as well */
    int checksum = 0;
    
    /* Run multiple test iterations */
    for (int iter = 0; iter < 3; iter++) {
        run_comparisons(dvals, fvals, 6, &checksum);
        
        /* Test with infinities */
        for (int i = 0; i < 6; i++) {
            for (int op = 0; op < 8; op++) {
                checksum += test_nan_comparisons(
                    op, 
                    dvals[i], 
                    inf, 
                    fvals[i], 
                    finf
                );
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
