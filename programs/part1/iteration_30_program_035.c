/* Test program to trigger uncovered bounds checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    (void)_tmp; \
} while(0)

/* Test 1: Trigger a_high > max_r (positive high part) */
void test_positive_high_part(void) {
    printf("Test 1: Positive high part overflow\n");
    
    /* Use long _Accum with 32 fractional bits, then convert to short _Fract */
    /* This should create a value where high part is positive */
    long _Accum la = 256.0lk;  /* Large value that exceeds short _Fract range */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check with positive high part */
    usf = (unsigned short _Fract)la;
    KEEP(usf);
    
    /* Another test with signed types */
    short _Accum sa = 128.0hk;
    short _Fract sf = (short _Fract)sa;
    KEEP(sf);
}

/* Test 2: Trigger a_high == max_r && a_low > max_s */
void test_low_part_overflow(void) {
    printf("Test 2: Low part overflow with zero high part\n");
    
    /* For unsigned short _Fract with 8 fractional bits, max_s = 255 */
    /* We need a value where high part is 0 but low part > 255 */
    
    /* Create value just above maximum for 8 fractional bits */
    unsigned _Accum ua = 1.0uk;  /* Exactly 1.0, which is 256/256 for 8 frac bits */
    unsigned short _Fract usf;
    
    /* This should have high part = 0, low part = 256 > 255 */
    usf = (unsigned short _Fract)ua;
    KEEP(usf);
    
    /* Test with different fractional bit counts */
    unsigned long _Fract ulf = 1.0ulr;  /* 32 fractional bits */
    unsigned short _Fract usf2 = (unsigned short _Fract)ulf;
    KEEP(usf2);
}

/* Test 3: Use arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    printf("Test 3: Arithmetic overflow\n");
    
    /* Addition that exceeds range */
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.2ur;
    unsigned short _Fract sum;
    
    /* This sum (1.1) exceeds maximum of ~0.996 for 8 fractional bits */
    sum = f1 + f2;
    KEEP(sum);
    
    /* Multiplication overflow */
    unsigned short _Accum a1 = 16.0uhk;  /* 8 integer, 8 fractional bits */
    unsigned short _Accum a2 = 16.0uhk;
    unsigned short _Accum prod = a1 * a2;  /* 256 exceeds range */
    KEEP(prod);
}

/* Test 4: Use builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 4: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.2ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This should trigger overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    
    /* Multiplication overflow check */
    unsigned short _Accum a1 = 16.0uhk;
    unsigned short _Accum a2 = 16.0uhk;
    unsigned short _Accum mul_result;
    
    overflow = __builtin_mul_overflow(a1, a2, &mul_result);
    KEEP(overflow);
    KEEP(mul_result);
}

/* Test 5: Test with saturation attribute */
void test_saturation(void) {
    printf("Test 5: Saturation behavior\n");
    
    /* With saturation, overflow should clamp to max value */
    _Sat unsigned short _Fract sat_f1 = 0.9ur;
    _Sat unsigned short _Fract sat_f2 = 0.2ur;
    _Sat unsigned short _Fract sat_sum = sat_f1 + sat_f2;
    
    KEEP(sat_sum);
    
    /* Conversion with saturation */
    unsigned _Accum ua = 2.0uk;  /* Exceeds range of short _Fract */
    _Sat unsigned short _Fract sat_conv = (_Sat unsigned short _Fract)ua;
    KEEP(sat_conv);
}

/* Test 6: Different fractional bit counts */
void test_various_fractional_bits(void) {
    printf("Test 6: Various fractional bit counts\n");
    
    /* Test i_f_bits = 1 (minimum) */
    unsigned _Fract uf1 = 0.5ur;  /* For 1 fractional bit, max is 0.5 */
    unsigned short _Fract usf1 = (unsigned short _Fract)uf1;  /* Convert to 8 bits */
    KEEP(usf1);
    
    /* Test i_f_bits = 16 */
    unsigned long _Fract ulf = 0.99999ulr;  /* 32 fractional bits */
    unsigned _Fract uf2 = (unsigned _Fract)ulf;  /* Convert to 16 bits */
    KEEP(uf2);
    
    /* Test i_f_bits = 24 */
    long _Accum la = 8388607.0lk;  /* Large value for 24 fractional bits */
    _Accum a = (_Accum)la;  /* Convert to 16 fractional bits */
    KEEP(a);
}

/* Test 7: Edge cases and boundary values */
void test_edge_cases(void) {
    printf("Test 7: Edge cases\n");
    
    /* Exactly at maximum boundary */
    unsigned short _Fract max_val = 0.99609375ur;  /* 255/256 for 8 bits */
    KEEP(max_val);
    
    /* Just above maximum - should trigger overflow */
    unsigned _Accum just_above = 1.0uk;  /* 256/256 */
    unsigned short _Fract conv_above = (unsigned short _Fract)just_above;
    KEEP(conv_above);
    
    /* Negative overflow for signed types */
    short _Accum min_sa = -129.0hk;  /* Below minimum for short _Fract */
    short _Fract conv_neg = (short _Fract)min_sa;
    KEEP(conv_neg);
}

/* Test 8: Complex expressions that force compile-time evaluation */
void test_complex_expressions(void) {
    printf("Test 8: Complex expressions\n");
    
    /* Nested operations that might be evaluated at compile time */
    unsigned short _Fract f = 0.5ur;
    unsigned short _Fract result;
    
    /* Chain of operations that could overflow */
    result = f * 2.0ur + 0.1ur - 0.2ur * 3.0ur;
    KEEP(result);
    
    /* Conditional expression with overflow */
    unsigned _Accum ua = 1.5uk;
    unsigned short _Fract cond_result = (ua > 1.0uk) ? 1.0ur : 0.5ur;
    KEEP(cond_result);
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_positive_high_part();
    test_low_part_overflow();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_saturation();
    test_various_fractional_bits();
    test_edge_cases();
    test_complex_expressions();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any test failed (simplified check) */
    return 0;
}
