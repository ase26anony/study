/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test.c */

#include <stdio.h>

/* Prevent optimization */
static volatile unsigned int dummy;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract(void)
{
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 */
    /* Try to assign 1.0 which exceeds the maximum */
    unsigned short _Fract usf1 = 0.5ur;
    unsigned short _Fract usf2 = 0.6ur;
    unsigned short _Fract usf_sum = usf1 + usf2;  /* 1.1 > 255/256 */
    
    dummy = *(unsigned int*)&usf_sum;
    
    /* Explicit overflow through cast */
    unsigned short _Accum usa = 1.0uhk;  /* 1.0 in _Accum format */
    unsigned short _Fract usf_cast = (unsigned short _Fract)usa;  /* Should trigger overflow */
    
    dummy += *(unsigned int*)&usf_cast;
}

/* Test 2: Overflow in signed short _Fract (i_f_bits = 7 for signed) */
void test_signed_short_fract(void)
{
    /* Maximum value for signed short _Fract is (2^7-1)/2^7 = 127/128 */
    signed short _Fract ssf1 = 0.7hr;
    signed short _Fract ssf2 = 0.6hr;
    signed short _Fract ssf_sum = ssf1 + ssf2;  /* 1.3 > 127/128 */
    
    dummy += *(unsigned int*)&ssf_sum;
    
    /* Test with negative overflow too */
    signed short _Fract ssf3 = -0.8hr;
    signed short _Fract ssf4 = -0.7hr;
    signed short _Fract ssf_sum2 = ssf3 + ssf4;  /* -1.5 < -1.0 */
    
    dummy += *(unsigned int*)&ssf_sum2;
}

/* Test 3: Overflow with different i_f_bits values (16 bits) */
void test_unsigned_fract(void)
{
    /* unsigned _Fract typically has 16 fractional bits */
    unsigned _Fract uf1 = 0.9999ur;
    unsigned _Fract uf2 = 0.0002ur;
    unsigned _Fract uf_sum = uf1 + uf2;  /* Should be > 1.0 */
    
    dummy += *(unsigned int*)&uf_sum;
    
    /* Cast from larger type */
    unsigned _Accum ua = 2.0uk;  /* 2.0 in _Accum */
    unsigned _Fract uf_cast = (unsigned _Fract)ua;  /* Should trigger overflow */
    
    dummy += *(unsigned int*)&uf_cast;
}

/* Test 4: Overflow with long accum (more fractional bits) */
void test_long_accum(void)
{
    /* Test conversion between different fixed-point types */
    long _Accum la = 1000.0lk;
    short _Accum sa = (short _Accum)la;  /* Potential overflow */
    
    dummy += *(unsigned int*)&sa;
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void)
{
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.3ur;
    unsigned short _Fract result;
    
    /* This builtin might trigger the overflow checking path */
    int overflow = __builtin_add_overflow(f1, f2, &result);
    
    dummy += overflow + *(unsigned int*)&result;
}

/* Test 6: Saturation mode (if supported) */
#ifdef __SAT_FRACT__
void test_saturation(void)
{
    /* With saturation, overflow should clamp to max value */
    _Sat unsigned short _Fract sat_usf1 = 0.9ur;
    _Sat unsigned short _Fract sat_usf2 = 0.2ur;
    _Sat unsigned short _Fract sat_sum = sat_usf1 + sat_usf2;  /* Should saturate to max */
    
    dummy += *(unsigned int*)&sat_sum;
    
    /* Test multiplication overflow */
    _Sat unsigned short _Fract sat_mul = sat_usf1 * 1.5ur;
    
    dummy += *(unsigned int*)&sat_mul;
}
#endif

/* Test 7: Direct integer to fixed-point conversion */
void test_int_conversion(void)
{
    /* Integer to fixed-point conversion can trigger overflow */
    int large_int = 256;  /* For 8 fractional bits, max is 255 */
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    
    dummy += *(unsigned int*)&from_int;
    
    /* Test with negative integer for signed types */
    int neg_int = -200;
    signed short _Fract from_neg_int = (signed short _Fract)neg_int;
    
    dummy += *(unsigned int*)&from_neg_int;
}

/* Test 8: Edge case - exactly at boundary */
void test_boundary(void)
{
    /* Test value exactly at max boundary */
    unsigned short _Fract max_val = 0.99609375ur;  /* 255/256 */
    
    /* Add a tiny amount to push over boundary */
    unsigned short _Fract tiny = 0.001ur;
    unsigned short _Fract boundary_sum = max_val + tiny;
    
    dummy += *(unsigned int*)&boundary_sum;
}

/* Test 9: Multiple operations that accumulate to overflow */
void test_accumulated_overflow(void)
{
    unsigned short _Fract val = 0.5ur;
    
    /* Repeated addition that should overflow */
    for (int i = 0; i < 3; i++) {
        val = val + 0.3ur;
    }
    
    dummy += *(unsigned int*)&val;
}

/* Test 10: Different i_f_bits through type combinations */
void test_mixed_types(void)
{
    /* Mix different fixed-point types */
    unsigned _Accum ua = 1.5uk;
    unsigned short _Fract usf = (unsigned short _Fract)ua;
    
    dummy += *(unsigned int*)&usf;
    
    /* Another combination */
    short _Accum sa = 100.0hk;
    unsigned _Fract uf = (unsigned _Fract)sa;
    
    dummy += *(unsigned int*)&uf;
}

int main(void)
{
    printf("Starting fixed-point overflow tests...\n");
    
    /* Run all tests to trigger various overflow scenarios */
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_unsigned_fract();
    test_long_accum();
    test_builtin_overflow();
    
    #ifdef __SAT_FRACT__
    test_saturation();
    #endif
    
    test_int_conversion();
    test_boundary();
    test_accumulated_overflow();
    test_mixed_types();
    
    printf("Tests completed. Dummy value: %u\n", dummy);
    printf("If compilation succeeded, the compiler should have executed\n");
    printf("the overflow checking code in fixed-value.cc lines 264-277.\n");
    
    return 0;
}
