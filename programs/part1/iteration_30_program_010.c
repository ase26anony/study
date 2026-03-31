/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int prevent_opt = 0;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract(void) {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* Maximum value for unsigned short _Fract is (2^8-1)/2^8 = 255/256 ≈ 0.996 */
    unsigned short _Fract usf_max = 0.99609375ur;  /* 255/256 */
    unsigned short _Fract usf_test;
    
    /* This should trigger overflow (value > max) */
    usf_test = 1.0ur;  /* 256/256, exceeds max by 1/256 */
    
    /* Use the value to prevent dead code elimination */
    prevent_opt = *(unsigned int*)&usf_test;
    
    printf("  Max value: %u/256 = %f\n", 
           (unsigned)(usf_max * 256), (double)usf_max);
    printf("  Test value: %u/256 = %f\n", 
           (unsigned)(usf_test * 256), (double)usf_test);
}

/* Test 2: Overflow in signed short _Fract (i_f_bits = 7 for signed) */
void test_signed_short_fract(void) {
    printf("\nTest 2: Signed short _Fract overflow\n");
    
    /* Maximum value for signed short _Fract is (2^7-1)/2^7 = 127/128 ≈ 0.992 */
    signed short _Fract ssf_max = 0.9921875r;  /* 127/128 */
    signed short _Fract ssf_test;
    
    /* This should trigger overflow (value > max) */
    ssf_test = 1.0r;  /* 128/128, exceeds max by 1/128 */
    
    prevent_opt = *(unsigned int*)&ssf_test;
    
    printf("  Max value: %d/128 = %f\n", 
           (int)(ssf_max * 128), (double)ssf_max);
    printf("  Test value: %d/128 = %f\n", 
           (int)(ssf_test * 128), (double)ssf_test);
}

/* Test 3: Overflow from _Accum to _Fract conversion */
void test_accum_to_fract(void) {
    printf("\nTest 3: _Accum to _Fract conversion overflow\n");
    
    /* unsigned short _Accum has 8 integer bits, 8 fractional bits */
    unsigned short _Accum usa = 256.0uhk;  /* Exactly at overflow boundary */
    unsigned short _Fract usf;
    
    /* This conversion should trigger overflow checking */
    usf = (unsigned short _Fract)usa;
    
    prevent_opt = *(unsigned int*)&usf;
    
    printf("  _Accum value: %f\n", (double)usa);
    printf("  Converted to _Fract: %f\n", (double)usf);
}

/* Test 4: Overflow in arithmetic operations */
void test_arithmetic_overflow(void) {
    printf("\nTest 4: Arithmetic operation overflow\n");
    
    unsigned _Fract f1 = 0.8ur;
    unsigned _Fract f2 = 0.3ur;
    unsigned _Fract sum;
    
    /* This addition might trigger overflow checking */
    sum = f1 + f2;  /* 0.8 + 0.3 = 1.1 > 1.0 */
    
    prevent_opt = *(unsigned int*)&sum;
    
    printf("  f1 = %f, f2 = %f\n", (double)f1, (double)f2);
    printf("  f1 + f2 = %f\n", (double)sum);
}

/* Test 5: Explicit overflow using builtins */
void test_builtin_overflow(void) {
    printf("\nTest 5: Builtin overflow detection\n");
    
    unsigned short _Fract a = 0.9ur;
    unsigned short _Fract b = 0.2ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use __builtin_add_overflow for fixed-point */
    overflow = __builtin_add_overflow(a, b, &result);
    
    prevent_opt = overflow;
    
    printf("  Builtin add overflow detection: %s\n", 
           overflow ? "OVERFLOW" : "no overflow");
    printf("  Result: %f\n", (double)result);
}

/* Test 6: Different i_f_bits values */
void test_various_i_f_bits(void) {
    printf("\nTest 6: Various i_f_bits values\n");
    
    /* Test unsigned _Fract (i_f_bits = 16) */
    unsigned _Fract uf = 1.0ur;  /* Should overflow */
    prevent_opt = *(unsigned int*)&uf;
    printf("  unsigned _Fract (16 bits): %f\n", (double)uf);
    
    /* Test unsigned long _Fract (i_f_bits = 32) */
    unsigned long _Fract ulf = 1.0ulr;  /* Should overflow */
    prevent_opt = *(unsigned int*)&ulf;
    printf("  unsigned long _Fract (32 bits): %f\n", (double)ulf);
    
    /* Test unsigned short _Accum (i_f_bits = 8) */
    unsigned short _Accum usa = 256.0uhk;  /* Boundary case */
    prevent_opt = *(unsigned int*)&usa;
    printf("  unsigned short _Accum (8 bits): %f\n", (double)usa);
}

/* Test 7: Saturation behavior */
void test_saturation(void) {
    printf("\nTest 7: Saturation behavior\n");
    
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract usf;
    
    /* This should saturate to maximum value */
    usf = 1.5ur;  /* > 1.0 */
    
    prevent_opt = *(unsigned int*)&usf;
    
    printf("  Attempt to assign 1.5 to unsigned short _Fract\n");
    printf("  Result (should be ~0.996): %f\n", (double)usf);
}

/* Test 8: Complex expression that might overflow */
void test_complex_expression(void) {
    printf("\nTest 8: Complex expression overflow\n");
    
    unsigned short _Fract a = 0.7ur;
    unsigned short _Fract b = 0.6ur;
    unsigned short _Fract c = 0.5ur;
    unsigned short _Fract result;
    
    /* (0.7 + 0.6) * 0.5 = 1.3 * 0.5 = 0.65 (no overflow) */
    /* But compiler might check intermediate results */
    result = (a + b) * c;
    
    prevent_opt = *(unsigned int*)&result;
    
    printf("  (0.7 + 0.6) * 0.5 = %f\n", (double)result);
}

int main(void) {
    int test_result = 0;
    
    printf("=== Testing overflow conditions for fixed-value.cc lines 264-277 ===\n\n");
    
    /* Run all tests */
    test_unsigned_short_fract();
    test_signed_short_fract();
    test_accum_to_fract();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_various_i_f_bits();
    test_saturation();
    test_complex_expression();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any overflow was expected but not handled */
    /* In practice, we'd check actual results vs expected */
    return test_result;
}
