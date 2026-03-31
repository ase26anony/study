/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
#if defined(__AVX512F__)
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#elif defined(__AVX__)
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#else
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Method 1: Complex vector operations with chained FMAs */
__attribute__((noinline))
static void test_vector_operations() {
    volatile int result = 0;
    
#if defined(__FMA__)
    /* Chained FMA operations that may generate many operands */
    v4df a = {1.0, 2.0, 3.0, 4.0};
    v4df b = {5.0, 6.0, 7.0, 8.0};
    v4df c = {9.0, 10.0, 11.0, 12.0};
    v4df d = {13.0, 14.0, 15.0, 16.0};
    v4df e = {17.0, 18.0, 19.0, 20.0};
    
    /* Complex expression that might generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    result += (int)a[0];
#endif
    
    (void)result;
}

/* Method 2: Inline assembly with exactly 10 operands */
__attribute__((noinline))
static void test_asm_10_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9)
        : "cc"
    );
    
    volatile int dummy = o0;
    (void)dummy;
}

/* Method 3: Inline assembly with exactly 11 operands */
__attribute__((noinline))
static void test_asm_11_operands() {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, 
            i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "mov %[out], %[in1]\n\t"
        "add %[out], %[out], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (o0)
        : [in1] "r" (i1), [in2] "r" (i2), [in3] "r" (i3),
          [in4] "r" (i4), [in5] "r" (i5), [in6] "r" (i6),
          [in7] "r" (i7), [in8] "r" (i8), [in9] "r" (i9),
          [in10] "r" (i10)
        : "cc"
    );
    
    volatile int dummy = o0;
    (void)dummy;
}

/* Method 4: Complex shuffle/permute with large mask */
__attribute__((noinline))
static void test_vector_shuffle() {
#if defined(__SSE2__) || defined(__AVX__)
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Shuffle with immediate mask - might generate RTL with immediate operands */
    v4sf c = __builtin_shuffle(a, b, 
        (typeof(a)){0, 4, 1, 5});  /* 4-element mask */
    
    volatile float dummy = c[0] + c[1] + c[2] + c[3];
    (void)dummy;
#endif
}

/* Method 5: Target-specific builtins for AVX-512 */
__attribute__((noinline))
static void test_avx512_builtins() {
#if defined(__AVX512F__)
    /* AVX-512 gather instructions have many parameters */
    __m512i index = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512 src = _mm512_set1_ps(1.0f);
    __mmask16 mask = 0xFFFF;
    
    float base[64] = {0};
    __m512 result = _mm512_mask_i32gather_ps(src, mask, index, base, 4);
    
    volatile float dummy = _mm512_cvtss_f32(result);
    (void)dummy;
#endif
}

/* Method 6: Complex constant expression */
__attribute__((noinline))
static int test_complex_const_expr() {
    /* Large constant expression that might not fold immediately */
    const int x = 
        1 + (2 * 3) + (4 << 5) + (6 & 7) + (8 | 9) + 
        10 - 11 + 12 * 13 + 14 / 15 + 16 % 17 + 
        18 ^ 19 + 20 * 21 + 22 + 23 + 24 + 25;
    
    /* Use __builtin_constant_p to potentially prevent folding */
    if (__builtin_constant_p(x)) {
        return x;
    } else {
        return x + 1;
    }
}

/* Method 7: Template-like approach using macros (C version) */
#define GEN_OPERATION(TYPE, N) \
    __attribute__((noinline)) \
    static TYPE operation_##TYPE##_##N(TYPE a, TYPE b) { \
        return a + b + (TYPE)N + (TYPE)(N*2) + (TYPE)(N*3) + \
               (TYPE)(N*4) + (TYPE)(N*5) + (TYPE)(N*6) + \
               (TYPE)(N*7) + (TYPE)(N*8) + (TYPE)(N*9); \
    }

/* Generate multiple instantiations */
GEN_OPERATION(int, 1)
GEN_OPERATION(int, 2)
GEN_OPERATION(int, 3)
GEN_OPERATION(float, 1)
GEN_OPERATION(float, 2)
GEN_OPERATION(float, 3)
GEN_OPERATION(double, 1)
GEN_OPERATION(double, 2)
GEN_OPERATION(double, 3)

/* Main function that exercises all patterns */
int main() {
    int result = 0;
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test inline assembly with 10 and 11 operands */
    test_asm_10_operands();
    test_asm_11_operands();
    
    /* Test vector shuffle */
    test_vector_shuffle();
    
    /* Test AVX-512 builtins if available */
    test_avx512_builtins();
    
    /* Test complex constant expression */
    result += test_complex_const_expr();
    
    /* Test generated operations */
    result += operation_int_1(1, 2);
    result += operation_int_2(2, 3);
    result += operation_int_3(3, 4);
    
    volatile float f1 = operation_float_1(1.0f, 2.0f);
    volatile float f2 = operation_float_2(2.0f, 3.0f);
    volatile float f3 = operation_float_3(3.0f, 4.0f);
    
    volatile double d1 = operation_double_1(1.0, 2.0);
    volatile double d2 = operation_double_2(2.0, 3.0);
    volatile double d3 = operation_double_3(3.0, 4.0);
    
    (void)f1; (void)f2; (void)f3;
    (void)d1; (void)d2; (void)d3;
    
    return result == 0 ? 0 : 1;
}
