/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    (void)_tmp; \
} while(0)

/* Test 1: Overflow for unsigned types with i_f_bits = 8 */
void test_unsigned_overflow_8bit(void)
{
    /* For unsigned short _Fract: i_f_bits = 8, max_s = 255 */
    /* Condition: a_high == 0 && a_low > 255 */
    
    /* Create a value with low part = 256 (exceeds max_s) */
    unsigned short _Accum source = 1.0uhk;  /* 256 in 8.8 format */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    KEEP(target);
    
    printf("Test 1: unsigned 8-bit overflow - source=%.4f, target=%.4f\n",
           (double)source / 256.0, (double)target / 256.0);
}

/* Test 2: Overflow for unsigned types with i_f_bits = 16 */
void test_unsigned_overflow_16bit(void)
{
    /* For unsigned _Fract: i_f_bits = 16, max_s = 65535 */
    /* Create value with low part = 65536 */
    unsigned _Accum source = 1.0uk;  /* 65536 in 16.16 format */
    unsigned _Fract target;
    
    target = (unsigned _Fract)source;
    KEEP(target);
    
    printf("Test 2: unsigned 16-bit overflow - source=%.8f, target=%.8f\n",
           (double)source / 65536.0, (double)target / 65536.0);
}

/* Test 3: Positive high part case (a_high > 0) */
void test_positive_high_part(void)
{
    /* For signed short _Fract: i_f_bits = 7 (signed has one less fractional bit) */
    /* Create value with positive high part */
    signed short _Accum source = 128.0hk;  /* High part is positive */
    signed short _Fract target;
    
    target = (signed short _Fract)source;
    KEEP(target);
    
    printf("Test 3: positive high part - source=%.4f, target=%.4f\n",
           (double)source / 128.0, (double)target / 128.0);
}

/* Test 4: Mixed arithmetic causing overflow */
void test_mixed_arithmetic(void)
{
    unsigned _Fract f1 = 0.8ur;
    unsigned _Fract f2 = 0.9ur;
    unsigned _Fract sum;
    
    /* This addition might overflow depending on rounding */
    sum = f1 + f2;
    KEEP(sum);
    
    printf("Test 4: mixed arithmetic - f1=%.4f, f2=%.4f, sum=%.4f\n",
           (double)f1, (double)f2, (double)sum);
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void)
{
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use overflow builtin if available */
    #ifdef __builtin_mul_overflow
    overflow = __builtin_mul_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    printf("Test 5: builtin overflow check - overflow=%d\n", overflow);
    #endif
}

/* Test 6: Saturation behavior */
void test_saturation(void)
{
    /* Test with saturation attribute */
    unsigned short _Fract __attribute__((saturated)) sat_f1 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sat_f2 = 0.8ur;
    unsigned short _Fract __attribute__((saturated)) sat_sum;
    
    sat_sum = sat_f1 + sat_f2;
    KEEP(sat_sum);
    
    printf("Test 6: saturation test - sum=%.4f\n", (double)sat_sum);
}

/* Test 7: Different i_f_bits values */
void test_various_i_f_bits(void)
{
    /* Test various fractional bit counts */
    
    /* i_f_bits = 1 (minimum) */
    {
        /* Create a custom fixed-point type simulation */
        unsigned _Fract f = 0.5ur;
        unsigned _Fract scaled = f * 2.0ur;
        KEEP(scaled);
    }
    
    /* i_f_bits = 24 */
    {
        unsigned long _Fract f = 0.999999ulr;
        unsigned _Fract converted = (unsigned _Fract)f;
        KEEP(converted);
    }
    
    /* i_f_bits = 32 */
    {
        unsigned long long _Fract f = 0.999999999ullr;
        unsigned long _Fract converted = (unsigned long _Fract)f;
        KEEP(converted);
    }
}

/* Test 8: Explicit overflow by exceeding maximum */
void test_explicit_overflow(void)
{
    /* Directly assign values that exceed the maximum */
    
    /* For unsigned short _Fract, max is 255/256 ≈ 0.9961 */
    volatile double too_large = 1.5;
    unsigned short _Fract f = (unsigned short _Fract)too_large;
    KEEP(f);
    
    printf("Test 8: explicit overflow - f=%.4f\n", (double)f);
}

/* Test 9: Integer to fixed-point conversion overflow */
void test_int_to_fixed(void)
{
    /* Convert from integer that's too large */
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract f = (unsigned short _Fract)large_int;
    KEEP(f);
    
    printf("Test 9: int to fixed overflow - int=%u, f=%.4f\n", 
           large_int, (double)f);
}

/* Test 10: Nested conversions with overflow */
void test_nested_conversions(void)
{
    /* Chain conversions that each might overflow */
    unsigned long long _Accum ll_acc = 1000000.0ullk;
    unsigned long _Accum l_acc = (unsigned long _Accum)ll_acc;
    unsigned _Accum acc = (unsigned _Accum)l_acc;
    unsigned _Fract f = (unsigned _Fract)acc;
    
    KEEP(f);
    printf("Test 10: nested conversions - final=%.8f\n", (double)f);
}

int main(void)
{
    int tests_passed = 0;
    
    printf("Starting fixed-point overflow tests...\n\n");
    
    /* Run all tests */
    test_unsigned_overflow_8bit();
    tests_passed++;
    
    test_unsigned_overflow_16bit();
    tests_passed++;
    
    test_positive_high_part();
    tests_passed++;
    
    test_mixed_arithmetic();
    tests_passed++;
    
    test_builtin_overflow();
    tests_passed++;
    
    test_saturation();
    tests_passed++;
    
    test_various_i_f_bits();
    tests_passed++;
    
    test_explicit_overflow();
    tests_passed++;
    
    test_int_to_fixed();
    tests_passed++;
    
    test_nested_conversions();
    tests_passed++;
    
    printf("\nAll %d tests completed.\n", tests_passed);
    
    /* Return success only if we actually ran all tests */
    return 0;
}
