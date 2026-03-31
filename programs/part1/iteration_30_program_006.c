/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed_overflow test_fixed_overflow.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow in unsigned short _Fract (8 fractional bits) */
void test_ushort_fract_overflow(void) {
    printf("Test 1: Unsigned short _Fract overflow (i_f_bits=8)\n");
    
    /* For unsigned short _Fract: i_f_bits = 8, max value = (2^8-1)/2^8 = 255/256 */
    /* We need a_high > 0 OR (a_high == 0 AND a_low > 255) */
    
    /* Case 1: a_high == 0, a_low > 255 */
    unsigned short _Accum usa1 = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract usf1 = (unsigned short _Fract)usa1;  /* Should overflow */
    
    /* Case 2: a_high > 0 */
    unsigned short _Accum usa2 = 256.0uhk;  /* 256.0, high part will be > 0 */
    unsigned short _Fract usf2 = (unsigned short _Fract)usa2;  /* Definitely overflow */
    
    dummy = (int)usf1 + (int)usf2;
}

/* Test 2: Overflow in unsigned _Fract (16 fractional bits) */
void test_uint_fract_overflow(void) {
    printf("Test 2: Unsigned _Fract overflow (i_f_bits=16)\n");
    
    /* For unsigned _Fract: i_f_bits = 16, max value = (2^16-1)/2^16 */
    /* We need a_high > 0 OR (a_high == 0 AND a_low > 65535) */
    
    /* Case 1: a_high == 0, a_low > 65535 */
    unsigned _Accum ua1 = 1.0uk;  /* 1.0 in unsigned _Accum */
    unsigned _Fract uf1 = (unsigned _Fract)ua1;  /* Should overflow */
    
    /* Case 2: a_high > 0 */
    unsigned _Accum ua2 = 65536.0uk;  /* 65536.0, high part will be > 0 */
    unsigned _Fract uf2 = (unsigned _Fract)ua2;  /* Definitely overflow */
    
    dummy += (int)uf1 + (int)uf2;
}

/* Test 3: Overflow in signed short _Fract (7 fractional bits for signed) */
void test_sshort_fract_overflow(void) {
    printf("Test 3: Signed short _Fract overflow (i_f_bits=7)\n");
    
    /* For signed short _Fract: i_f_bits = 7, max value = (2^7-1)/2^7 = 127/128 */
    /* We need a_high > 0 OR (a_high == 0 AND a_low > 127) */
    
    /* Case 1: a_high == 0, a_low > 127 */
    short _Accum sa1 = 1.0hk;  /* 1.0 in short _Accum */
    short _Fract sf1 = (short _Fract)sa1;  /* Should overflow */
    
    /* Case 2: a_high > 0 */
    short _Accum sa2 = 128.0hk;  /* 128.0, high part will be > 0 */
    short _Fract sf2 = (short _Fract)sa2;  /* Definitely overflow */
    
    dummy += (int)sf1 + (int)sf2;
}

/* Test 4: Overflow in signed _Fract (15 fractional bits for signed) */
void test_sint_fract_overflow(void) {
    printf("Test 4: Signed _Fract overflow (i_f_bits=15)\n");
    
    /* For signed _Fract: i_f_bits = 15, max value = (2^15-1)/2^15 = 32767/32768 */
    
    /* Case 1: a_high == 0, a_low > 32767 */
    _Accum a1 = 1.0k;  /* 1.0 in _Accum */
    _Fract f1 = (_Fract)a1;  /* Should overflow */
    
    /* Case 2: a_high > 0 */
    _Accum a2 = 32768.0k;  /* 32768.0, high part will be > 0 */
    _Fract f2 = (_Fract)a2;  /* Definitely overflow */
    
    dummy += (int)f1 + (int)f2;
}

