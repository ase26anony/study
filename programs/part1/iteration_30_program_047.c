/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile unsigned int use_result;

/* Test different i_f_bits values through different fixed-point types */

/* i_f_bits = 8 (unsigned short _Fract) */
void test_8_fractional_bits(void) {
    printf("Testing with 8 fractional bits (unsigned short _Fract):\n");
    
    /* Case 1: High part positive - should trigger a_high.sgt(max_r) */
    /* Convert from _Accum (more integer bits) to _Fract (fewer integer bits) */
    unsigned short _Accum a1 = 256.0uhk;  /* 256.0 has high part > 0 */
    unsigned short _Fract f1 = (unsigned short _Fract)a1;
    use_result = f1;
    printf("  Conversion 256.0uhk -> usf: %u (0x%x)\n", (unsigned int)f1, (unsigned int)f1);
    
    /* Case 2: High part zero, low part > 2^8-1 = 255 */
    /* Create value with low part = 256 (exceeds max_s) */
    unsigned int val = 256;  /* 256 > 255 */
    unsigned short _Fract f2 = val / 256.0ur;  /* This should be exactly 1.0 */
    unsigned short _Fract f3 = (unsigned short _Fract)f2;  /* Self-conversion */
    use_result = f3;
    printf("  Value 256/256.0 = %u (0x%x)\n", (unsigned int)f3, (unsigned int)f3);
}

/* i_f_bits = 16 (unsigned _Fract) */
void test_16_fractional_bits(void) {
    printf("\nTesting with 16 fractional bits (unsigned _Fract):\n");
    
    /* High part positive case */
    unsigned _Accum a1 = 65536.0uk;  /* 65536 has high part > 0 */
    unsigned _Fract f1 = (unsigned _Fract)a1;
    use_result = f1;
    printf("  Conversion 65536.0uk -> uf: %u (0x%x)\n", (unsigned int)f1, (unsigned int)f1);
    
    /* High part zero, low part > 2^16-1 = 65535 */
    unsigned int val = 65536;  /* 65536 > 65535 */
    unsigned _Fract f2 = val / 65536.0ur;
    use_result = f2;
    printf("  Value 65536/65536.0 = %u (0x%x)\n", (unsigned int)f2, (unsigned int)f2);
}

/* i_f_bits = 24 (unsigned long _Fract) */
void test_24_fractional_bits(void) {
    printf("\nTesting with 24 fractional bits (unsigned long _Fract):\n");
    
    /* High part positive case - need a very large value */
    unsigned long _Accum a1 = 16777216.0ulk;  /* 2^24 */
    unsigned long _Fract f1 = (unsigned long _Fract)a1;
    use_result = f1;
    printf("  Conversion 16777216.0ulk -> ulf: %u (0x%x)\n", (unsigned int)f1, (unsigned int)f1);
    
    /* High part zero, low part > 2^24-1 = 16777215 */
    unsigned long val = 16777216UL;  /* 2^24 */
    unsigned long _Fract f2 = val / 16777216.0ulr;
    use_result = f2;
    printf("  Value 2^24/2^24.0 = %u (0x%x)\n", (unsigned int)f2, (unsigned int)f2);
}

/* Test signed types as well - they use different bounds but same logic */
void test_signed_types(void) {
    printf("\nTesting signed types:\n");
    
    /* Signed short _Fract with 7 fractional bits (plus sign) */
    short _Accum a1 = 128.0hk;  /* Exceeds max for 7 fractional bits */
    short _Fract f1 = (short _Fract)a1;
    use_result = f1;
    printf("  Signed conversion 128.0hk -> sf: %d (0x%x)\n", (int)f1, (unsigned int)f1);
    
    /* Negative overflow case */
    short _Accum a2 = -129.0hk;
    short _Fract f2 = (short _Fract)a2;
    use_result = f2;
    printf("  Signed conversion -129.0hk -> sf: %d (0x%x)\n", (int)f2, (unsigned int)f2);
}

/* Test with saturation attribute */
void test_saturation(void) {
    printf("\nTesting with saturation:\n");
    
    /* These should saturate rather than wrap */
    unsigned short _Accum a1 = 300.0uhk;
    unsigned short _Fract f1 = (unsigned short _Fract)a1;
    use_result = f1;
    printf("  Saturated conversion 300.0uhk -> usf: %u (0x%x)\n", 
           (unsigned int)f1, (unsigned int)f1);
    
    /* Another saturation case */
    unsigned _Accum a2 = 70000.0uk;
    unsigned _Fract f2 = (unsigned _Fract)a2;
    use_result = f2;
    printf("  Saturated conversion 70000.0uk -> uf: %u (0x%x)\n",
           (unsigned int)f2, (unsigned int)f2);
}

/* Test arithmetic operations that can overflow */
void test_arithmetic_overflow(void) {
    printf("\nTesting arithmetic overflow:\n");
    
    /* Addition that overflows */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum = f1 + f2;  /* 1.7 > 1.0 */
    use_result = sum;
    printf("  Addition 0.8ur + 0.9ur = %u (0x%x)\n", (unsigned int)sum, (unsigned int)sum);
    
    /* Multiplication that overflows */
    unsigned short _Fract f3 = 1.5ur;
    unsigned short _Fract f4 = 1.5ur;
    unsigned short _Fract prod = f3 * f4;  /* 2.25 > 1.0 */
    use_result = prod;
    printf("  Multiplication 1.5ur * 1.5ur = %u (0x%x)\n", (unsigned int)prod, (unsigned int)prod);
}

/* Test builtin overflow functions */
void test_builtin_overflow(void) {
    printf("\nTesting builtin overflow detection:\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* This builtin should trigger overflow checking */
    overflow = __builtin_add_overflow(f1, f2, &result);
    use_result = result;
    printf("  __builtin_add_overflow(0.8ur, 0.9ur): result=%u, overflow=%d\n",
           (unsigned int)result, overflow);
    
    /* Multiplication overflow */
    overflow = __builtin_mul_overflow(f1, f2, &result);
    use_result = result;
    printf("  __builtin_mul_overflow(0.8ur, 0.9ur): result=%u, overflow=%d\n",
           (unsigned int)result, overflow);
}

int main(void) {
    int total_tests = 0;
    
    printf("=== Testing fixed-point overflow checking ===\n\n");
    
    /* Run all test cases */
    test_8_fractional_bits();
    total_tests++;
    
    test_16_fractional_bits();
    total_tests++;
    
    test_24_fractional_bits();
    total_tests++;
    
    test_signed_types();
    total_tests++;
    
    test_saturation();
    total_tests++;
    
    test_arithmetic_overflow();
    total_tests++;
    
    test_builtin_overflow();
    total_tests++;
    
    printf("\n=== All %d test cases completed ===\n", total_tests);
    
    /* Return success if we compiled and ran */
    return 0;
}
