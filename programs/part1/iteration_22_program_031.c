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
        case 0: { /* UNEQ_EXPR-like behavior */
            /* unordered or equal: a == b OR (a != a AND b != b) */
            int unordered_equal = (__builtin_isunordered(a, b) || a == b);
            /* Use result to prevent dead code elimination */
            result += unordered_equal ? 1 : 0;
            sink += unordered_equal;
            break;
        }
        case 1: { /* LTGT_EXPR-like behavior */
            /* ordered and not equal: !(a == b) AND !__builtin_isunordered(a, b) */
            int less_greater = (!__builtin_isunordered(a, b) && a != b);
            result += less_greater ? 2 : 0;
            sink += less_greater;
            break;
        }
        case 2: { /* Mixed float/double comparisons */
            /* Use GCC builtins that generate UNEQ/LTGT codes */
            int cmp1 = __builtin_isgreater(c, d);
            int cmp2 = __builtin_islessequal(c, d);
            int cmp3 = __builtin_isunordered(c, d);
            
            /* Complex expression that might fold to UNEQ/LTGT */
            int complex_cmp = (cmp1 && !cmp3) || (cmp2 && cmp3);
            result += complex_cmp ? 3 : 0;
            sink += complex_cmp;
            break;
        }
        case 3: { /* Direct NaN checks with comparisons */
            /* This should generate UNEQ_EXPR nodes */
            int nan_check1 = (__builtin_isnan(a) || __builtin_isnan(b)) ? (a == b) : 0;
            /* This should generate LTGT_EXPR nodes */
            int nan_check2 = (!__builtin_isnan(a) && !__builtin_isnan(b)) ? (a != b) : 0;
            
            result += (nan_check1 || nan_check2) ? 4 : 0;
            sink += nan_check1 + nan_check2;
            break;
        }
    }
    
    /* Additional volatile store to prevent optimization */
    *(volatile int*)&sink = result;
    return result + sink;
}

/* Another non-inlineable function with different comparison patterns */
static __attribute__((noinline, optimize("no-tree-vectorize")))
int compare_with_nans(double x, double y, int mode) {
    volatile double vsink = 0.0;
    int res = 0;
    
    /* Generate various NaN-aware comparison expressions */
    if (mode == 0) {
        /* Expression that may become UNEQ_EXPR */
        int cond = (x != x || y != y) ? 1 : (x == y);
        res = cond;
        vsink = cond ? x : y;
    } else if (mode == 1) {
        /* Expression that may become LTGT_EXPR */
        int cond = (x == x && y == y) ? (x < y || x > y) : 0;
        res = cond;
        vsink = cond ? x + y : x - y;
    } else {
        /* Mixed comparisons using math.h macros */
        int cond = isgreater(x, y) || isless(x, y) || isunordered(x, y);
        res = cond;
        vsink = x * y;
    }
    
    return res + (int)vsink;
}

int main(void) {
    /* Create various NaN and non-NaN values */
    double nan1 = 0.0 / 0.0;                    /* Quiet NaN */
    double nan2 = __builtin_nan("0xdead");      /* Another NaN with payload */
    double inf1 = __builtin_inf();              /* Positive infinity */
    double inf2 = -__builtin_inf();             /* Negative infinity */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    
    float fnan1 = 0.0f / 0.0f;
    float fnan2 = __builtin_nanf("");
    float finf = __builtin_inff();
    float fnormal = 1.41421356f;
    
    int checksum = 0;
    
    /* Test many combinations to hit different code paths */
    for (int i = 0; i < 100; i++) {
        double a, b;
        float c, d;
        
        /* Select different value combinations */
        switch (i % 8) {
            case 0: a = nan1; b = normal1; break;
            case 1: a = normal1; b = nan2; break;
            case 2: a = nan1; b = nan2; break;
            case 3: a = inf1; b = normal2; break;
            case 4: a = normal2; b = inf2; break;
            case 5: a = zero; b = neg_zero; break;
            case 6: a = inf1; b = inf1; break;
            case 7: a = normal1; b = normal2; break;
        }
        
        switch (i % 4) {
            case 0: c = fnan1; d = fnormal; break;
            case 1: c = fnormal; d = fnan2; break;
            case 2: c = finf; d = fnormal; break;
            case 3: c = fnormal; d = fnormal; break;
        }
        
        /* Call comparison functions with different opcodes */
        checksum += test_nan_comparisons(i % 4, a, b, c, d);
        checksum += compare_with_nans(a, b, i % 3);
        
        /* Additional direct comparisons that might generate target tree codes */
        volatile int v1 = (__builtin_isunordered(a, b) || a == b);
        volatile int v2 = (!__builtin_isunordered(a, b) && a != b);
        checksum += v1 + v2;
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
