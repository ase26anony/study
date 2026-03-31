/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test test.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int dummy;

/* Test different i_f_bits values through various fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_unsigned_short_fract(void) {
    printf("Test 1: unsigned short _Fract (i_f_bits=8)\n");
    
    /* max_s = 2^8 - 1 = 255 */
    /* Try to trigger a_high == 0 && a_low > 255 */
    
    /* Create a value that when converted has low part > 255 */
    unsigned short _Accum source = 256.0uhk;  /* 256 * 2^8 = 65536 in internal representation */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    dummy = (int)target;
    printf("  Conversion result: %u/256\n", (unsigned int)(target * 256));
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_unsigned_fract(void) {
    printf("Test 2: unsigned _Fract (i_f_bits=16)\n");
    
    /* max_s = 2^16 - 1 = 65535 */
    /* Try to trigger a_high > 0 */
    
    /* Create a value with positive high part */
    unsigned _Accum source = 65536.0uk;  /* Will have high part > 0 when converted */
    unsigned _Fract target;
    
    target = (unsigned _Fract)source;
    
    dummy = (int)target;
    printf("  Conversion result: %u/65536\n", (unsigned int)(target * 65536));
}

/* Test 3: i_f_bits = 24 (unsigned long _Fract) */
void test_unsigned_long_fract(void) {
    printf("Test 3: unsigned long _Fract (i_f_bits=24)\n");
    
    /* max_s = 2^24 - 1 = 16777215 */
    /* Trigger both conditions with different values */
    
    /* Condition 1: a_high == 0 && a_low > max_s */
    unsigned long _Accum source1 = 16777216.0ulk;  /* max_s + 1 */
    unsigned long _Fract target1;
    
    target1 = (unsigned long _Fract)source1;
    
    /* Condition 2: a_high > 0 */
    unsigned long _Accum source2 = 16777217.0ulk;  /* Even larger */
    unsigned long _Fract target2;
    
    target2 = (unsigned long _Fract)source2;
    
    dummy = (int)target1 + (int)target2;
    printf("  Conversion 1 result: %u/16777216\n", (unsigned int)(target1 * 16777216));
    printf("  Conversion 2 result: %u/16777216\n", (unsigned int)(target2 * 16777216));
}

/* Test 4: i_f_bits = 8 with signed type */
void test_signed_short_fract(void) {
    printf("Test 4: signed short _Fract (i_f_bits=8)\n");
    
    /* For signed, the logic is similar but with sign extension */
    signed short _Accum source = 128.0hk;  /* Max positive for 7 bits + sign */
    signed short _Fract target;
    
    /* Try to overflow positive range */
    source = source + 0.5hk;
    target = (signed short _Fract)source;
    
    dummy = (int)target;
    printf("  Conversion result: %d/128\n", (int)(target * 128));
}

/* Test 5: i_f_bits = 1 (minimum) */
void test_minimum_fbits(void) {
    printf("Test 5: Test with i_f_bits=1\n");
    
    /* Use saturating arithmetic attribute to ensure overflow checking */
    typedef unsigned _Fract __attribute__((saturated)) sat_ufract;
    
    sat_ufract a = 0.5ur;
    sat_ufract b = 0.6ur;
    sat_ufract sum;
    
    /* This addition might overflow depending on implementation */
    sum = a + b;
    
    dummy = (int)sum;
    printf("  Saturating addition result: %u/65536\n", (unsigned int)(sum * 65536));
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 6: Using overflow builtins\n");
    
    unsigned short _Fract f1 = 0.9ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This multiplication should overflow for 8 fractional bits */
    overflow = __builtin_mul_overflow((unsigned int)(f1 * 256), 
                                      (unsigned int)(f2 * 256), 
                                      (unsigned int*)&result);
    
    dummy = overflow + (int)result;
    printf("  Overflow detected: %d\n", overflow);
}

/* Test 7: Mixed integer and fixed-point conversions */
void test_mixed_conversions(void) {
    printf("Test 7: Integer to fixed-point conversion\n");
    
    /* Convert from integer that exceeds fixed-point range */
    unsigned int large_int = 1000;
    unsigned short _Fract f;
    
    /* This should trigger overflow (max is 255/256) */
    f = (unsigned short _Fract)large_int;
    
    dummy = (int)f;
    printf("  Integer conversion result: %u/256\n", (unsigned int)(f * 256));
}

/* Test 8: Chain of operations causing overflow */
void test_operation_chain(void) {
    printf("Test 8: Chain of operations\n");
    
    unsigned _Fract f = 0.9ur;
    unsigned _Fract accumulator = 0.0ur;
    int i;
    
    /* Repeated addition that should eventually overflow */
    for (i = 0; i < 10; i++) {
        accumulator = accumulator + f;
    }
    
    dummy = (int)accumulator;
    printf("  Accumulator after 10 additions: %u/65536\n", 
           (unsigned int)(accumulator * 65536));
}

int main(void) {
    int result = 0;
    
    printf("=== Testing fixed-value.cc uncovered lines ===\n\n");
    
    test_unsigned_short_fract();
    test_unsigned_fract();
    test_unsigned_long_fract();
    test_signed_short_fract();
    test_minimum_fbits();
    test_builtin_overflow();
    test_mixed_conversions();
    test_operation_chain();
    
    printf("\n=== All tests completed ===\n");
    
    /* Use dummy to prevent dead code elimination */
    if (dummy) result = 1;
    
    return result;
}
