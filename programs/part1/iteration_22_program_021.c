/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int sink = 0;  /* Side effect variable */
    int result = 0;
    
    /* Complex expression with side effects */
    sink += opcode * 2;
    
    /* Use GCC built-ins for NaN-aware comparisons */
    switch (opcode) {
        case 0: /* UNEQ_EXPR: unordered or equal */
            if (__builtin_isunordered(a, b) || a == b) {
                result = 1;
                sink += 100;
            }
            /* Also test with floats */
            if (__builtin_isunordered(c, d) || c == d) {
                result |= 2;
                sink += 200;
            }
            break;
            
        case 1: /* LTGT_EXPR: ordered and not equal (less than or greater than) */
            if (!__builtin_isunordered(a, b) && a != b) {
                result = 1;
                sink += 300;
            }
            if (!__builtin_isunordered(c, d) && c != d) {
                result |= 2;
                sink += 400;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            if (__builtin_isgreater(a, b)) {
                result = 1;
                sink += 500;
            }
            if (__builtin_islessequal(c, d)) {
                result |= 2;
                sink += 600;
            }
            break;
            
        case 3: /* Direct NaN checks */
            if (__builtin_isnan(a) || __builtin_isnan(c)) {
                result = 1;
                sink += 700;
            }
            break;
    }
    
    /* Use sink to prevent dead code elimination */
    return result + (sink & 1);
}

/* Dummy function with side effects */
static __attribute__((noinline)) 
void dummy_side_effect(int *counter) {
    *counter += 1;
}

int main(void) {
    /* Initialize test values including NaNs */
    double nan_dbl = 0.0 / 0.0;          /* Generate NaN */
    double inf_dbl = __builtin_inf();    /* Infinity */
    double neg_inf_dbl = -__builtin_inf();
    double normal_dbl = 3.14159;
    double zero_dbl = 0.0;
    
    /* Alternative NaN generation methods */
    double nan_dbl2 = __builtin_nan("");
    double nan_dbl3 = sqrt(-1.0);        /* Another NaN source */
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    int checksum = 0;
    int counter = 0;
    
    /* Test various combinations */
    for (int op = 0; op < 4; op++) {
        /* Test NaN vs normal */
        checksum += test_nan_comparisons(op, nan_dbl, normal_dbl, nan_flt, normal_flt);
        dummy_side_effect(&counter);
        
        /* Test NaN vs NaN */
        checksum += test_nan_comparisons(op, nan_dbl, nan_dbl2, nan_flt, nan_flt);
        dummy_side_effect(&counter);
        
        /* Test normal vs normal */
        checksum += test_nan_comparisons(op, normal_dbl, zero_dbl, normal_flt, zero_flt);
        dummy_side_effect(&counter);
        
        /* Test infinity vs NaN */
        checksum += test_nan_comparisons(op, inf_dbl, nan_dbl3, inf_flt, nan_flt);
        dummy_side_effect(&counter);
        
        /* Test infinity vs normal */
        checksum += test_nan_comparisons(op, inf_dbl, normal_dbl, inf_flt, normal_flt);
        dummy_side_effect(&counter);
        
        /* Test negative infinity vs NaN */
        checksum += test_nan_comparisons(op, neg_inf_dbl, nan_dbl, -inf_flt, nan_flt);
        dummy_side_effect(&counter);
    }
    
    /* Additional complex expressions */
    for (int i = 0; i < 10; i++) {
        /* Create expressions that might fold differently */
        double a = (i % 2) ? nan_dbl : normal_dbl;
        double b = (i % 3) ? inf_dbl : zero_dbl;
        float c = (i % 2) ? nan_flt : normal_flt;
        float d = (i % 3) ? inf_flt : zero_flt;
        
        checksum += test_nan_comparisons(i % 4, a, b, c, d);
        dummy_side_effect(&counter);
        
        /* Test with arithmetic that might produce NaN */
        double temp = a * b;
        float tempf = c * d;
        checksum += test_nan_comparisons((i + 1) % 4, temp, b, tempf, d);
        dummy_side_effect(&counter);
    }
    
    printf("Checksum: %d (counter: %d)\n", checksum, counter);
    return 0;
}
