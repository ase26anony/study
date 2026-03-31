/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fsaturated-arithmetic -o test_fixed_overflow test_fixed_overflow.c */

#include <stdio.h>

/* Helper to prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different fractional bit counts */
void test_i_f_bits_8(void) {
    printf("Testing i_f_bits = 8 (unsigned short _Fract):\n");
    
    /* For i_f_bits=8, max_s = 2^8 - 1 = 255 */
    /* Case 1: High part positive (should trigger a_high.sgt(max_r)) */
    unsigned short _Accum a1 = 256.0hk;  /* 256 = 1 << 8, high part = 1 */
    unsigned short _Fract b1 = (unsigned short _Fract)a1;  /* Should saturate to max */
    
    printf("  Case 1 - High part positive: 256.0hk -> usf = %hu/256\n", 
           (unsigned short)(b1 * 256.0ur));
    KEEP(b1);
    
    /* Case 2: High part zero, low part > 255 (should trigger a_low.ugt(max_s)) */
    unsigned short _Accum a2 = 255.5hk;  /* 255.5, high part = 0, low part > 255 */
    unsigned short _Fract b2 = (unsigned short _Fract)a2;  /* Should saturate */
    
    printf("  Case 2 - Low part overflow: 255.5hk -> usf = %hu/256\n",
           (unsigned short)(b2 * 256.0ur));
    KEEP(b2);
}

void test_i_f_bits_16(void) {
    printf("Testing i_f_bits = 16 (unsigned _Fract):\n");
    
    /* For i_f_bits=16, max_s = 2^16 - 1 = 65535 */
    /* Case 1: High part positive */
    unsigned _Accum a1 = 65536.0uk;  /* 65536 = 1 << 16 */
    unsigned _Fract b1 = (unsigned _Fract)a1;
    
    printf("  Case 1 - High part positive: 65536.0uk -> uf = %u/65536\n",
           (unsigned)(b1 * 65536.0ur));
    KEEP(b1);
    
    /* Case 2: High part zero, low part > 65535 */
    unsigned _Accum a2 = 65535.5uk;  /* Just over max */
    unsigned _Fract b2 = (unsigned _Fract)a2;
    
    printf("  Case 2 - Low part overflow: 65535.5uk -> uf = %u/65536\n",
           (unsigned)(b2 * 65536.0ur));
    KEEP(b2);
}

void test_i_f_bits_24(void) {
    printf("Testing i_f_bits = 24 (unsigned long _Fract):\n");
    
    /* For i_f_bits=24, max_s = 2^24 - 1 = 16777215 */
    /* Use long accum to get enough bits */
    unsigned long _Accum a1 = 16777216.0ulk;  /* 1 << 24 */
    unsigned long _Fract b1 = (unsigned long _Fract)a1;
    
    printf("  Case 1 - High part positive: 16777216.0ulk -> ulf\n");
    KEEP(b1);
    
    unsigned long _Accum a2 = 16777215.5ulk;  /* Just over max */
    unsigned long _Fract b2 = (unsigned long _Fract)a2;
    
    printf("  Case 2 - Low part overflow: 16777215.5ulk -> ulf\n");
    KEEP(b2);
}

/* Test signed types as well */
void test_signed_i_f_bits_8(void) {
    printf("Testing signed i_f_bits = 8 (short _Fract):\n");
    
    /* For signed, the bounds are different but the same overflow logic applies */
    short _Accum a1 = 128.0hk;  /* Max positive for 8 fractional bits is 127/128 */
    short _Fract b1 = (short _Fract)a1;  /* Should saturate to max */
    
    printf("  Case 1 - Positive overflow: 128.0hk -> sf = %d/128\n",
           (short)(b1 * 128.0r));
    KEEP(b1);
    
    short _Accum a2 = -129.0hk;  /* Min negative for 8 fractional bits is -128/128 = -1 */
    short _Fract b2 = (short _Fract)a2;  /* Should saturate to min */
    
    printf("  Case 2 - Negative overflow: -129.0hk -> sf = %d/128\n",
           (short)(b2 * 128.0r));
    KEEP(b2);
}

/* Test with arithmetic operations that might overflow */
void test_arithmetic_overflow(void) {
    printf("Testing arithmetic overflow:\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.5ur;
    
    /* This addition might overflow depending on saturation mode */
    unsigned short _Fract sum = f1 + f2;  /* 0.8 + 0.5 = 1.3 > 1.0 */
    
    printf("  Addition: 0.8ur + 0.5ur = %hu/256 (expected 255/256 with saturation)\n",
           (unsigned short)(sum * 256.0ur));
    KEEP(sum);
    
    /* Test multiplication overflow */
    unsigned short _Fract f3 = 1.5ur;  /* Will be clamped to 1.0 */
    unsigned short _Fract f4 = 1.2ur;  /* Will be clamped to 1.0 */
    unsigned short _Fract prod = f3 * f4;
    
    printf("  Multiplication: 1.5ur * 1.2ur = %hu/256\n",
           (unsigned short)(prod * 256.0ur));
    KEEP(prod);
}

/* Test with builtins for overflow detection */
void test_builtin_overflow(void) {
    printf("Testing builtin overflow detection:\n");
    
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.5ur;
    unsigned short _Fract result;
    int overflow;
    
    /* Use generic overflow builtin - compiler should handle fixed-point */
    overflow = __builtin_add_overflow(f1, f2, &result);
    
    printf("  __builtin_add_overflow(0.8ur, 0.5ur): overflow=%d, result=%hu/256\n",
           overflow, (unsigned short)(result * 256.0ur));
    KEEP(result);
    KEEP(overflow);
}

/* Test various conversions that should trigger the bounds check */
void test_mixed_conversions(void) {
    printf("Testing mixed type conversions:\n");
    
    /* Integer to fixed-point with potential overflow */
    unsigned int large_int = 300;
    unsigned short _Fract from_int = (unsigned short _Fract)large_int;
    
    printf("  int %u -> usf = %hu/256\n",
           large_int, (unsigned short)(from_int * 256.0ur));
    KEEP(from_int);
    
    /* Float to fixed-point */
    float large_float = 1.5f;
    unsigned short _Fract from_float = (unsigned short _Fract)large_float;
    
    printf("  float %.2f -> usf = %hu/256\n",
           large_float, (unsigned short)(from_float * 256.0ur));
    KEEP(from_float);
}

int main(void) {
    int test_result = 0;
    
    printf("=== Testing fixed-point overflow bounds checking ===\n\n");
    
    /* Run all tests */
    test_i_f_bits_8();
    printf("\n");
    
    test_i_f_bits_16();
    printf("\n");
    
    test_i_f_bits_24();
    printf("\n");
    
    test_signed_i_f_bits_8();
    printf("\n");
    
    test_arithmetic_overflow();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_mixed_conversions();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    /* Return 0 if all tests executed without runtime errors */
    return test_result;
}
