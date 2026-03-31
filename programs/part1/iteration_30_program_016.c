/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int prevent_opt = 0;

/* Test different fractional bit counts through various fixed-point types */

/* Test 1: i_f_bits = 8 (unsigned short _Fract) */
void test_8_fractional_bits(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract):\n");
    
    /* Case 1: Trigger a_high == max_r (0) && a_low > max_s (2^8-1 = 255) */
    /* Convert from _Accum (16 fractional bits) to _Fract (8 fractional bits) */
    unsigned short _Accum source1 = 1.0uhk;  /* 1.0 in 8.8 format = 256 */
    unsigned short _Fract target1 = (unsigned short _Fract)source1;
    
    /* This should trigger overflow since 256 > 255/256 */
    /* In internal representation: high=0, low=256 > 255 */
    printf("  Conversion 1.0uhk -> usf: target = %u (0x%x)\n", 
           (unsigned)(target1 * 256), (unsigned)(target1 * 256));
    
    /* Case 2: Trigger a_high > max_r (positive high part) */
    /* Use a larger integer value */
    unsigned _Accum source2 = 256.0uhk;  /* 256 in 8.8 format = 65536 */
    unsigned short _Fract target2 = (unsigned short _Fract)source2;
    
    /* This should have high part > 0 */
    printf("  Conversion 256.0uhk -> usf: target = %u (0x%x)\n",
           (unsigned)(target2 * 256), (unsigned)(target2 * 256));
}

/* Test 2: i_f_bits = 16 (unsigned _Fract) */
void test_16_fractional_bits(void) {
    printf("\nTesting i_f_bits = 16 (unsigned _Fract):\n");
    
    /* Case 1: a_low > 2^16-1 = 65535 */
    unsigned _Accum source1 = 1.0uhk;  /* 1.0 in 16.16 format = 65536 */
    unsigned _Fract target1 = (unsigned _Fract)source1;
    
    printf("  Conversion 1.0uhk -> uf: target = %u (0x%x)\n",
           (unsigned)(target1 * 65536), (unsigned)(target1 * 65536));
    
    /* Case 2: Positive high part */
    unsigned long _Accum source2 = 65536.0ulhk;  /* Large value */
    unsigned _Fract target2 = (unsigned _Fract)source2;
    
    printf("  Conversion 65536.0ulhk -> uf: target = %u (0x%x)\n",
           (unsigned)(target2 * 65536), (unsigned)(target2 * 65536));
}

/* Test 3: i_f_bits = 24 (unsigned long _Fract) */
void test_24_fractional_bits(void) {
    printf("\nTesting i_f_bits = 24 (unsigned long _Fract):\n");
    
    /* Need to create value > 2^24-1 = 16777215 */
    unsigned long _Accum source = 1.0ulhk;  /* 1.0 in 24.24 format = 16777216 */
    unsigned long _Fract target = (unsigned long _Fract)source;
    
    printf("  Conversion 1.0ulhk -> ulf: target = %lu (0x%lx)\n",
           (unsigned long)(target * 16777216), (unsigned long)(target * 16777216));
}

/* Test 4: i_f_bits = 1 (edge case) */
void test_1_fractional_bit(void) {
    printf("\nTesting i_f_bits = 1:\n");
    
    /* Create custom type using saturating arithmetic */
    typedef _Sat unsigned short _Fract usf_sat;
    
    /* With 1 fractional bit, max_s = 2^1-1 = 1 */
    /* We need low part > 1 with high part = 0 */
    unsigned short _Accum source = 0.75uhk;  /* 0.75 in 8.8 format = 192 */
    usf_sat target = (usf_sat)source;
    
    printf("  Conversion 0.75uhk -> usf_sat: target = %u (0x%x)\n",
           (unsigned)(target * 2), (unsigned)(target * 2));
}

/* Test 5: Signed types to trigger different paths */
void test_signed_types(void) {
    printf("\nTesting signed types:\n");
    
    /* Signed short _Fract with 8 fractional bits */
    short _Accum source_pos = 1.0hk;    /* 1.0 in s7.8 format = 256 */
    short _Accum source_neg = -1.0hk;   /* -1.0 in s7.8 format = -256 */
    
    _Sat short _Fract target_pos = (_Sat short _Fract)source_pos;
    _Sat short _Fract target_neg = (_Sat short _Fract)source_neg;
    
    printf("  Signed conversion +1.0hk -> sf_sat: %d (0x%x)\n",
           (int)(target_pos * 256), (int)(target_pos * 256));
    printf("  Signed conversion -1.0hk -> sf_sat: %d (0x%x)\n",
           (int)(target_neg * 256), (int)(target_neg * 256));
}

/* Test 6: Using builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("\nTesting builtin overflow:\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    
    /* This addition might overflow 1.0 */
    int overflow = __builtin_add_overflow(f1, f2, &result);
    
    printf("  Builtin add overflow: 0.5ur + 0.6ur = %u, overflow=%d\n",
           (unsigned)(result * 256), overflow);
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 0.9ur;
    overflow = __builtin_mul_overflow(f1, f3, &result);
    
    printf("  Builtin mul overflow: 0.5ur * 0.9ur = %u, overflow=%d\n",
           (unsigned)(result * 256), overflow);
}

/* Test 7: Arithmetic operations that cause overflow */
void test_arithmetic_overflow(void) {
    printf("\nTesting arithmetic overflow:\n");
    
    /* Use non-saturating types to potentially trigger overflow checks */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    
    /* This sum > 1.0, should overflow for 8 fractional bits */
    unsigned short _Fract sum = f1 + f2;
    
    printf("  Arithmetic: 0.8ur + 0.9ur = %u (0x%x)\n",
           (unsigned)(sum * 256), (unsigned)(sum * 256));
    
    /* Chain operations to create larger values */
    unsigned _Accum acc = 100.0uhk;
    for (int i = 0; i < 10; i++) {
        acc = acc * 1.1uhk;
    }
    unsigned short _Fract converted = (unsigned short _Fract)acc;
    
    printf("  Chained ops -> conversion: %u (0x%x)\n",
           (unsigned)(converted * 256), (unsigned)(converted * 256));
}

/* Test 8: Different conversion paths */
void test_mixed_conversions(void) {
    printf("\nTesting mixed conversions:\n");
    
    /* Integer to fixed-point */
    unsigned int int_val = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract from_int = (unsigned short _Fract)int_val;
    
    printf("  int 300 -> usf: %u (0x%x)\n",
           (unsigned)(from_int * 256), (unsigned)(from_int * 256));
    
    /* Float to fixed-point */
    float float_val = 1.5f;
    unsigned short _Fract from_float = (unsigned short _Fract)float_val;
    
    printf("  float 1.5 -> usf: %u (0x%x)\n",
           (unsigned)(from_float * 256), (unsigned)(from_float * 256));
}

int main(void) {
    int test_result = 0;
    
    printf("=== Testing overflow conditions for fixed-value.cc coverage ===\n\n");
    
    /* Run all tests */
    test_8_fractional_bits();
    test_16_fractional_bits();
    test_24_fractional_bits();
    test_1_fractional_bit();
    test_signed_types();
    test_builtin_overflow();
    test_arithmetic_overflow();
    test_mixed_conversions();
    
    printf("\n=== All tests completed ===\n");
    
    /* Use results to prevent dead code elimination */
    prevent_opt = 1;
    
    return test_result;
}
