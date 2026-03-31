/* Test program to trigger uncovered lines in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -fdump-tree-all test.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Test different i_f_bits values through different fixed-point types */

/* i_f_bits = 8 (unsigned short _Fract) */
void test_8_fractional_bits(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract):\n");
    
    /* Case 1: High part positive (a_high > max_r where max_r = 0) */
    /* Convert from _Accum (16 integer bits) to _Fract (0 integer bits) */
    unsigned short _Accum usa = 256.0hk;  /* 256.0 exceeds range of unsigned short _Fract */
    unsigned short _Fract usf1 = (unsigned short _Fract)usa;
    
    /* Use the result to prevent dead code elimination */
    global_counter += (int)(usf1 * 256);
    printf("  Conversion 256.0hk -> usf: %u (0x%x)\n", 
           (unsigned)(usf1 * 256), (unsigned)(usf1 * 256));
    
    /* Case 2: High part zero, low part > max_s where max_s = 2^8 - 1 = 255 */
    /* 1.0 is 256 in 8.8 format, which has low part = 0, but we need low part > 255 */
    /* Let's try a value slightly less than 1.0 but with low part > 255 */
    unsigned _Accum ua = 0.9999ur * 256;  /* Close to 255.97 in 8.8 format */
    unsigned short _Fract usf2 = (unsigned short _Fract)ua;
    
    global_counter += (int)(usf2 * 256);
    printf("  Conversion ~0.9999ur -> usf: %u (0x%x)\n",
           (unsigned)(usf2 * 256), (unsigned)(usf2 * 256));
}

/* i_f_bits = 16 (unsigned _Fract) */
void test_16_fractional_bits(void) {
    printf("\nTesting i_f_bits = 16 (unsigned _Fract):\n");
    
    /* Case 1: High part positive */
    unsigned _Accum ua = 65536.0ur;  /* 65536.0 exceeds range of unsigned _Fract */
    unsigned _Fract uf1 = (unsigned _Fract)ua;
    
    global_counter += (int)(uf1 * 65536);
    printf("  Conversion 65536.0ur -> uf: %u (0x%x)\n",
           (unsigned)(uf1 * 65536), (unsigned)(uf1 * 65536));
    
    /* Case 2: High part zero, low part > 65535 */
    /* Create a value with low part > 65535 in 16.16 format */
    unsigned long _Accum ula = 0.999999ur * 65536;  /* Close to 65535.999 in 16.16 format */
    unsigned _Fract uf2 = (unsigned _Fract)ula;
    
    global_counter += (int)(uf2 * 65536);
    printf("  Conversion ~0.999999ur -> uf: %u (0x%x)\n",
           (unsigned)(uf2 * 65536), (unsigned)(uf2 * 65536));
}

/* i_f_bits = 24 (unsigned long _Fract) */
void test_24_fractional_bits(void) {
    printf("\nTesting i_f_bits = 24 (unsigned long _Fract):\n");
    
    /* Case 1: High part positive */
    unsigned long _Accum ula = 16777216.0ulk;  /* 2^24 */
    unsigned long _Fract ulf1 = (unsigned long _Fract)ula;
    
    global_counter += (int)(ulf1 * 16777216);
    printf("  Conversion 16777216.0ulk -> ulf: %u (0x%x)\n",
           (unsigned)(ulf1 * 16777216), (unsigned)(ulf1 * 16777216));
}

/* Test signed types as well */
void test_signed_types(void) {
    printf("\nTesting signed types:\n");
    
    /* Signed short _Fract (i_f_bits = 7 for signed?) */
    /* Actually, for signed types, the logic might be different */
    /* but let's test overflow in positive direction */
    short _Accum sa = 128.0hk;  /* Maximum for signed 8-bit is 127 */
    short _Fract sf1 = (short _Fract)sa;
    
    global_counter += (int)(sf1 * 128);
    printf("  Signed conversion 128.0hk -> sf: %d (0x%x)\n",
           (int)(sf1 * 128), (int)(sf1 * 128));
    
    /* Test negative overflow too */
    short _Accum sa2 = -129.0hk;  /* Minimum for signed 8-bit is -128 */
    short _Fract sf2 = (short _Fract)sa2;
    
    global_counter += (int)(sf2 * 128);
    printf("  Signed conversion -129.0hk -> sf: %d (0x%x)\n",
           (int)(sf2 * 128), (int)(sf2 * 128));
}

/* Test using builtins that might trigger overflow checking */
void test_builtin_overflow(void) {
    printf("\nTesting builtin overflow operations:\n");
    
    unsigned short _Fract f1 = 0.5ur;
    unsigned short _Fract f2 = 0.6ur;
    unsigned short _Fract result;
    
    /* This addition might overflow 1.0 */
    result = f1 + f2;
    
    global_counter += (int)(result * 256);
    printf("  Addition 0.5ur + 0.6ur = %u (0x%x)\n",
           (unsigned)(result * 256), (unsigned)(result * 256));
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 0.9ur;
    unsigned short _Fract f4 = 0.9ur;
    result = f3 * f4;
    
    global_counter += (int)(result * 256);
    printf("  Multiplication 0.9ur * 0.9ur = %u (0x%x)\n",
           (unsigned)(result * 256), (unsigned)(result * 256));
}

/* Test with saturation attribute */
__attribute__((saturated)) unsigned short _Fract sat_add(unsigned short _Fract a, 
                                                         unsigned short _Fract b) {
    return a + b;
}

void test_saturation(void) {
    printf("\nTesting saturation behavior:\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    
    /* This should saturate to 1.0ur if saturation is enabled */
    unsigned short _Fract result = sat_add(f1, f2);
    
    global_counter += (int)(result * 256);
    printf("  Saturated add 0.8ur + 0.9ur = %u (0x%x)\n",
           (unsigned)(result * 256), (unsigned)(result * 256));
}

/* Test various conversions that should trigger the bound check */
void test_mixed_conversions(void) {
    printf("\nTesting mixed conversions:\n");
    
    /* Integer to fixed-point with potential overflow */
    unsigned int large_int = 300;  /* > 255 for 8 fractional bits */
    unsigned short _Fract f_from_int = (unsigned short _Fract)large_int;
    
    global_counter += (int)(f_from_int * 256);
    printf("  int 300 -> usf: %u (0x%x)\n",
           (unsigned)(f_from_int * 256), (unsigned)(f_from_int * 256));
    
    /* Float to fixed-point */
    float f = 1.5f;
    unsigned short _Fract f_from_float = (unsigned short _Fract)f;
    
    global_counter += (int)(f_from_float * 256);
    printf("  float 1.5 -> usf: %u (0x%x)\n",
           (unsigned)(f_from_float * 256), (unsigned)(f_from_float * 256));
}

int main(void) {
    printf("Starting fixed-point overflow tests...\n");
    
    test_8_fractional_bits();
    test_16_fractional_bits();
    test_24_fractional_bits();
    test_signed_types();
    test_builtin_overflow();
    test_saturation();
    test_mixed_conversions();
    
    printf("\nAll tests completed. Global counter: %d\n", global_counter);
    
    /* Return non-zero if any test produced unexpected results */
    /* In practice, you'd want to compare against expected saturated values */
    return 0;
}
