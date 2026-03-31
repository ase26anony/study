/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int use_result = 0;

/* Test different i_f_bits scenarios */
void test_unsigned_short_fract() {
    /* For unsigned short _Fract: i_f_bits = 8 (assuming 8 fractional bits) */
    /* max_s = (2^8 - 1) = 255, max_r = 0 */
    /* Condition triggers when: a_high > 0 OR (a_high == 0 AND a_low > 255) */
    
    unsigned short _Fract usf;
    unsigned short _Accum usa;
    
    /* Test 1: High part > 0 (a_high > max_r) */
    /* Convert 256.0 which has high part > 0 */
    usa = 256.0uhk;  /* 256.0 in unsigned short _Accum */
    usf = (unsigned short _Fract)usa;  /* This should trigger overflow check */
    
    /* Test 2: High part = 0 but low part > max_s */
    /* Convert 1.0 which is 256 in 8.8 fixed-point representation */
    usa = 1.0uhk;    /* 1.0 = 256 in 8.8 format */
    usf = (unsigned short _Fract)usa;  /* This should also trigger check */
    
    use_result += (unsigned int)usf;
}

void test_signed_short_fract() {
    /* For signed short _Fract: i_f_bits = 7 (1 sign bit, 7 fractional bits) */
    /* max_s = (2^7 - 1) = 127, max_r = 0 */
    
    signed short _Fract ssf;
    signed short _Accum ssa;
    
    /* Test positive overflow */
    ssa = 1.0hk;     /* 1.0 in signed short _Accum */
    ssf = (signed short _Fract)ssa;  /* Should trigger overflow */
    
    /* Test with larger value */
    ssa = 128.0hk;
    ssf = (signed short _Fract)ssa;  /* Definitely triggers a_high > 0 */
    
    use_result += (unsigned int)ssf;
}

void test_unsigned_fract() {
    /* For unsigned _Fract: i_f_bits = 16 (assuming 16 fractional bits) */
    /* max_s = (2^16 - 1) = 65535 */
    
    unsigned _Fract uf;
    unsigned _Accum ua;
    
    /* Test overflow with high part = 0, low part > 65535 */
    ua = 1.0uk;      /* 1.0 = 65536 in 16.16 format */
    uf = (unsigned _Fract)ua;  /* Should trigger (a_low > max_s) */
    
    /* Test with high part > 0 */
    ua = 65536.0uk;  /* High part definitely > 0 */
    uf = (unsigned _Fract)ua;
    
    use_result += (unsigned int)uf;
}

void test_signed_fract() {
    /* For signed _Fract: i_f_bits = 15 (1 sign bit, 15 fractional bits) */
    /* max_s = (2^15 - 1) = 32767 */
    
    signed _Fract sf;
    signed _Accum sa;
    
    /* Test positive overflow */
    sa = 1.0k;       /* 1.0 = 32768 in 16.16 format */
    sf = (signed _Fract)sa;  /* Should trigger overflow */
    
    use_result += (unsigned int)sf;
}

/* Test with builtins that might trigger overflow checking */
void test_builtin_overflow() {
    unsigned short _Fract f1, f2, result;
    
    f1 = 0.9ur;
    f2 = 0.9ur;
    
    /* This multiplication might overflow and trigger checking */
    result = f1 * f2;
    
    use_result += (unsigned int)result;
}

/* Test saturation behavior */
void test_saturation() {
    unsigned short _Fract usf;
    unsigned short _Accum usa;
    
    /* With saturation, overflow should clamp to maximum */
    usa = 256.0uhk;
    usf = (unsigned short _Fract)usa;  /* Should saturate to 0.996... */
    
    use_result += (unsigned int)usf;
}

/* Test integer to fixed-point conversions */
void test_int_to_fixed() {
    unsigned short _Fract usf;
    unsigned int ui;
    
    /* Integer to fixed-point conversion can also trigger overflow */
    ui = 256;  /* > 255 */
    usf = (unsigned short _Fract)ui;  /* Should trigger overflow check */
    
    use_result += (unsigned int)usf;
}

/* Test with different optimization levels */
void test_mixed_conversions() {
    /* Mix different types and conversions to hit various i_f_bits */
    
    /* 8 fractional bits */
    unsigned short _Fract usf1 = 0.5ur;
    unsigned short _Fract usf2 = 0.6ur;
    unsigned short _Fract sum1 = usf1 + usf2;  /* Might not overflow */
    
    /* Force overflow with explicit cast */
    unsigned _Accum ua = 1.5uk;
    unsigned short _Fract usf3 = (unsigned short _Fract)ua;  /* Should overflow */
    
    /* 16 fractional bits */
    unsigned _Fract uf1 = 0.999ur;
    unsigned _Fract uf2 = 0.999ur;
    unsigned _Fract sum2 = uf1 + uf2;  /* Might overflow */
    
    use_result += (unsigned int)sum1 + (unsigned int)usf3 + (unsigned int)sum2;
}

int main() {
    printf("Testing fixed-point overflow conditions...\n");
    
    /* Call all test functions to trigger different overflow scenarios */
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_unsigned_fract();
    test_signed_fract();
    test_builtin_overflow();
    test_saturation();
    test_int_to_fixed();
    test_mixed_conversions();
    
    printf("Test completed. use_result = %u\n", use_result);
    
    /* Return non-zero if any test might have failed (simplified check) */
    return use_result == 0 ? 0 : 1;
}
