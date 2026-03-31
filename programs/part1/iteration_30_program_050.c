/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed test_fixed.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static volatile unsigned int prevent_opt = 0;

/* Test 1: Overflow with zero high part but low part > max_s */
void test_low_part_overflow() {
    printf("Test 1: Low part overflow (high=0, low > max_s)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       max_s = 2^8 - 1 = 255
       We need a value with high=0, low > 255 */
    
    /* Create a value that's just above 255/256 */
    unsigned short _Accum a = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract b;
    
    /* This conversion should trigger overflow check */
    b = (unsigned short _Fract)a;
    
    /* Use the result to prevent dead code elimination */
    prevent_opt = (unsigned int)b;
    printf("  Result: %u\n", (unsigned int)b);
}

/* Test 2: Overflow with positive high part (high > max_r=0) */
void test_high_part_overflow() {
    printf("Test 2: High part overflow (high > 0)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       We need a value with high part > 0 */
    
    /* 256.0 has high part > 0 for 8 fractional bits */
    unsigned short _Accum a = 256.0uhk;
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;
    
    prevent_opt += (unsigned int)b;
    printf("  Result: %u\n", (unsigned int)b);
}

/* Test 3: Signed types with different i_f_bits */
void test_signed_types() {
    printf("Test 3: Signed types overflow\n");
    
    /* Test with 16 fractional bits */
    signed long _Accum a = 32768.0lk;  /* Large positive value */
    signed short _Fract b;
    
    b = (signed short _Fract)a;  /* Should overflow */
    
    prevent_opt += (unsigned int)b;
    printf("  Signed overflow result: %d\n", (int)b);
    
    /* Test negative overflow */
    signed long _Accum c = -32769.0lk;
    signed short _Fract d;
    
    d = (signed short _Fract)c;  /* Should underflow */
    
    prevent_opt += (unsigned int)d;
    printf("  Signed underflow result: %d\n", (int)d);
}

/* Test 4: Different i_f_bits values */
void test_various_fractional_bits() {
    printf("Test 4: Various fractional bit counts\n");
    
    /* Test with _Fract (typically 8 bits) */
    {
        unsigned _Accum a = 2.0uk;
        unsigned _Fract b = (unsigned _Fract)a;  /* Should overflow */
        prevent_opt += b;
        printf("  _Fract (8-bit): %u\n", (unsigned int)b);
    }
    
    /* Test with short _Fract (typically 8 bits) */
    {
        unsigned short _Accum a = 2.0uhk;
        unsigned short _Fract b = (unsigned short _Fract)a;
        prevent_opt += b;
        printf("  short _Fract (8-bit): %u\n", (unsigned int)b);
    }
    
    /* Test with long _Fract (typically 16 bits) */
    {
        unsigned long _Accum a = 65536.0ulk;
        unsigned long _Fract b = (unsigned long _Fract)a;
        prevent_opt += b;
        printf("  long _Fract (16-bit): %u\n", (unsigned int)b);
    }
}

/* Test 5: Arithmetic operations that cause overflow */
void test_arithmetic_overflow() {
    printf("Test 5: Arithmetic overflow\n");
    
    /* Addition overflow */
    unsigned short _Fract a = 0.8uhr;
    unsigned short _Fract b = 0.8uhr;
    unsigned short _Fract sum;
    
    /* This addition might overflow depending on saturation mode */
    sum = a + b;
    
    prevent_opt += sum;
    printf("  Addition: 0.8 + 0.8 = %u/256\n", (unsigned int)sum);
    
    /* Multiplication overflow */
    unsigned short _Accum c = 200.0uhk;
    unsigned short _Accum d = 2.0uhk;
    unsigned short _Accum prod;
    
    prod = c * d;  /* 400 > max for short _Accum? */
    
    prevent_opt += prod;
    printf("  Multiplication: 200 * 2 = %u\n", (unsigned int)prod);
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("Test 6: Builtin overflow checks\n");
    
    unsigned short _Fract a = 0.9uhr;
    unsigned short _Fract b = 0.9uhr;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin should trigger overflow checking */
    overflow = __builtin_add_overflow(a, b, &result);
    
    prevent_opt += result + overflow;
    printf("  Builtin add overflow: result=%u, overflow=%d\n", 
           (unsigned int)result, overflow);
}

/* Test 7: Saturation behavior */
void test_saturation() {
    printf("Test 7: Saturation behavior\n");
    
    /* With -fsaturated-arithmetic, values should saturate */
    unsigned short _Accum a = 300.0uhk;
    unsigned short _Fract b;
    
    b = (unsigned short _Fract)a;  /* Should saturate to max */
    
    prevent_opt += b;
    printf("  Saturated conversion: %u (expected 255)\n", (unsigned int)b);
    
    /* Test signed saturation */
    signed short _Accum c = 200.0hk;
    signed short _Accum d = 200.0hk;
    signed short _Accum sum;
    
    sum = c + d;  /* Should saturate if enabled */
    
    prevent_opt += (unsigned int)sum;
    printf("  Saturated addition: %d\n", (int)sum);
}

/* Test 8: Edge cases around the boundary */
void test_boundary_cases() {
    printf("Test 8: Boundary cases\n");
    
    /* Test exactly at the boundary (should not overflow) */
    unsigned short _Fract max_val = 0.99609375uhr;  /* 255/256 */
    unsigned short _Accum source = 255.0uhk / 256.0uhk;
    unsigned short _Fract result;
    
    result = (unsigned short _Fract)source;
    
    prevent_opt += result;
    printf("  At boundary: %u (expected 255)\n", (unsigned int)result);
    
    /* Test just above the boundary (should overflow) */
    unsigned short _Accum just_above = 256.0uhk / 256.0uhk;  /* 1.0 */
    unsigned short _Fract result2;
    
    result2 = (unsigned short _Fract)just_above;
    
    prevent_opt += result2;
    printf("  Just above boundary: %u\n", (unsigned int)result2);
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_low_part_overflow();
    printf("\n");
    
    test_high_part_overflow();
    printf("\n");
    
    test_signed_types();
    printf("\n");
    
    test_various_fractional_bits();
    printf("\n");
    
    test_arithmetic_overflow();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_saturation();
    printf("\n");
    
    test_boundary_cases();
    printf("\n");
    
    printf("All tests completed. prevent_opt = %u\n", prevent_opt);
    
    /* Return non-zero if any test showed unexpected behavior */
    return (prevent_opt == 0) ? 1 : 0;
}
