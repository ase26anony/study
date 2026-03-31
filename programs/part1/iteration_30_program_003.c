/* Test program to trigger uncovered bounds checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test test.c */

#include <stdio.h>

/* Helper to prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different fractional bit counts through various fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_8_fractional_bits(void) {
    printf("Test 1: 8 fractional bits (unsigned short _Fract)\n");
    
    /* max_s = 2^8 - 1 = 255 */
    /* Condition: a_high == 0 && a_low > 255 */
    
    /* Create value with low part > 255 but high part = 0 */
    unsigned short _Accum source = 256.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    KEEP(target);
    printf("  Conversion result: %u/256\n", (unsigned int)(target * 256));
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_16_fractional_bits(void) {
    printf("Test 2: 16 fractional bits (unsigned _Fract)\n");
    
    /* max_s = 2^16 - 1 = 65535 */
    /* Create value with low part > 65535 but high part = 0 */
    unsigned _Accum source = 65536.0uhk;  /* 65536 in 16.16 format */
    unsigned _Fract target;
    
    target = (unsigned _Fract)source;
    
    KEEP(target);
    printf("  Conversion result: %u/65536\n", (unsigned int)(target * 65536));
}

/* Test 3: i_f_bits = 8 with positive high part */
void test_positive_high_part(void) {
    printf("Test 3: Positive high part (signed short _Accum to signed short _Fract)\n");
    
    /* Condition: a_high > 0 */
    /* Create value with high part > 0 */
    signed short _Accum source = 256.0hk;  /* Positive value > what fits in _Fract */
    signed short _Fract target;
    
    target = (signed short _Fract)source;
    
    KEEP(target);
    printf("  Conversion result: %d/256\n", (int)(target * 256));
}

/* Test 4: i_f_bits = 24 (unsigned long _Fract) */
void test_24_fractional_bits(void) {
    printf("Test 4: 24 fractional bits (unsigned long _Fract)\n");
    
    /* max_s = 2^24 - 1 = 16777215 */
    unsigned long _Accum source = 16777216.0ulrk;  /* Just above max */
    unsigned long _Fract target;
    
    target = (unsigned long _Fract)source;
    
    KEEP(target);
    /* Note: multiplying by 2^24 might overflow 32-bit int */
    printf("  Conversion performed\n");
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 5: Builtin overflow detection\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This might use internal overflow checking */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    KEEP(result);
    KEEP(overflow);
    printf("  Overflow detected: %d\n", overflow);
}

/* Test 6: Saturated arithmetic */
void test_saturated_arithmetic(void) {
    printf("Test 6: Saturated arithmetic\n");
    
    /* With -fsaturated-arithmetic, overflow should saturate */
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum;
    
    sum = f1 + f2;  /* 1.8 > 1.0, should saturate to 1.0 */
    
    KEEP(sum);
    printf("  Saturated sum: %u/256\n", (unsigned int)(sum * 256));
}

/* Test 7: Mixed integer and fixed-point conversions */
void test_mixed_conversions(void) {
    printf("Test 7: Integer to fixed-point conversion\n");
    
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract f;
    
    f = (unsigned short _Fract)large_int;
    
    KEEP(f);
    printf("  Integer conversion result: %u/256\n", (unsigned int)(f * 256));
}

/* Test 8: Very small i_f_bits (edge case) */
void test_small_fractional_bits(void) {
    printf("Test 8: Small fractional bits (hypothetical 1-bit)\n");
    
    /* For i_f_bits = 1, max_s = 2^1 - 1 = 1 */
    /* Use a type with minimal precision if available */
    /* For demonstration, use explicit scaling */
    typedef _Fract __attribute__((scaling(1))) one_bit_fract;
    
    unsigned short _Accum source = 2.0uhk;  /* Exceeds max_s when converted */
    one_bit_fract target;
    
    target = (one_bit_fract)source;
    
    KEEP(target);
    printf("  1-bit conversion performed\n");
}

/* Test 9: Direct overflow through multiplication */
void test_multiplication_overflow(void) {
    printf("Test 9: Multiplication overflow\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 2.0ur;  /* Actually 2.0/256 in this type */
    
    /* This multiplication in wider intermediate type might overflow */
    unsigned _Accum product = (unsigned _Accum)f1 * (unsigned _Accum)f2;
    unsigned short _Fract result = (unsigned short _Fract)product;
    
    KEEP(result);
    printf("  Multiplication result: %u/256\n", (unsigned int)(result * 256));
}

int main(void) {
    int test_result = 0;
    
    printf("=== Testing fixed-point overflow conditions ===\n\n");
    
    /* Run all tests */
    test_8_fractional_bits();
    test_16_fractional_bits();
    test_positive_high_part();
    test_24_fractional_bits();
    test_builtin_overflow();
    test_saturated_arithmetic();
    test_mixed_conversions();
    test_small_fractional_bits();
    test_multiplication_overflow();
    
    printf("\n=== All tests completed ===\n");
    
    /* Return non-zero if any test failed (simplified) */
    return test_result;
}