/* Test 5: Using builtins to force overflow checking */
void test_builtin_overflow(void) {
    printf("Test 5: Using builtin overflow checks\n");
    
    /* Use __builtin_add_overflow with fixed-point types */
    unsigned short _Fract usf1 = 0.8ur;
    unsigned short _Fract usf2 = 0.8ur;
    unsigned short _Fract usf_result;
    int overflow;
    
    /* This addition should overflow for unsigned short _Fract */
    overflow = __builtin_add_overflow(usf1, usf2, &usf_result);
    
    /* Use __builtin_mul_overflow */
    unsigned short _Fract usf3 = 0.9ur;
    unsigned short _Fract usf4 = 0.9ur;
    
    overflow += __builtin_mul_overflow(usf3, usf4, &usf_result);
    
    dummy += overflow;
}

/* Test 6: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    printf("Test 6: Arithmetic operations causing overflow\n");
    
    /* Addition overflow */
    unsigned _Fract uf1 = 0.7ur;
    unsigned _Fract uf2 = 0.6ur;
    unsigned _Fract sum = uf1 + uf2;  /* 1.3 > 1.0, should overflow */
    
    /* Multiplication overflow */
    unsigned _Fract uf3 = 0.9ur;
    unsigned _Fract uf4 = 0.9ur;
    unsigned _Fract prod = uf3 * uf4;  /* 0.81 < 1.0, but check still happens */
    
    /* Chain operations */
    unsigned _Fract chain = 0.5ur;
    for (int i = 0; i < 3; i++) {
        chain = chain + 0.3ur;  /* Eventually exceeds 1.0 */
    }
    
    dummy += (int)sum + (int)prod + (int)chain;
}

/* Test 7: Different i_f_bits values with long types */
void test_long_types(void) {
    printf("Test 7: Long fixed-point types\n");
    
    /* unsigned long _Fract: typically 32 fractional bits */
    unsigned long _Accum ula = 1.0ulk;
    unsigned long _Fract ulf = (unsigned long _Fract)ula;  /* Should overflow */
    
    /* signed long _Fract: typically 31 fractional bits */
    long _Accum sla = 1.0lk;
    long _Fract slf = (long _Fract)sla;  /* Should overflow */
    
    dummy += (int)(ulf != 0) + (int)(slf != 0);
}

/* Test 8: Saturated arithmetic (should use different code path but still check bounds) */
void test_saturated_arithmetic(void) {
    printf("Test 8: Saturated arithmetic\n");
    
    /* With -fsaturated-arithmetic, these should saturate instead of overflow */
    unsigned short _Fract usf1 = 0.9ur;
    unsigned short _Fract usf2 = 0.9ur;
    unsigned short _Fract sum_sat = usf1 + usf2;  /* Should saturate to max */
    
    /* Overflow in conversion with saturation */
    unsigned short _Accum usa = 2.0uhk;
    unsigned short _Fract usf_conv = (unsigned short _Fract)usa;  /* Should saturate */
    
    dummy += (int)(sum_sat * 1000) + (int)(usf_conv * 1000);
}

/* Test 9: Mixed integer and fixed-point conversions */
void test_mixed_conversions(void) {
    printf("Test 9: Mixed integer/fixed-point conversions\n");
    
    /* Integer to fixed-point that overflows */
    unsigned int ui = 2;
    unsigned _Fract uf_from_int = (unsigned _Fract)ui;  /* 2 > 1.0, should overflow */
    
    /* Fixed-point to integer that could overflow */
    unsigned _Fract uf_large = 0.999ur;
    unsigned int ui_from_fract = (unsigned int)uf_large;
    
    dummy += ui_from_fract;
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_ushort_fract_overflow();
    test_uint_fract_overflow();
    test_sshort_fract_overflow();
    test_sint_fract_overflow();
    test_builtin_overflow();
    test_arithmetic_overflow();
    test_long_types();
    test_saturated_arithmetic();
    test_mixed_conversions();
    
    printf("\nAll tests completed. dummy = %d\n", dummy);
    
    /* Return non-zero if any test failed (simplified check) */
    return dummy == 0 ? 0 : 1;
}
