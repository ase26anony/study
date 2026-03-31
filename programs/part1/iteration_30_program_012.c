/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract_overflow(void) {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* For unsigned short _Fract: 8 fractional bits, 0 integer bits */
    /* Maximum value = (2^8 - 1)/2^8 = 255/256 ≈ 0.996 */
    
    /* This should trigger a_high == 0, a_low > max_s */
    /* max_s = 2^8 - 1 = 255 */
    /* We need a value where high part is 0 and low part > 255 */
    
    /* Create a value slightly above 1.0 */
    unsigned short _Accum source = 1.1uk;  /* 1.1 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* Explicit cast that should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    /* Use the result to prevent dead code elimination */
    global_counter += (target > 0);
    printf("  Result: %u/256\n", (unsigned)(target * 256));
}

/* Test 2: Overflow with positive high part */
void test_positive_high_part(void) {
    printf("Test 2: Positive high part overflow\n");
    
    /* For unsigned _Fract: typically 16 fractional bits */
    /* We need a_high > 0 (positive high part) */
    
    /* Use a large _Accum value that has non-zero high part */
    unsigned long _Accum large_value = 65536.0ULK;  /* 2^16 */
    unsigned _Fract small_target;
    
    /* This should trigger a_high.sgt(0) */
    small_target = (unsigned _Fract)large_value;
    
    global_counter += (small_target > 0);
    printf("  Result: %u/65536\n", (unsigned)(small_target * 65536));
}

/* Test 3: Multiple fractional bit widths */
void test_various_fractional_bits(void) {
    printf("Test 3: Various fractional bit widths\n");
    
    /* Test different i_f_bits values */
    
    /* i_f_bits = 1 (minimum) */
    {
        /* For 1 fractional bit: max = (2^1 - 1)/2^1 = 0.5 */
        unsigned _Fract f1 = 0.6ur;
        global_counter += (f1 > 0);
    }
    
    /* i_f_bits = 16 (typical for unsigned _Fract) */
    {
        unsigned long _Accum a1 = 2.0ULK;  /* > 1.0 */
        unsigned _Fract f2 = (unsigned _Fract)a1;
        global_counter += (f2 > 0);
    }
    
    /* i_f_bits = 24 */
    {
        unsigned long long _Accum a2 = 2.0ULLK;
        unsigned long _Fract f3 = (unsigned long _Fract)a2;
        global_counter += (f3 > 0);
    }
}

/* Test 4: Signed types with overflow */
void test_signed_overflow(void) {
    printf("Test 4: Signed type overflow\n");
    
    /* Signed types also go through similar overflow checking */
    short _Accum sa = 1.5hk;  /* > 1.0 for short _Fract */
    short _Fract sf;
    
    sf = (short _Fract)sa;
    global_counter += (sf > 0);
    
    /* Test negative overflow too */
    short _Accum sa2 = -1.5hk;
    short _Fract sf2;
    
    sf2 = (short _Fract)sa2;
    global_counter += (sf2 < 0);
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Test 5: Builtin overflow operations\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    
    /* This addition might overflow (0.5 + 0.6 = 1.1 > 1.0) */
    result = f1 + f2;
    global_counter += (result > 0);
    
    /* Multiplication that could overflow */
    f1 = 0.9ur;
    f2 = 0.9ur;
    result = f1 * f2;  /* 0.81 < 1.0, but let's see */
    global_counter += (result > 0);
}

/* Test 6: Explicit overflow with constants */
void test_constant_overflow(void) {
    printf("Test 6: Constant overflow\n");
    
    /* Direct assignment that should overflow */
    unsigned short _Fract f;
    
    /* These should trigger overflow checking at compile time */
    f = 1.0ur;  /* Exactly at boundary */
    global_counter += (f > 0);
    
    f = 1.1ur;  /* Definitely overflows */
    global_counter += (f > 0);
    
    /* Using different representations */
    f = 256.0ur / 256.0ur;  /* Should be 1.0 */
    global_counter += (f > 0);
    
    f = 300.0ur / 256.0ur;  /* > 1.0, should overflow */
    global_counter += (f > 0);
}

/* Test 7: Saturated arithmetic */
#ifdef __SAT_FRACT__
void test_saturated_arithmetic(void) {
    printf("Test 7: Saturated arithmetic\n");
    
    /* With saturation, overflow should saturate to max value */
    _Sat unsigned short _Fract sf1 = 0.9ur;
    _Sat unsigned short _Fract sf2 = 0.9ur;
    _Sat unsigned short _Fract sum;
    
    sum = sf1 + sf2;  /* 1.8 > 1.0, should saturate to 1.0 */
    global_counter += (sum == 1.0ur);
    
    /* Overflow in conversion */
    _Sat unsigned short _Accum sa = 2.0uk;
    _Sat unsigned short _Fract sf3;
    
    sf3 = (_Sat unsigned short _Fract)sa;  /* Should saturate to 1.0 */
    global_counter += (sf3 == 1.0ur);
}
#endif

int main(void) {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_short_fract_overflow();
    printf("\n");
    
    test_positive_high_part();
    printf("\n");
    
    test_various_fractional_bits();
    printf("\n");
    
    test_signed_overflow();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_constant_overflow();
    printf("\n");
    
#ifdef __SAT_FRACT__
    test_saturated_arithmetic();
    printf("\n");
#endif
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    printf("(Note: The actual coverage happens during compilation,\n");
    printf(" not necessarily at runtime)\n");
    
    return 0;
}
