/* Test program to trigger overflow checking in fixed-value.cc lines 264-277 */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
#define KEEP(V) asm volatile("" : : "r"(V))

/* Test different i_f_bits scenarios */
void test_i_f_bits_8(void) {
    /* For i_f_bits = 8, max_s = 2^8 - 1 = 255 */
    /* Test case 1: high part positive (triggers a_high.sgt(max_r)) */
    unsigned short _Accum a1 = 256.0uhk;  /* 256 << 8 = 65536 */
    unsigned short _Fract b1 = (unsigned short _Fract)a1;  /* Should overflow */
    KEEP(b1);
    
    /* Test case 2: high part zero, low part > 255 (triggers a_low.ugt(max_s)) */
    unsigned short _Accum a2 = 255.5uhk;  /* 255.5 << 8 = 65408 */
    unsigned short _Fract b2 = (unsigned short _Fract)a2;  /* Should overflow */
    KEEP(b2);
    
    printf("Test i_f_bits=8: b1=%u, b2=%u\n", 
           (unsigned)(b1 * 256), 
           (unsigned)(b2 * 256));
}

void test_i_f_bits_16(void) {
    /* For i_f_bits = 16, max_s = 2^16 - 1 = 65535 */
    unsigned _Accum a1 = 65536.0uhk;  /* High part becomes 1 */
    unsigned _Fract b1 = (unsigned _Fract)a1;  /* Should overflow */
    KEEP(b1);
    
    unsigned _Accum a2 = 65535.5uhk;  /* Low part > 65535 */
    unsigned _Fract b2 = (unsigned _Fract)a2;  /* Should overflow */
    KEEP(b2);
    
    printf("Test i_f_bits=16: b1=%u, b2=%u\n",
           (unsigned)(b1 * 65536),
           (unsigned)(b2 * 65536));
}

void test_i_f_bits_24(void) {
    /* For i_f_bits = 24, max_s = 2^24 - 1 = 16777215 */
    unsigned long _Accum a1 = 16777216.0ulhk;  /* High part becomes 1 */
    unsigned long _Fract b1 = (unsigned long _Fract)a1;  /* Should overflow */
    KEEP(b1);
    
    unsigned long _Accum a2 = 16777215.5ulhk;  /* Low part > 16777215 */
    unsigned long _Fract b2 = (unsigned long _Fract)a2;  /* Should overflow */
    KEEP(b2);
    
    printf("Test i_f_bits=24: b1=%u, b2=%u\n",
           (unsigned)(b1 * 16777216),
           (unsigned)(b2 * 16777216));
}

void test_signed_types(void) {
    /* Test signed types - they use different bounds but same overflow logic */
    short _Accum a1 = 128.0hk;  /* Positive high part for signed */
    short _Fract b1 = (short _Fract)a1;  /* Should overflow */
    KEEP(b1);
    
    short _Accum a2 = 127.5hk;  /* Low part overflow for signed */
    short _Fract b2 = (short _Fract)a2;  /* Should overflow */
    KEEP(b2);
    
    printf("Test signed: b1=%d, b2=%d\n",
           (int)(b1 * 128),
           (int)(b2 * 128));
}

void test_arithmetic_overflow(void) {
    /* Arithmetic operations that overflow */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract sum = f1 + f2;  /* 1.7 > 1.0, should overflow */
    KEEP(sum);
    
    unsigned short _Fract prod = f1 * 2.0ur;  /* 1.6 > 1.0, should overflow */
    KEEP(prod);
    
    printf("Test arithmetic: sum=%u, prod=%u\n",
           (unsigned)(sum * 256),
           (unsigned)(prod * 256));
}

void test_builtin_overflow(void) {
    /* Use builtins that might trigger the overflow checking */
    unsigned short _Fract f1 = 0.8ur;
    unsigned short _Fract f2 = 0.9ur;
    unsigned short _Fract result;
    int overflow;
    
    /* These builtins might use the same overflow checking logic */
    overflow = __builtin_add_overflow(f1, f2, &result);
    KEEP(overflow);
    KEEP(result);
    
    overflow = __builtin_mul_overflow(f1, 2.0ur, &result);
    KEEP(overflow);
    KEEP(result);
    
    printf("Test builtins: add_overflow=%d, mul_overflow=%d\n",
           overflow, overflow);
}

void test_saturation(void) {
    /* Test with saturation attribute */
    unsigned short _Fract __attribute__((saturated)) f1 = 0.8ur;
    unsigned short _Fract __attribute__((saturated)) f2 = 0.9ur;
    unsigned short _Fract __attribute__((saturated)) sum = f1 + f2;
    KEEP(sum);
    
    printf("Test saturation: sum=%u (should be 255/256)\n",
           (unsigned)(sum * 256));
}

void test_mixed_conversions(void) {
    /* Mixed integer and fixed-point conversions */
    unsigned int large_int = 300;
    unsigned short _Fract f = (unsigned short _Fract)large_int;  /* Should overflow */
    KEEP(f);
    
    unsigned short _Fract small_fract = 0.5ur;
    unsigned int i = (unsigned int)small_fract;  /* Should not overflow */
    KEEP(i);
    
    printf("Test mixed: f=%u, i=%u\n",
           (unsigned)(f * 256),
           i);
}

int main(void) {
    int result = 0;
    
    printf("Starting fixed-point overflow tests...\n");
    
    test_i_f_bits_8();
    test_i_f_bits_16();
    test_i_f_bits_24();
    test_signed_types();
    test_arithmetic_overflow();
    test_builtin_overflow();
    test_saturation();
    test_mixed_conversions();
    
    printf("All tests completed.\n");
    
    return result;
}
