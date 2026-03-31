/* Test program to trigger overflow checks in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int prevent_opt = 0;

/* Test 1: Overflow in unsigned fract with 8 fractional bits */
void test_unsigned_fract_overflow()
{
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* We need a_high = 0 and a_low > 255 */
    
    /* Create a value that when converted to unsigned short _Fract exceeds 255/256 */
    unsigned short _Accum source = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    /* Use the result to prevent dead code elimination */
    prevent_opt = (unsigned int)target;
}

/* Test 2: Positive high part case */
void test_positive_high_part()
{
    /* We need a_high > 0 */
    /* Use a large integer that when converted has high part > 0 */
    
    long _Accum source = 256.0lk;  /* Large value that will have high part > 0 */
    unsigned short _Fract target;
    
    target = (unsigned short _Fract)source;
    
    prevent_opt += (unsigned int)target;
}

/* Test 3: Different i_f_bits values - 16 fractional bits */
void test_16_fractional_bits()
{
    /* For unsigned _Fract: i_f_bits = 16, max_s = 65535 */
    
    unsigned _Accum source = 1.0uk;  /* 1.0 in unsigned _Accum */
    unsigned _Fract target;
    
    target = (unsigned _Fract)source;
    
    prevent_opt += (unsigned int)target;
}

/* Test 4: Signed types with overflow */
void test_signed_overflow()
{
    /* For signed short _Fract: i_f_bits = 7 (one sign bit), max_s = 127 */
    
    short _Accum source = 1.0hk;  /* 1.0 in short _Accum */
    short _Fract target;
    
    target = (short _Fract)source;
    
    prevent_opt += (unsigned int)target;
}

/* Test 5: Using builtins that might trigger overflow checks */
void test_builtin_overflow()
{
    unsigned short _Fract a = 0.5uhr;
    unsigned short _Fract b = 0.6uhr;
    unsigned short _Fract result;
    
    /* This addition might overflow and trigger checks */
    result = a + b;
    
    prevent_opt += (unsigned int)result;
}

/* Test 6: Explicit overflow with multiplication */
void test_multiplication_overflow()
{
    unsigned short _Fract a = 0.9uhr;
    unsigned short _Fract b = 0.9uhr;
    unsigned short _Fract result;
    
    result = a * b;  /* 0.81, should be fine but still goes through checks */
    
    prevent_opt += (unsigned int)result;
}

/* Test 7: Edge case - value exactly at boundary */
void test_boundary_case()
{
    /* Create value with a_low = max_s = 255 */
    /* For unsigned short _Fract, max value is 255/256 ≈ 0.996 */
    
    unsigned short _Accum source = 255.0uhk / 256.0uhk;
    unsigned short _Fract target;
    
    target = (unsigned short _Fract)source;
    
    prevent_opt += (unsigned int)target;
}

/* Test 8: Overflow with different conversion paths */
void test_mixed_conversions()
{
    /* Chain conversions to trigger different code paths */
    int integer = 1000;
    unsigned short _Accum accum;
    unsigned short _Fract fract;
    
    accum = (unsigned short _Accum)integer;
    fract = (unsigned short _Fract)accum;  /* Should overflow */
    
    prevent_opt += (unsigned int)fract;
}

/* Test 9: Using saturating arithmetic attribute */
void test_saturating_arithmetic()
{
    unsigned short _Fract __attribute__((saturated)) a = 0.5uhr;
    unsigned short _Fract __attribute__((saturated)) b = 0.6uhr;
    unsigned short _Fract __attribute__((saturated)) result;
    
    result = a + b;  /* With saturation attribute, might use different overflow logic */
    
    prevent_opt += (unsigned int)result;
}

/* Test 10: Very large i_f_bits (24 bits for long fract) */
void test_large_fractional_bits()
{
    unsigned long _Accum source = 1.0ulk;
    unsigned long _Fract target;
    
    target = (unsigned long _Fract)source;
    
    prevent_opt += (unsigned int)target;
}

int main()
{
    int tests_passed = 0;
    
    printf("Starting fixed-point overflow tests...\n");
    
    /* Run all tests */
    test_unsigned_fract_overflow();
    tests_passed++;
    
    test_positive_high_part();
    tests_passed++;
    
    test_16_fractional_bits();
    tests_passed++;
    
    test_signed_overflow();
    tests_passed++;
    
    test_builtin_overflow();
    tests_passed++;
    
    test_multiplication_overflow();
    tests_passed++;
    
    test_boundary_case();
    tests_passed++;
    
    test_mixed_conversions();
    tests_passed++;
    
    test_saturating_arithmetic();
    tests_passed++;
    
    test_large_fractional_bits();
    tests_passed++;
    
    printf("Ran %d tests\n", tests_passed);
    printf("prevent_opt = %u (used to prevent optimization)\n", prevent_opt);
    
    /* Return success if we got here */
    return 0;
}
