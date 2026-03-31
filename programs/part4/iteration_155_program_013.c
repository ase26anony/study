/* Test program to cover 10/11 operand expansion cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>

/* Define vector types for different architectures */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

#ifdef __AVX__
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    v8df e = {5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    
    /* Chain multiple FMA operations - may generate complex RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    volatile v8df result = a; /* Prevent optimization */
    (void)result;
#endif
}
#endif

/* Strategy 2: Inline assembly with many operands */
static void test_many_operand_asm(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
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
    
    volatile int64_t check = o0;
    (void)check;
}

/* Strategy 3: Complex shuffle/permute operations */
#ifdef __AVX512F__
static void test_complex_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Complex shuffle with many constant indices */
    int16_t mask[16] = {0, 16, 1, 17, 2, 18, 3, 19,
                        4, 20, 5, 21, 6, 22, 7, 23};
    
    /* This may generate RTL with many immediate operands */
    v16sf result = __builtin_shufflevector(v1, v2, 
        0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
    
    volatile v16sf res = result;
    (void)res;
}
#endif

/* Strategy 4: Complex constant expressions */
static int test_complex_const_expr(void) {
    /* Force compiler to handle complex constant expression */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    
    /* Use __builtin_constant_p to potentially generate RTL for both paths */
    if (__builtin_constant_p(x)) {
        x = x * 2;
    } else {
        x = x / 2;
    }
    
    /* Chain more operations */
    x = (((((((((x + 1) + 2) + 3) + 4) + 5) + 6) + 7) + 8) + 9) + 10;
    
    return x;
}

/* Strategy 5: Template/generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex expression that may expand to many operands */
    return a + b + N + (N+1) + (N+2) + (N+3) + (N+4) + 
           (N+5) + (N+6) + (N+7) + (N+8) + (N+9);
}

static void test_template_operations(void) {
    /* Instantiate with different types and constants */
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    volatile int check1 = r1;
    volatile float check2 = r2;
    volatile double check3 = r3;
    (void)check1; (void)check2; (void)check3;
}
#endif

/* Strategy 6: AVX-512 specific builtins with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    /* Gather instruction with multiple parameters */
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
    
    volatile __m512 res = result;
    (void)res;
}
#endif

/* Main function that exercises all strategies */
int main(void) {
    /* Test inline assembly with many operands */
    test_many_operand_asm();
    
    /* Test complex constant expressions */
    int x = test_complex_const_expr();
    
#ifdef __FMA__
    /* Test vector FMA chaining if supported */
    test_vector_fma_chain();
#endif

#ifdef __AVX512F__
    /* Test AVX-512 specific operations */
    test_complex_shuffle();
    test_avx512_gather();
#endif

#ifdef __cplusplus
    /* Test template operations for C++ */
    test_template_operations();
#endif
    
    /* Use result to prevent dead code elimination */
    volatile int result = x;
    return result != 0 ? 0 : 1;
}
