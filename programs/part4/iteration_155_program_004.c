/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Vector types for different architectures */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

#ifdef __AVX__
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
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
    
    /* Chain multiple FMA operations - may generate RTL with many operands */
    a = __builtin_fma(b, c, __builtin_fma(d, e, a));
    
    /* Use volatile to prevent optimization */
    volatile v8df *volatile ptr = &a;
    (void)ptr;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
static void test_inline_asm_10_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* dummy 10-operand asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (o0)
        : "r" (i1), "r" (i2), "r" (i3), "r" (i4), 
          "r" (i5), "r" (i6), "r" (i7), "r" (i8), "r" (i9)
        : "cc"
    );
    
    volatile int64_t *volatile ptr = &o0;
    (void)ptr;
}

static void test_inline_asm_11_operands(void) {
    int64_t o0, i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int64_t i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* dummy 11-operand asm */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r" (o0)
        : "r" (i1), "r" (i2), "r" (i3), "r" (i4), 
          "r" (i5), "r" (i6), "r" (i7), "r" (i8), 
          "r" (i9), "r" (i10)
        : "cc"
    );
    
    volatile int64_t *volatile ptr = &o0;
    (void)ptr;
}

/* Strategy 3: Complex shuffle/permute with large masks */
#ifdef __AVX512F__
static void test_vector_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Large shuffle mask - may generate many immediate operands */
    const int mask[16] = {0, 16, 1, 17, 2, 18, 3, 19, 
                          4, 20, 5, 21, 6, 22, 7, 23};
    
    v16sf result = __builtin_shuffle(v1, v2, mask);
    
    volatile v16sf *volatile ptr = &result;
    (void)ptr;
}
#endif

/* Strategy 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
static void test_avx512_gather(void) {
    /* AVX-512 gather can have many operands: base, scale, index, mask, etc. */
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
    
    volatile __m512 *volatile ptr = &result;
    (void)ptr;
}
#endif

/* Strategy 5: Complex constant expression that may not fold immediately */
static int test_complex_const_expression(void) {
    /* Use __builtin_constant_p to potentially prevent folding */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    if (__builtin_constant_p(a)) {
        /* This branch may generate RTL for the complex expression */
        return a + b + c + d + e + f + g + h + i + j + 11;
    } else {
        /* Force compiler to consider both paths */
        return a * b * c * d * e * f * g * h * i * j;
    }
}

/* C++ template version for multiple instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that may generate many operands */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + 
           (T)(N+4) + (T)(N+5) + (T)(N+6) + (T)(N+7);
}

static void test_cpp_templates(void) {
    /* Instantiate with multiple types and values */
    int r1 = template_operation<int, 1>(10, 20);
    float r2 = template_operation<float, 2>(10.5f, 20.5f);
    double r3 = template_operation<double, 3>(10.5, 20.5);
    
    volatile int *volatile p1 = &r1;
    volatile float *volatile p2 = &r2;
    volatile double *volatile p3 = &r3;
    (void)p1; (void)p2; (void)p3;
}
#endif

/* Main function that calls all test patterns */
int main(void) {
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Test inline assembly patterns */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
    /* Test vector operations if supported */
#ifdef __FMA__
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    test_vector_shuffle();
    test_avx512_gather();
#endif
    
    /* Test complex constant expression */
    int result = test_complex_const_expression();
    
#ifdef __cplusplus
    test_cpp_templates();
#endif
    
    /* Use result to prevent dead code elimination */
    volatile int *volatile ptr = &result;
    (void)ptr;
    
    return 0;
}
