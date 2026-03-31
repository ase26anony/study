/* Test program to trigger uncovered lines in fixed-value.cc
 * Lines 264-277: overflow checking for fixed-point conversions
 */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _x = (expr); \
    (void)_x; \
} while(0)

/* Test 1: Unsigned fract with 8 fractional bits (i_f_bits=8) */
void test_unsigned_fract_overflow() {
    printf("Test 1: Unsigned short _Fract (8 fractional bits)\n");
    
    /* Maximum representable: (2^8-1)/2^8 = 255/256 = 0.99609375 */
    unsigned short _Fract max_val = 0.99609375ur;
    printf("  Max value: %u/256 = %f\n", 255, (double)max_val);
    
    /* Try to assign 1.0 which exceeds maximum */
    unsigned short _Fract f1 = 1.0ur;  /* Should trigger overflow */
    printf("  Assign 1.0: %f\n", (double)f1);
    
    /* Try arithmetic that overflows */
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract f3 = 0.3ur;
    unsigned short _Fract sum = f2 + f3;  /* 1.1 > max */
    printf("  0.8 + 0.3 = %f\n", (double)sum);
}

/* Test 2: Signed fract with 7 fractional bits (i_f_bits=7 for signed) */
void test_signed_fract_overflow() {
    printf("\nTest 2: Signed short _Fract (7 fractional bits)\n");
    
    /* Maximum: (2^7-1)/2^7 = 127/128 = 0.9921875 */
    signed short _Fract max_val = 0.9921875r;
    printf("  Max positive: %d/128 = %f\n", 127, (double)max_val);
    
    /* Try to exceed positive maximum */
    signed short _Fract f1 = 1.0r;  /* Should trigger overflow */
    printf("  Assign 1.0: %f\n", (double)f1);
    
    /* Also test negative overflow */
    signed short _Fract f2 = -1.0r;  /* Minimum is -1.0, so this is OK */
    printf("  Assign -1.0: %f\n", (double)f2);
}

/* Test 3: Unsigned accum with 8 fractional bits (i_f_bits=8) */
void test_unsigned_accum_overflow() {
    printf("\nTest 3: Unsigned short _Accum (8 fractional bits)\n");
    
    /* Maximum: (2^(8+8)-1)/2^8 = 65535/256 = 255.99609375 */
    unsigned short _Accum max_val = 255.99609375uk;
    printf("  Max value: %f\n", (double)max_val);
    
    /* Try to exceed maximum */
    unsigned short _Accum a1 = 256.0uk;  /* Should trigger overflow */
    printf("  Assign 256.0: %f\n", (double)a1);
    
    /* Large multiplication that overflows */
    unsigned short _Accum a2 = 200.0uk;
    unsigned short _Accum a3 = 2.0uk;
    unsigned short _Accum prod = a2 * a3;  /* 400 > max */
    printf("  200.0 * 2.0 = %f\n", (double)prod);
}

/* Test 4: Conversion from larger to smaller type */
void test_conversion_overflow() {
    printf("\nTest 4: Conversion overflow scenarios\n");
    
    /* From _Accum to _Fract - guaranteed overflow for values >= 1.0 */
    unsigned short _Accum large_val = 2.5uk;
    unsigned short _Fract small_val = (unsigned short _Fract)large_val;
    printf("  Convert 2.5uk to _Fract: %f\n", (double)small_val);
    
    /* From integer to fixed-point */
    unsigned int int_val = 300;
    unsigned short _Accum accum_val = (unsigned short _Accum)int_val;
    printf("  Convert 300 to _Accum: %f\n", (double)accum_val);
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("\nTest 5: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.3ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use overflow checking builtin if available */
    #ifdef __has_builtin
    #if __has_builtin(__builtin_add_overflow)
    overflow = __builtin_add_overflow(f1, f2, &result);
    printf("  Builtin add overflow: %d, result: %f\n", overflow, (double)result);
    #endif
    #endif
    
    /* Alternative: Use saturating arithmetic attribute */
    unsigned short _Fract __attribute__((saturated)) s1 = 0.8ur;
    unsigned short _Fract __attribute__((saturated)) s2 = 0.3ur;
    unsigned short _Fract ssum = s1 + s2;
    printf("  Saturating add: 0.8 + 0.3 = %f\n", (double)ssum);
}

/* Test 6: Edge cases with different fractional bit counts */
void test_edge_cases() {
    printf("\nTest 6: Edge cases\n");
    
    /* Test with minimal fractional bits (i_f_bits=1) */
    unsigned _Fract f1 = 0.5ur;  /* Max for 1 bit: 0.5 */
    printf("  1-bit fract max (0.5): %f\n", (double)f1);
    
    /* Try to exceed it */
    unsigned _Fract f2 = 0.6ur;
    printf("  Assign 0.6 to 1-bit fract: %f\n", (double)f2);
    
    /* Test with many fractional bits */
    unsigned long _Fract f3 = 0.999999ur;  /* Close to 1.0 */
    printf("  Long fract near 1.0: %f\n", (double)f3);
}

/* Test 7: Specific case to trigger a_high > max_r (positive high part) */
void test_high_part_overflow() {
    printf("\nTest 7: High part overflow (a_high > 0)\n");
    
    /* Create a value where high part is positive */
    /* For unsigned short _Accum with 8 fractional bits, 
       values >= 256.0 will have positive high part */
    unsigned short _Accum large_val = 300.0uk;  /* High part > 0 */
    printf("  Large value (300.0): %f\n", (double)large_val);
    
    /* Convert to smaller type to force overflow check */
    unsigned short _Fract small_val = (unsigned short _Fract)large_val;
    printf("  Converted to _Fract: %f\n", (double)small_val);
    
    /* Another test with very large value */
    unsigned _Accum huge_val = 1000000.0uk;
    unsigned _Fract tiny_val = (unsigned _Fract)huge_val;
    printf("  Convert 1000000.0 to _Fract: %f\n", (double)tiny_val);
}

/* Test 8: Specific case to trigger a_high == 0 && a_low > max_s */
void test_low_part_overflow() {
    printf("\nTest 8: Low part overflow (a_high == 0, a_low > max_s)\n");
    
    /* For unsigned short _Fract with 8 fractional bits,
       max_s = 2^8 - 1 = 255. We need 0 < value < 1.0 but
       in the internal representation, low part > 255 */
    
    /* The value 1.0 has internal representation with high part 0
       and low part = 2^8 = 256 (exceeds max_s = 255) */
    unsigned short _Fract f1 = 1.0ur;
    printf("  Assign 1.0 to _Fract: %f\n", (double)f1);
    
    /* Value between max and 1.0: 255.5/256 = 0.998046875 */
    /* This should also trigger overflow since low part > 255 */
    unsigned short _Fract f2 = 0.998046875ur;
    printf("  Assign 0.998046875: %f\n", (double)f2);
}

int main() {
    printf("=== Testing fixed-point overflow conditions ===\n\n");
    
    /* Run all tests */
    test_unsigned_fract_overflow();
    test_signed_fract_overflow();
    test_unsigned_accum_overflow();
    test_conversion_overflow();
    test_builtin_overflow();
    test_edge_cases();
    test_high_part_overflow();
    test_low_part_overflow();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return success */
    return 0;
}
