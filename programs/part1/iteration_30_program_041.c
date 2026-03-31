/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed test_fixed.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow in low part with zero high part (a_high == max_r && a_low > max_s) */
void test_low_part_overflow() {
    printf("Test 1: Low part overflow with zero high part\n");
    
    /* For i_f_bits = 8, max_s = 2^8 - 1 = 255 */
    /* We need a value where high part = 0, low part > 255 */
    
    /* Using unsigned short _Accum with 8 fractional bits */
    unsigned short _Accum usa = 256.0uhk;  /* 256 = 256 * 2^8 / 2^8 = 65536 in internal representation */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow check */
    usf = (unsigned short _Fract)usa;
    
    /* Prevent dead code elimination */
    dummy = (int)usf;
    printf("  Result: %u\n", (unsigned int)usf);
}

/* Test 2: Positive high part (a_high > max_r) */
void test_positive_high_part() {
    printf("Test 2: Positive high part\n");
    
    /* We need a value where high part > 0 */
    /* Using long _Accum to get more integer bits */
    long _Accum la = 65536.0lk;  /* Large value that will have positive high part */
    short _Fract sf;
    
    /* This conversion should trigger overflow check with positive high part */
    sf = (short _Fract)la;
    
    dummy = (int)sf;
    printf("  Result: %d\n", (int)sf);
}

/* Test 3: Different i_f_bits values */
void test_various_fractional_bits() {
    printf("Test 3: Various fractional bits\n");
    
    /* Test with i_f_bits = 1 */
    {
        unsigned _Fract uf;
        /* For i_f_bits = 1, max_s = 2^1 - 1 = 1 */
        /* Need value > 1 in internal representation */
        unsigned _Accum ua = 2.0uk;  /* 2.0 = 2 * 2^1 / 2^1 = 4 in internal rep */
        uf = (unsigned _Fract)ua;
        dummy = (int)uf;
        printf("  i_f_bits=1: %u\n", (unsigned int)uf);
    }
    
    /* Test with i_f_bits = 16 */
    {
        unsigned long _Fract ulf;
        /* For i_f_bits = 16, max_s = 65535 */
        unsigned long _Accum ula = 65536.0ulk;  /* Exceeds max_s */
        ulf = (unsigned long _Fract)ula;
        dummy = (int)ulf;
        printf("  i_f_bits=16: %u\n", (unsigned int)ulf);
    }
}

/* Test 4: Signed types with overflow */
void test_signed_overflow() {
    printf("Test 4: Signed types overflow\n");
    
    /* Signed overflow in positive direction */
    short _Accum sa = 128.0hk;  /* Maximum for short _Fract is 127/128 */
    short _Fract sf;
    
    sf = (short _Fract)sa;  /* Should trigger overflow */
    dummy = (int)sf;
    printf("  Signed positive overflow: %d\n", (int)sf);
    
    /* Test with builtin overflow detection */
    short _Fract sf1 = 0.9hr;
    short _Fract sf2 = 0.9hr;
    short _Fract result;
    int overflow = __builtin_mul_overflow(sf1, sf2, &result);
    printf("  Builtin mul overflow detected: %d\n", overflow);
}

/* Test 5: Arithmetic operations causing overflow */
void test_arithmetic_overflow() {
    printf("Test 5: Arithmetic operations\n");
    
    /* Addition causing overflow */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    sum = f1 + f2;  /* 0.8 + 0.8 = 1.6 > 1.0, should overflow */
    dummy = (int)sum;
    printf("  Addition overflow result: %u\n", (unsigned int)sum);
    
    /* Multiplication causing overflow */
    unsigned _Fract m1 = 0.9ur;
    unsigned _Fract m2 = 0.9ur;
    unsigned _Fract prod;
    
    prod = m1 * m2;  /* 0.9 * 0.9 = 0.81 < 1.0, no overflow */
    dummy = (int)prod;
    printf("  Multiplication result: %u\n", (unsigned int)prod);
}

/* Test 6: Explicit casts with different bit patterns */
void test_explicit_casts() {
    printf("Test 6: Explicit casts\n");
    
    /* Create a value that definitely has high part > 0 */
    long long value = 0x100000000LL;  /* High part = 1, low part = 0 */
    
    /* Cast through various types to trigger conversion */
    long _Accum la = (long _Accum)value;
    _Fract f;
    
    /* This should trigger a_high.sgt(max_r) since high part > 0 */
    f = (_Fract)la;
    
    dummy = (int)f;
    printf("  Large value cast result: %d\n", (int)f);
}

/* Test 7: Using volatile to prevent constant folding */
void test_volatile_conversions() {
    printf("Test 7: Volatile conversions\n");
    
    volatile unsigned short _Accum v_usa = 300.0uhk;
    volatile unsigned short _Fract v_usf;
    
    /* Force runtime conversion */
    v_usf = (unsigned short _Fract)v_usa;
    
    printf("  Volatile conversion: %u\n", (unsigned int)v_usf);
}

/* Test 8: Nested conversions */
void test_nested_conversions() {
    printf("Test 8: Nested conversions\n");
    
    /* Chain of conversions that may overflow */
    long _Accum la = 1000.0lk;
    short _Accum sa = (short _Accum)la;
    short _Fract sf = (short _Fract)sa;
    
    dummy = (int)sf;
    printf("  Nested conversion result: %d\n", (int)sf);
}

int main() {
    int result = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_low_part_overflow();
    test_positive_high_part();
    test_various_fractional_bits();
    test_signed_overflow();
    test_arithmetic_overflow();
    test_explicit_casts();
    test_volatile_conversions();
    test_nested_conversions();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any test failed (simplified check) */
    return result;
}
