/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test.c */

#include <stdio.h>

/* Prevent optimization */
static volatile int dummy = 0;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract(void) {
    printf("Test 1: unsigned short _Fract overflow\n");
    
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 ≈ 0.996 */
    unsigned short _Fract usf_max = 0.99609375ur;  /* 255/256 */
    unsigned short _Fract usf_overflow = 1.0ur;    /* This should trigger overflow */
    
    /* Try to assign value > maximum */
    unsigned short _Fract result = usf_overflow;
    
    /* Use result to prevent dead code elimination */
    dummy += (int)(result * 256);
}

/* Test 2: Overflow in signed short _Fract (i_f_bits = 7 for signed) */
void test_signed_short_fract(void) {
    printf("Test 2: signed short _Fract overflow\n");
    
    /* Maximum value for signed short _Fract is (2^7-1)/2^7 = 127/128 ≈ 0.992 */
    signed short _Fract ssf_max = 0.9921875r;      /* 127/128 */
    signed short _Fract ssf_overflow = 1.0r;       /* This should trigger overflow */
    
    signed short _Fract result = ssf_overflow;
    dummy += (int)(result * 128);
}

/* Test 3: Overflow from _Accum to _Fract conversion */
void test_accum_to_fract(void) {
    printf("Test 3: _Accum to _Fract conversion overflow\n");
    
    /* unsigned short _Accum has 8 integer bits, 8 fractional bits */
    /* unsigned short _Fract has 0 integer bits, 8 fractional bits */
    unsigned short _Accum usa = 256.0uhk;          /* Value too large for _Fract */
    unsigned short _Fract usf;
    
    /* Explicit cast that should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    
    dummy += (int)(usf * 256);
}

/* Test 4: Overflow in arithmetic operations */
void test_arithmetic_overflow(void) {
    printf("Test 4: Arithmetic operation overflow\n");
    
    unsigned _Fract f1 = 0.8ur;
    unsigned _Fract f2 = 0.9ur;
    
    /* Sum > 1.0, should trigger overflow */
    unsigned _Fract sum = f1 + f2;
    
    dummy += (int)(sum * 256);
}

/* Test 5: Overflow with different i_f_bits values (16 bits) */
void test_unsigned_fract_16bit(void) {
    printf("Test 5: unsigned _Fract (16-bit) overflow\n");
    
    /* unsigned _Fract has 16 fractional bits */
    /* Maximum is (2^16-1)/2^16 = 65535/65536 ≈ 0.99998 */
    unsigned _Fract uf_max = 0.99998ur;
    unsigned _Fract uf_overflow = 1.0ur;           /* Should trigger overflow */
    
    unsigned _Fract result = uf_overflow;
    dummy += (int)(result * 65536);
}

/* Test 6: Overflow with long accum types */
void test_long_accum_overflow(void) {
    printf("Test 6: long _Accum overflow test\n");
    
    /* unsigned long _Accum to unsigned long _Fract conversion */
    unsigned long _Accum ula = 65536.0ulrk;        /* Large value */
    unsigned long _Fract ulf;
    
    ulf = (unsigned long _Fract)ula;               /* Should trigger overflow */
    
    dummy += (int)(ulf * 65536);
}

/* Test 7: Test with saturation attribute */
void test_saturation(void) {
    printf("Test 7: Saturation behavior\n");
    
    /* With saturation, overflow should clamp to maximum */
    unsigned short _Fract __attribute__((saturated)) usf_sat;
    unsigned short _Accum usa = 2.0uhk;            /* > 1.0 */
    
    usf_sat = (unsigned short _Fract)usa;          /* Should saturate to max */
    
    dummy += (int)(usf_sat * 256);
}

/* Test 8: Explicit overflow check using builtins */
void test_builtin_overflow(void) {
    printf("Test 8: Builtin overflow check\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This might trigger the overflow checking path */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    if (overflow) {
        printf("  Overflow detected in addition\n");
    }
    
    dummy += overflow;
}

/* Test 9: Multiple conversions to trigger different i_f_bits */
void test_multiple_conversions(void) {
    printf("Test 9: Multiple conversions\n");
    
    /* Test various conversions that should trigger overflow */
    signed short _Accum ssa = 128.0hk;             /* > 127.0 */
    signed short _Fract ssf;
    
    /* This conversion should trigger overflow (high part > 0) */
    ssf = (signed short _Fract)ssa;
    
    dummy += (int)(ssf * 128);
    
    /* Test with zero high part but low part > max_s */
    unsigned _Accum ua = 1.9999urk;                /* Close to 2.0 */
    unsigned _Fract uf;
    
    uf = (unsigned _Fract)ua;                      /* Should trigger overflow */
    
    dummy += (int)(uf * 65536);
}

/* Test 10: Edge case - value exactly at boundary */
void test_boundary_case(void) {
    printf("Test 10: Boundary case\n");
    
    /* Create a value with high part = 0, low part = max_s + 1 */
    /* For unsigned short _Fract: max_s = 255, so we need low part = 256 */
    /* Represented as 256/256 = 1.0 */
    unsigned short _Fract boundary = 1.0ur;        /* Exactly at overflow boundary */
    
    unsigned short _Fract result = boundary;
    
    dummy += (int)(result * 256);
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_accum_to_fract();
    test_arithmetic_overflow();
    test_unsigned_fract_16bit();
    test_long_accum_overflow();
    test_saturation();
    test_builtin_overflow();
    test_multiple_conversions();
    test_boundary_case();
    
    printf("\nAll tests completed.\n");
    printf("Dummy value (to prevent optimization): %d\n", dummy);
    
    return 0;
}
