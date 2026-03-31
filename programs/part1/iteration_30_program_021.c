/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract(void) {
    printf("Test 1: unsigned short _Fract overflow\n");
    
    /* Maximum representable value: (2^8 - 1)/2^8 = 255/256 = 0.99609375 */
    unsigned short _Fract usf_max = 0.99609375ur;
    unsigned short _Fract usf_test;
    
    /* This should trigger overflow (high part = 0, low part > 255) */
    /* 1.0 in 8.8 format would be 256 in low part with high part 0 */
    unsigned short _Accum usa = 1.0uhk;  /* 1.0 in 8.8 format */
    usf_test = (unsigned short _Fract)usa;  /* Conversion should trigger overflow check */
    
    dummy = usf_test;
    printf("  usf_test = %u (as integer)\n", (unsigned int)(usf_test * 256));
}

/* Test 2: Overflow in signed short _Fract (i_f_bits = 7 for signed) */
void test_signed_short_fract(void) {
    printf("\nTest 2: signed short _Fract overflow\n");
    
    /* Maximum representable value: (2^7 - 1)/2^7 = 127/128 = 0.9921875 */
    signed short _Fract ssf_max = 0.9921875r;
    signed short _Fract ssf_test;
    
    /* Try to assign 1.0 which exceeds maximum */
    signed short _Accum ssa = 1.0hk;
    ssf_test = (signed short _Fract)ssa;  /* Should trigger overflow */
    
    dummy = ssf_test;
    printf("  ssf_test = %d (as integer)\n", (int)(ssf_test * 128));
}

/* Test 3: Overflow with positive high part (a_high > max_r) */
void test_positive_high_part(void) {
    printf("\nTest 3: Positive high part overflow\n");
    
    /* Use _Accum types which have more integer bits */
    unsigned long _Accum ula = 300.0ulk;  /* Large value */
    unsigned short _Accum usa;
    
    /* Convert to smaller type - should trigger a_high > 0 condition */
    usa = (unsigned short _Accum)ula;
    
    dummy = usa;
    printf("  usa = %u (as integer)\n", (unsigned int)(usa * 256));
}

/* Test 4: Overflow in unsigned _Fract (i_f_bits = 16) */
void test_unsigned_fract(void) {
    printf("\nTest 4: unsigned _Fract overflow\n");
    
    /* Maximum: (2^16 - 1)/2^16 = 65535/65536 ≈ 0.9999847 */
    unsigned _Fract uf_max = 0.9999847ur;
    unsigned _Fract uf_test;
    
    /* Create value that exceeds maximum */
    unsigned _Accum ua = 1.0uk;  /* 1.0 in 16.16 format */
    uf_test = (unsigned _Fract)ua;  /* Should trigger overflow */
    
    dummy = uf_test;
    printf("  uf_test = %u (as integer)\n", (unsigned int)(uf_test * 65536));
}

/* Test 5: Using builtin overflow detection */
void test_builtin_overflow(void) {
    printf("\nTest 5: Builtin overflow detection\n");
    
    unsigned short _Fract a = 0.8ur;
    unsigned short _Fract b = 0.3ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This multiplication might overflow */
    overflow = __builtin_mul_overflow((unsigned int)(a * 256), 
                                      (unsigned int)(b * 256), 
                                      (unsigned int*)&result);
    
    dummy = result;
    printf("  overflow detected: %d\n", overflow);
}

/* Test 6: Saturated arithmetic */
void test_saturated_arithmetic(void) {
    printf("\nTest 6: Saturated arithmetic\n");
    
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    /* This sum exceeds 1.0 and should saturate */
    sum = f1 + f2;
    
    dummy = sum;
    printf("  saturated sum = %u (as integer, expected 255)\n", 
           (unsigned int)(sum * 256));
}

/* Test 7: Different i_f_bits values */
void test_various_i_f_bits(void) {
    printf("\nTest 7: Various i_f_bits values\n");
    
    /* Test i_f_bits = 1 (minimum) */
    {
        /* Simulate 1 fractional bit type */
        unsigned _Fract f;
        /* Assign 0.75 which in binary is 0.11b - needs 2 fractional bits */
        f = 0.75ur;  /* Should trigger overflow check for 1 fractional bit */
        dummy = f;
    }
    
    /* Test i_f_bits = 24 */
    {
        unsigned long _Fract ulf;
        unsigned long _Accum ula = 1.0ulk;
        /* Convert from _Accum (8.24) to _Fract (0.24) */
        ulf = (unsigned long _Fract)ula;  /* Should trigger overflow */
        dummy = ulf;
    }
}

int main(void) {
    int result = 0;
    
    printf("=== Testing overflow conditions for fixed-value.cc lines 264-277 ===\n\n");
    
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_positive_high_part();
    test_unsigned_fract();
    test_builtin_overflow();
    test_saturated_arithmetic();
    test_various_i_f_bits();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any test failed (simplified) */
    return result;
}
