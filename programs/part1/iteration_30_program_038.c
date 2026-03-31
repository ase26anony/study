/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test_fixed test_fixed.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    __asm__ volatile ("" : : "r" (_tmp)); \
} while(0)

/* Test 1: Unsigned fract with overflow in low part (a_high == 0, a_low > max_s) */
void test_unsigned_fract_overflow_low() {
    printf("Test 1: Unsigned fract overflow in low part\n");
    
    /* For unsigned short _Fract, i_f_bits = 8, max_s = 255 */
    /* We need a value where high part = 0, low part > 255 */
    
    /* Create a value that's just above maximum for unsigned short _Fract */
    unsigned short _Accum source = 256.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    KEEP(target);
    
    printf("  Converted 256.0uhk to unsigned short _Fract: %u (as integer)\n", 
           (unsigned int)(target * 256));
}

/* Test 2: Unsigned fract with positive high part (a_high > 0) */
void test_unsigned_fract_overflow_high() {
    printf("Test 2: Unsigned fract overflow with positive high part\n");
    
    /* Create a value with positive high part */
    unsigned _Accum source = 65536.0uhk;  /* Large value with high part > 0 */
    unsigned short _Fract target;
    
    target = (unsigned short _Fract)source;
    KEEP(target);
    
    printf("  Converted 65536.0uhk to unsigned short _Fract\n");
}

/* Test 3: Signed fract with overflow */
void test_signed_fract_overflow() {
    printf("Test 3: Signed fract overflow\n");
    
    /* For signed short _Fract, i_f_bits = 7 (one sign bit) */
    /* max_s after sext will be different */
    
    short _Accum source = 128.0hk;  /* Exceeds max for signed short _Fract */
    short _Fract target;
    
    target = (short _Fract)source;
    KEEP(target);
    
    printf("  Converted 128.0hk to short _Fract\n");
}

/* Test 4: Different i_f_bits values - using long fract types */
void test_long_fract_overflow() {
    printf("Test 4: Long fract overflow\n");
    
    /* unsigned long _Fract has i_f_bits = 16 (on typical targets) */
    unsigned long _Accum source = 65536.0ulrk;  /* 65536 in 16.16 format */
    unsigned long _Fract target;
    
    target = (unsigned long _Fract)source;
    KEEP(target);
    
    printf("  Converted 65536.0ulrk to unsigned long _Fract\n");
}

/* Test 5: Using builtins that might trigger overflow checking */
void test_builtin_overflow() {
    printf("Test 5: Using overflow builtins\n");
    
    unsigned short _Fract f1 = 0.5uhr;
    unsigned short _Fract f2 = 0.6uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin might use the same overflow checking logic */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(result);
    KEEP(overflow);
    
    printf("  Builtin add overflow check: %d + %d, overflow = %d\n",
           (int)(f1 * 256), (int)(f2 * 256), overflow);
}

/* Test 6: Saturated arithmetic */
void test_saturated_arithmetic() {
    printf("Test 6: Saturated arithmetic\n");
    
    /* With saturation attribute */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9uhr;
    unsigned short _Fract __attribute__((saturated)) sum;
    
    sum = f1 + f2;  /* Should saturate to max value */
    KEEP(sum);
    
    printf("  Saturated addition: 0.9 + 0.9 = %u (as 0.%u)\n",
           (unsigned int)(sum * 256), (unsigned int)(sum * 256));
}

/* Test 7: Mixed-type conversions */
void test_mixed_conversions() {
    printf("Test 7: Mixed-type conversions\n");
    
    /* Convert from integer to fixed-point with overflow */
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract f;
    
    f = (unsigned short _Fract)large_int;
    KEEP(f);
    
    printf("  Converted integer %u to unsigned short _Fract: %u\n",
           large_int, (unsigned int)(f * 256));
    
    /* Convert between different fixed-point types */
    unsigned _Accum acc = 512.0uhk;
    unsigned short _Fract fr;
    
    fr = (unsigned short _Fract)acc;
    KEEP(fr);
    
    printf("  Converted _Accum 512.0 to _Fract\n");
}

/* Test 8: Edge cases with different i_f_bits values */
void test_edge_cases() {
    printf("Test 8: Edge cases\n");
    
    /* Test with minimum i_f_bits (1) - use _Sat _Fract types */
    unsigned _Sat _Fract sf1 = 0.99ur;
    unsigned _Sat _Fract sf2 = 0.99ur;
    unsigned _Sat _Fract sf_sum;
    
    sf_sum = sf1 + sf2;
    KEEP(sf_sum);
    
    printf("  Edge case: saturated fract addition\n");
    
    /* Test exact boundary value */
    unsigned short _Fract boundary = 255.0/256.0;  /* Maximum representable */
    unsigned short _Fract slightly_above = boundary + (1.0/512.0);
    KEEP(slightly_above);
    
    printf("  Boundary test: max=%u, slightly above attempted\n",
           (unsigned int)(boundary * 256));
}

/* Test 9: Arithmetic operations that generate overflow */
void test_arithmetic_overflow() {
    printf("Test 9: Arithmetic overflow\n");
    
    unsigned short _Fract f1 = 0.8uhr;
    unsigned short _Fract f2 = 0.8uhr;
    unsigned short _Fract product;
    
    /* Multiplication can overflow even with values < 1.0 */
    product = f1 * f2 * 2.0;  /* Force overflow */
    KEEP(product);
    
    printf("  Arithmetic overflow from multiplication\n");
    
    /* Division by small number */
    unsigned short _Fract small = 0.1uhr;
    unsigned short _Fract large_result = f1 / small;
    KEEP(large_result);
    
    printf("  Division overflow test\n");
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests to trigger different overflow scenarios */
    test_unsigned_fract_overflow_low();
    test_unsigned_fract_overflow_high();
    test_signed_fract_overflow();
    test_long_fract_overflow();
    test_builtin_overflow();
    test_saturated_arithmetic();
    test_mixed_conversions();
    test_edge_cases();
    test_arithmetic_overflow();
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any test showed unexpected behavior */
    /* In practice, you'd check actual results here */
    return 0;
}
