/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;  /* Side effect variable */
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    /* Switch-like structure for different comparison types */
    switch (opcode % 4) {
        case 0: /* UNEQ_EXPR-like: unordered or equal */
            /* Using builtins to generate UNEQ_EXPR */
            if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b) && __builtin_isgreaterequal(a, b)) {
                result |= 1;
            }
            /* Another UNEQ_EXPR variant */
            if (!__builtin_islessgreater(c, d) || __builtin_isunordered(c, d)) {
                result |= 2;
            }
            sink += result;
            break;
            
        case 1: /* LTGT_EXPR-like: ordered and not equal */
            /* Using builtins to generate LTGT_EXPR */
            if (!__builtin_isunordered(a, b) && __builtin_islessgreater(a, b)) {
                result |= 4;
            }
            /* Another LTGT_EXPR variant */
            if (__builtin_islessgreater(c, d) && !__builtin_isunordered(c, d)) {
                result |= 8;
            }
            sink += result * 3;
            break;
            
        case 2: /* Mixed comparisons */
            /* Generate both UNEQ_EXPR and LTGT_EXPR in same basic block */
            if (__builtin_isnan(a) || __builtin_isnan(b)) {
                if (!__builtin_islessgreater(a, b) || __builtin_isunordered(a, b)) {
                    result |= 16;  /* UNEQ_EXPR path */
                }
            } else {
                if (!__builtin_isunordered(c, d) && __builtin_islessgreater(c, d)) {
                    result |= 32;  /* LTGT_EXPR path */
                }
            }
            sink += result / 2;
            break;
            
        case 3: /* Complex nested comparisons */
            /* Create conditions that might fold differently based on HONOR_NANS */
            int temp = 0;
            if (__builtin_isunordered(a, b)) {
                temp = 1;
            } else if (a == b) {  /* This might become UNEQ_EXPR */
                temp = 2;
            } else if (a < b || a > b) {  /* This might become LTGT_EXPR */
                temp = 3;
            }
            
            /* Force evaluation with side effects */
            if (temp == 1 || temp == 2) {
                result |= 64;  /* UNEQ_EXPR-like */
            }
            if (temp == 3) {
                result |= 128; /* LTGT_EXPR-like */
            }
            sink += temp;
            break;
    }
    
    /* Additional side effects to prevent optimization */
    asm volatile("" : "+r" (sink) : : "memory");
    return result + sink;
}

/* Dummy function to prevent optimization */
static __attribute__((noinline)) 
void use_result(int val) {
    volatile static int storage;
    storage = val;
}

int main() {
    /* Create various floating-point values */
    double d_nan = 0.0 / 0.0;                    /* Quiet NaN */
    double d_nan2 = __builtin_nan("");           /* Another NaN */
    double d_inf = __builtin_inf();              /* Infinity */
    double d_neg_inf = -__builtin_inf();         /* Negative infinity */
    double d_normal = 3.14159;
    double d_zero = 0.0;
    double d_neg_zero = -0.0;
    
    float f_nan = sqrtf(-1.0f);                  /* Signaling NaN candidate */
    float f_nan2 = __builtin_nanf("0xdead");     /* Another NaN */
    float f_inf = __builtin_inff();
    float f_normal = 2.71828f;
    float f_zero = 0.0f;
    
    int checksum = 0;
    
    /* Test different combinations to trigger various code paths */
    const int num_tests = 16;
    for (int i = 0; i < num_tests; i++) {
        /* Vary the inputs systematically */
        double a, b;
        float c, d;
        
        switch (i % 8) {
            case 0: a = d_nan; b = d_normal; break;
            case 1: a = d_normal; b = d_nan2; break;
            case 2: a = d_inf; b = d_neg_inf; break;
            case 3: a = d_zero; b = d_neg_zero; break;
            case 4: a = d_nan; b = d_nan2; break;
            case 5: a = d_inf; b = d_inf; break;
            case 6: a = d_normal; b = d_normal; break;
            case 7: a = d_neg_inf; b = d_inf; break;
        }
        
        switch ((i / 2) % 4) {
            case 0: c = f_nan; d = f_normal; break;
            case 1: c = f_normal; d = f_nan2; break;
            case 2: c = f_inf; d = f_zero; break;
            case 3: c = f_normal; d = f_normal; break;
        }
        
        /* Call the comparison function with different opcodes */
        int res = test_nan_comparisons(i, a, b, c, d);
        checksum += res;
        
        /* Use the result to prevent dead code elimination */
        use_result(res);
        
        /* Additional comparisons directly in main to increase coverage */
        volatile int temp = 0;
        if (__builtin_isunordered(a, b) || a == b) {  /* Potential UNEQ_EXPR */
            temp += 1;
        }
        if (!__builtin_isunordered(c, d) && c != d) { /* Potential LTGT_EXPR */
            temp += 2;
        }
        checksum += temp;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
