/* Test program to trigger overflow checks in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization */
static volatile int dummy = 0;

/* Test 1: Overflow with zero high part but low part > max_s */
void test_low_part_overflow() {
    printf("Test 1: Low part overflow (high=0, low > max_s)\n");
    
    /* For unsigned short _Fract with 8 fractional bits:
       max_s = 2^8 - 1 = 255
       We need a value with high=0, low > 255 */
    
    /* Create a value slightly above 1.0 (which is 256 in 8.8 format) */
    unsigned short _Accum source = 1.1uk;  /* 1.1 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    /* Use the result to prevent dead code elimination */
    dummy += (int)target;
    printf("  Result: %u\n", (unsigned)target);
}

/* Test 2: Overflow with positive high part (a_high > max_r=0) */
void test_high_part_overflow() {
    printf("Test 2: High part overflow (a_high > 0)\n");
    
    /* For unsigned short _Fract (8 fractional bits, 0 integer bits):
       Any value >= 1.0 has high part > 0 */
    
    unsigned _Accum source = 2.0uk;  /* 2.0 in unsigned _Accum */
    unsigned short _Fract target;
    
    /* This should trigger a_high.sgt(max_r) */
    target = (unsigned short _Fract)source;
    
    dummy += (int)target;
    printf("  Result: %u\n", (unsigned)target);
}

/* Test 3: Signed types with different i_f_bits */
void test_signed_types() {
    printf("Test 3: Signed types overflow\n");
    
    /* Test with signed short _Fract (7 fractional bits + sign) */
    short _Accum source1 = 1.0hk;  /* Positive overflow */
    short _Fract target1;
    
    target1 = (short _Fract)source1;
    dummy += (int)target1;
    printf("  Signed overflow result: %d\n", (int)target1);
    
    /* Test negative overflow */
    short _Accum source2 = -1.0hk;
    short _Fract target2;
    
    target2 = (short _Fract)source2;
    dummy += (int)target2;
    printf("  Signed underflow result: %d\n", (int)target2);
}

/* Test 4: Using builtins to force overflow checking */
void test_builtin_overflow() {
    printf("Test 4: Using overflow builtins\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Multiplication that should overflow */
    overflow = __builtin_mul_overflow((unsigned)f1, (unsigned)f2, (unsigned*)&result);
    printf("  Builtin overflow: %d\n", overflow);
    
    dummy += overflow;
}

/* Test 5: Different i_f_bits values */
void test_different_fractional_bits() {
    printf("Test 5: Testing different fractional bit counts\n");
    
    /* Test with _Fract (typically 15 fractional bits) */
    unsigned _Fract f1 = 1.0ur;
    unsigned short _Fract f2;
    
    f2 = (unsigned short _Fract)f1;  /* Should overflow */
    dummy += (int)f2;
    printf("  _Fract to short _Fract: %u\n", (unsigned)f2);
    
    /* Test with long _Fract (typically 31 fractional bits) */
    unsigned long _Fract lf = 1.0ulr;
    unsigned _Fract f3;
    
    f3 = (unsigned _Fract)lf;  /* Should overflow */
    dummy += (int)f3;
    printf("  long _Fract to _Fract: %u\n", (unsigned)f3);
}

/* Test 6: Arithmetic operations causing overflow */
void test_arithmetic_overflow() {
    printf("Test 6: Arithmetic overflow\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.8ur;
    unsigned short _Fract sum;
    
    /* Addition that exceeds 1.0 */
    sum = f1 + f2;  /* 0.8 + 0.8 = 1.6 > 1.0 */
    
    dummy += (int)sum;
    printf("  Addition overflow result: %u\n", (unsigned)sum);
    
    /* Multiplication overflow */
    unsigned short _Fract f3 = 0.9ur;
    unsigned short _Fract f4 = 0.9ur;
    unsigned short _Fract product;
    
    product = f3 * f4;  /* 0.9 * 0.9 = 0.81 < 1.0, but test anyway */
    
    dummy += (int)product;
    printf("  Multiplication result: %u\n", (unsigned)product);
}

/* Test 7: Saturated arithmetic */
#ifdef __SAT_FRACT__
void test_saturated_arithmetic() {
    printf("Test 7: Saturated arithmetic\n");
    
    /* With saturation attribute */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sum;
    
    sum = f1 + f2;  /* Should saturate to max value */
    
    dummy += (int)sum;
    printf("  Saturated addition: %u\n", (unsigned)sum);
}
#endif

/* Test 8: Edge case - exact boundary value */
void test_boundary() {
    printf("Test 8: Boundary value test\n");
    
    /* For unsigned short _Fract, max value is 255/256 ≈ 0.99609375 */
    /* Create a value just below and just above the boundary */
    
    unsigned short _Accum below = 0.996uk;  /* Just below max */
    unsigned short _Accum above = 1.0uk;    /* At boundary */
    
    unsigned short _Fract result1 = (unsigned short _Fract)below;
    unsigned short _Fract result2 = (unsigned short _Fract)above;
    
    dummy += (int)result1 + (int)result2;
    printf("  Below boundary: %u, At boundary: %u\n", 
           (unsigned)result1, (unsigned)result2);
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_low_part_overflow();
    printf("\n");
    
    test_high_part_overflow();
    printf("\n");
    
    test_signed_types();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_different_fractional_bits();
    printf("\n");
    
    test_arithmetic_overflow();
    printf("\n");
    
    #ifdef __SAT_FRACT__
    test_saturated_arithmetic();
    printf("\n");
    #endif
    
    test_boundary();
    printf("\n");
    
    printf("All tests completed. Dummy sum: %d\n", dummy);
    printf("Note: The actual coverage happens during compilation,\n");
    printf("      not during runtime execution.\n");
    
    return 0;
}
