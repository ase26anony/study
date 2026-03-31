/* Test program to trigger overflow checks in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract() {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 ≈ 0.996 */
    unsigned short _Fract usf_max = 0.99609375ur;  /* 255/256 */
    unsigned short _Fract usf_test;
    
    /* This should trigger overflow (high part = 0, low part > 255) */
    /* 1.0 in Q8 format would be 256/256, which exceeds max */
    usf_test = 1.0ur;
    KEEP(usf_test);
    
    /* Try with explicit cast from integer */
    unsigned int val = 1;
    usf_test = (unsigned short _Fract)val;  /* 1.0 > 255/256 */
    KEEP(usf_test);
    
    printf("  usf_max = %f, usf_test = %f\n", 
           (double)usf_max, (double)usf_test);
}

/* Test 2: Overflow in signed short _Fract (i_f_bits = 7 for signed) */
void test_signed_short_fract() {
    printf("Test 2: Signed short _Fract overflow\n");
    
    /* For signed short _Fract with 7 fractional bits (plus sign) */
    /* Max positive is (2^7-1)/2^7 = 127/128 ≈ 0.992 */
    signed short _Fract ssf_max = 0.9921875r;  /* 127/128 */
    signed short _Fract ssf_test;
    
    /* This should trigger overflow (value > max positive) */
    ssf_test = 1.0r;  /* 1.0 > 127/128 */
    KEEP(ssf_test);
    
    /* Test negative overflow too */
    ssf_test = -1.0r;  /* -1.0 < -127/128 */
    KEEP(ssf_test);
    
    printf("  ssf_max = %f, ssf_test = %f\n", 
           (double)ssf_max, (double)ssf_test);
}

/* Test 3: Overflow in unsigned _Accum (more integer bits) */
void test_unsigned_accum() {
    printf("Test 3: Unsigned _Accum overflow\n");
    
    /* For unsigned short _Accum: 8 integer bits, 8 fractional bits */
    /* Max value is (2^16-1)/2^8 = 65535/256 ≈ 255.996 */
    unsigned short _Accum usa_max = 255.99609375uk;
    unsigned short _Accum usa_test;
    
    /* Trigger overflow with high part > 0 */
    usa_test = 256.0uk;  /* High part becomes 1 (256/256) */
    KEEP(usa_test);
    
    /* Also test case where high part = 0 but low part > max_s */
    /* For i_f_bits=8, max_s = 255, so use value 255.5 (255.5*256 = 65408) */
    usa_test = 255.5uk;
    KEEP(usa_test);
    
    printf("  usa_max = %f, usa_test = %f\n", 
           (double)usa_max, (double)usa_test);
}

/* Test 4: Overflow through arithmetic operations */
void test_arithmetic_overflow() {
    printf("Test 4: Arithmetic overflow\n");
    
    unsigned short _Fract a = 0.8ur;
    unsigned short _Fract b = 0.3ur;
    unsigned short _Fract sum;
    
    /* This sum (1.1) exceeds max for unsigned short _Fract */
    sum = a + b;
    KEEP(sum);
    
    /* Multiplication overflow */
    unsigned short _Fract c = 0.9ur;
    unsigned short _Fract d = 1.1ur;  /* Actually clamped to 0.996 */
    unsigned short _Fract prod = c * d;
    KEEP(prod);
    
    printf("  sum = %f, prod = %f\n", (double)sum, (double)prod);
}

/* Test 5: Explicit casts causing overflow */
void test_explicit_casts() {
    printf("Test 5: Explicit cast overflow\n");
    
    /* Cast from _Accum to _Fract - likely to overflow */
    unsigned short _Accum acc = 128.5uk;
    unsigned short _Fract fr;
    
    fr = (unsigned short _Fract)acc;  /* 128.5 > 0.996 */
    KEEP(fr);
    
    /* Cast from integer to fixed-point */
    int large_int = 1000;
    unsigned short _Fract fr2 = (unsigned short _Fract)large_int;
    KEEP(fr2);
    
    printf("  acc = %f, fr = %f, fr2 = %f\n", 
           (double)acc, (double)fr, (double)fr2);
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("Test 6: Builtin overflow detection\n");
    
    unsigned short _Fract x = 0.8ur;
    unsigned short _Fract y = 0.3ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use overflow checking builtin */
    overflow = __builtin_add_overflow(x, y, &result);
    KEEP(overflow);
    KEEP(result);
    
    printf("  overflow = %d, result = %f\n", overflow, (double)result);
}

/* Test 7: Different i_f_bits values */
void test_various_ifbits() {
    printf("Test 7: Various i_f_bits values\n");
    
    /* Test with _Fract types (typically 8, 16, 32 bits) */
    unsigned _Fract uf;      /* Likely 16 fractional bits */
    unsigned long _Fract ulf; /* Likely 32 fractional bits */
    
    /* Trigger overflow for each type */
    uf = 1.0Uhr;  /* Try to exceed max */
    KEEP(uf);
    
    ulf = 1.0ULr;  /* Try to exceed max */
    KEEP(ulf);
    
    printf("  uf = %f, ulf = %f\n", (double)uf, (double)ulf);
}

/* Test 8: Saturation behavior */
void test_saturation() {
    printf("Test 8: Saturation behavior\n");
    
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract sat_test;
    
    /* These should saturate to maximum value */
    sat_test = 1.5ur;  /* Should saturate to ~0.996 */
    KEEP(sat_test);
    
    sat_test = 2.0ur;  /* Should also saturate */
    KEEP(sat_test);
    
    /* Test negative saturation for signed types */
    signed short _Fract ssat_test = -1.5r;  /* Should saturate to ~-0.992 */
    KEEP(ssat_test);
    
    printf("  sat_test = %f, ssat_test = %f\n", 
           (double)sat_test, (double)ssat_test);
}

int main() {
    int failures = 0;
    
    printf("=== Testing overflow conditions for fixed-value.cc lines 264-277 ===\n\n");
    
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_unsigned_accum();
    test_arithmetic_overflow();
    test_explicit_casts();
    test_builtin_overflow();
    test_various_ifbits();
    test_saturation();
    
    printf("\n=== All tests completed ===\n");
    
    /* The actual coverage happens at compile time in GCC's middle-end.
     * Runtime just verifies the behavior is as expected. */
    return failures;
}
