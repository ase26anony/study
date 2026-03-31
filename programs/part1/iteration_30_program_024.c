/* Test program to trigger overflow checks in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed test_fixed.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Unsigned fract with 8 fractional bits (i_f_bits=8) */
void test_unsigned_fract_overflow() {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* For unsigned short _Fract: 8 fractional bits, max = (2^8-1)/2^8 = 255/256 */
    unsigned short _Fract usf_max = 0.99609375ur;  /* 255/256 */
    unsigned short _Fract usf_overflow = 1.0ur;    /* This should trigger overflow */
    
    /* The assignment itself should trigger the overflow check */
    unsigned short _Fract result = usf_overflow;
    
    dummy = (int)(result * 256);  /* Prevent dead code elimination */
    printf("  Result: %u/256\n", (unsigned)(result * 256));
}

/* Test 2: Signed fract with 7 fractional bits (i_f_bits=7 for signed) */
void test_signed_fract_overflow() {
    printf("Test 2: Signed short _Fract overflow\n");
    
    /* For signed short _Fract: 7 fractional bits + sign, max = (2^7-1)/2^7 = 127/128 */
    signed short _Fract ssf_max = 0.9921875r;      /* 127/128 */
    signed short _Fract ssf_overflow = 1.0r;       /* This should trigger overflow */
    
    signed short _Fract result = ssf_overflow;
    
    dummy = (int)(result * 128);
    printf("  Result: %d/128\n", (int)(result * 128));
}

/* Test 3: Conversion from _Accum to _Fract with different i_f_bits */
void test_accum_to_fract_overflow() {
    printf("Test 3: _Accum to _Fract conversion overflow\n");
    
    /* unsigned short _Accum has 8 integer + 8 fractional bits */
    /* unsigned short _Fract has 0 integer + 8 fractional bits */
    unsigned short _Accum usa = 256.0uhk;          /* 256.0, exceeds _Fract range */
    unsigned short _Fract usf;
    
    /* Explicit cast should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    
    dummy = (int)(usf * 256);
    printf("  _Accum value: %u.%u, _Fract result: %u/256\n", 
           (unsigned)(usa / 256), (unsigned)((usa % 256) * 100),
           (unsigned)(usf * 256));
}

/* Test 4: Arithmetic overflow in fixed-point operations */
void test_arithmetic_overflow() {
    printf("Test 4: Arithmetic operation overflow\n");
    
    unsigned _Fract f1 = 0.8ur;
    unsigned _Fract f2 = 0.3ur;
    
    /* Addition that exceeds 1.0 should trigger overflow */
    unsigned _Fract sum = f1 + f2;  /* 0.8 + 0.3 = 1.1 > 1.0 */
    
    dummy = (int)(sum * 256);
    printf("  Sum result: %u/256\n", (unsigned)(sum * 256));
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("Test 5: Builtin overflow detection\n");
    
    unsigned short _Fract a = 0.9ur;
    unsigned short _Fract b = 0.2ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin should trigger the overflow checking code */
    overflow = __builtin_mul_overflow(a, b, &result);
    
    dummy = overflow;
    printf("  Overflow detected: %d\n", overflow);
}

/* Test 6: Multiple i_f_bits values through different types */
void test_various_i_f_bits() {
    printf("Test 6: Various i_f_bits values\n");
    
    /* Test different fixed-point types to cover various i_f_bits */
    
    /* i_f_bits = 15 (signed short _Accum) */
    {
        signed short _Accum ssa = 32768.0hk;  /* Exceeds max for 7 integer bits */
        signed short _Fract ssf = (signed short _Fract)ssa;
        dummy = (int)(ssf * 128);
    }
    
    /* i_f_bits = 23 (signed _Accum) */
    {
        signed _Accum sa = 8388608.0k;  /* Large value */
        signed _Fract sf = (signed _Fract)sa;
        dummy = (int)(sf * 128);
    }
    
    /* i_f_bits = 31 (signed long _Accum) */
    {
        signed long _Accum sla = 2147483648.0lk;
        signed long _Fract slf = (signed long _Fract)sla;
        dummy = (int)(slf * 128);
    }
    
    printf("  Tested various fractional bit counts\n");
}

/* Test 7: Edge case - low part overflow with zero high part */
void test_low_part_overflow() {
    printf("Test 7: Low part overflow with zero high part\n");
    
    /* Create a value where high part is 0 but low part > 2^{i_f_bits} - 1 */
    /* For unsigned char _Fract (i_f_bits=8), max low part = 255 */
    
    /* Using intermediate integer to construct the value */
    unsigned int value = 256;  /* 256 > 255, high part = 0 */
    unsigned char _Fract ucf;
    
    /* Force conversion through integer */
    ucf = (unsigned char _Fract)(value / 256.0);
    
    dummy = (int)(ucf * 256);
    printf("  Result: %u/256\n", (unsigned)(ucf * 256));
}

/* Test 8: Positive high part case */
void test_positive_high_part() {
    printf("Test 8: Positive high part overflow\n");
    
    /* Create value with positive high part */
    /* For unsigned short _Fract, any value >= 1.0 has positive high part */
    
    unsigned short _Accum usa = 512.0uhk;  /* High part is 2 (512/256) */
    unsigned short _Fract usf = (unsigned short _Fract)usa;
    
    dummy = (int)(usf * 256);
    printf("  Result: %u/256\n", (unsigned)(usf * 256));
}

int main() {
    int failures = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_fract_overflow();
    test_signed_fract_overflow();
    test_accum_to_fract_overflow();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_various_i_f_bits();
    test_low_part_overflow();
    test_positive_high_part();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any test failed (simplified check) */
    return failures > 0 ? 1 : 0;
}
