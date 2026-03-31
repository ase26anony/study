/* test_nan_comparisons.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent inlining to ensure comparisons reach fold-const pass */
static __attribute__((noinline)) 
int test_nan_comparisons(int opcode, double a, double b, float c, float d) {
    volatile int side_effect = 0;
    int result = 0;
    
    /* Complex expression with side effects to prevent early optimization */
    side_effect += (int)(a * 1000.0);
    
    /* Switch-like structure for different comparison types */
    switch (opcode) {
        case 0: /* UNEQ_EXPR-like behavior */
            /* unordered or equal: !(a < b) && !(b < a) || isunordered(a, b) */
            if (__builtin_isunordered(a, b) || 
                (!__builtin_isless(a, b) && !__builtin_isless(b, a))) {
                result = 1;
                side_effect++;
            }
            /* Also test with floats */
            if (__builtin_isunordered(c, d) || 
                (!__builtin_isless(c, d) && !__builtin_isless(d, c))) {
                result |= 2;
                side_effect--;
            }
            break;
            
        case 1: /* LTGT_EXPR-like behavior */
            /* ordered and not equal: !isunordered(a, b) && (a < b || b < a) */
            if (!__builtin_isunordered(a, b) && 
                (__builtin_isless(a, b) || __builtin_isless(b, a))) {
                result = 1;
                side_effect += 2;
            }
            /* Also test with floats */
            if (!__builtin_isunordered(c, d) && 
                (__builtin_isless(c, d) || __builtin_isless(d, c))) {
                result |= 2;
                side_effect -= 2;
            }
            break;
            
        case 2: /* Mixed comparisons to generate various tree codes */
            /* Direct use of __builtin_isnan */
            if (__builtin_isnan(a) || __builtin_isnan(b)) {
                result = 3;
                side_effect = 100;
            }
            /* Using standard NaN-aware macros */
            if (isgreater(a, b) && !__builtin_isnan(a) && !__builtin_isnan(b)) {
                result |= 4;
                side_effect++;
            }
            if (islessequal(c, d) || __builtin_isnan(c)) {
                result |= 8;
                side_effect--;
            }
            break;
            
        default:
            /* More complex expression mixing different comparisons */
            int temp = (__builtin_isunordered(a, b) ? 1 : 0);
            temp += (islessgreater(a, b) ? 2 : 0);
            temp += (__builtin_isnan(c) ? 4 : 0);
            temp += (islessgreater(c, d) ? 8 : 0);
            result = temp;
            side_effect = temp * 2;
            break;
    }
    
    /* Use side_effect to prevent dead code elimination */
    return result + (side_effect & 1);
}

/* Dummy function to create control flow complexity */
static __attribute__((noinline)) 
void dummy_call(int *counter) {
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
    double nan_dbl3 = sqrt(-1.0);        /* Another way to get NaN */
    
    float nan_flt = 0.0f / 0.0f;
    float inf_flt = __builtin_inff();
    float normal_flt = 2.71828f;
    float zero_flt = 0.0f;
    
    int checksum = 0;
    volatile int global_side_effect = 0;
    
    /* Test different combinations to hit various code paths */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            double a, b;
            float c, d;
            
            /* Select different value combinations */
            switch (j) {
                case 0: a = nan_dbl; b = normal_dbl; c = nan_flt; d = normal_flt; break;
                case 1: a = normal_dbl; b = nan_dbl2; c = normal_flt; d = nan_flt; break;
                case 2: a = nan_dbl; b = nan_dbl3; c = nan_flt; d = nan_flt; break;
                case 3: a = inf_dbl; b = normal_dbl; c = inf_flt; d = normal_flt; break;
                case 4: a = normal_dbl; b = neg_inf_dbl; c = normal_flt; d = -inf_flt; break;
                case 5: a = zero_dbl; b = normal_dbl; c = zero_flt; d = normal_flt; break;
                default: a = b = 0.0; c = d = 0.0f; break;
            }
            
            /* Call the comparison function with different opcodes */
            int res = test_nan_comparisons(i, a, b, c, d);
            checksum += res;
            
            /* Add side effects to prevent optimization */
            dummy_call(&global_side_effect);
            
            /* Additional complex expressions in main */
            if (__builtin_isunordered(a, b) && !__builtin_isnan(c)) {
                checksum += 7;
            }
            
            /* Test LTGT directly using islessgreater macro */
            if (islessgreater(a, b)) {
                checksum += 11;
                global_side_effect++;
            }
            
            /* Test UNEQ-like behavior */
            if (__builtin_isunordered(a, b) || a == b) {
                checksum += 13;
                global_side_effect--;
            }
        }
    }
    
    /* Final computation to use all results */
    checksum += global_side_effect;
    
    printf("Result checksum: %d\n", checksum);
    printf("Final side effect: %d\n", global_side_effect);
    
    return checksum != 0 ? 0 : 1;
}
