/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Vector types for different architectures */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Strategy 1: Complex vector operations with FMA chaining */
__attribute__((noinline))
v4df vector_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* This may generate RTL with many operands when optimized */
    return __builtin_fma(a, b, __builtin_fma(c, d, e));
}

/* Strategy 2: Vector permutation with large constant mask */
__attribute__((noinline))
v8sf vector_shuffle_complex(v8sf a, v8sf b) {
    /* Large shuffle mask - 8 elements each requiring immediate operands */
    return __builtin_shuffle(a, b, 
        (v8si){7, 6, 5, 4, 3, 2, 1, 0});
}

/* Strategy 3: Inline assembly with exactly 10 operands */
__attribute__((noinline))
int asm_10_operands(int a, int b, int c, int d, int e, 
                    int f, int g, int h, int i, int j) {
    int result;
    /* 10 explicit operands: 1 output + 9 inputs */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i)
        : "cc"
    );
    return result + j; /* Makes 11th operand in some representations */
}

/* Strategy 4: Inline assembly with exactly 11 operands */
__attribute__((noinline))
int asm_11_operands(int a, int b, int c, int d, int e,
                    int f, int g, int h, int i, int j, int k) {
    int result;
    /* 11 explicit operands: 1 output + 10 inputs */
    asm volatile (
        "imul %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    return result + k;
}

/* Strategy 5: Complex constant expression that may not fold immediately */
__attribute__((noinline))
int complex_const_expression(void) {
    /* Force compiler to consider this as non-trivial constant expression */
    if (__builtin_constant_p(0)) {
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* This branch creates RTL with many immediate operands */
        int x = 1, y = 2, z = 3, w = 4, v = 5, u = 6, t = 7, s = 8, r = 9, q = 10, p = 11;
        return x + y + z + w + v + u + t + s + r + q + p;
    }
}

/* Strategy 6: Target-specific builtins (x86 AVX-512 example) */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
__m512i avx512_gather_like(__m512i index, __m512i mask, const int* base) {
    /* AVX-512 gather has multiple operands: base, scale, index, mask, etc. */
    return _mm512_maskz_expand_epi32(mask, _mm512_loadu_epi32(base));
}
#endif

/* Strategy 7: Template/generic approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Multiple instantiations with different constant operands */
    return a + b + T(N) + T(N+1) + T(N+2) + T(N+3) + T(N+4) + 
           T(N+5) + T(N+6) + T(N+7) + T(N+8);
}

/* Instantiate template with many different parameters */
template int template_operation<int, 1>(int, int);
template int template_operation<int, 2>(int, int);
template int template_operation<int, 3>(int, int);
template float template_operation<float, 1>(float, float);
template float template_operation<float, 2>(float, float);
#endif

/* Main function that uses all patterns */
int main(void) {
    volatile int result = 0;
    
    /* Test vector operations */
    v4df v1 = {1.0, 2.0, 3.0, 4.0};
    v4df v2 = {5.0, 6.0, 7.0, 8.0};
    v4df v3 = {9.0, 10.0, 11.0, 12.0};
    v4df v4 = {13.0, 14.0, 15.0, 16.0};
    v4df v5 = {17.0, 18.0, 19.0, 20.0};
    
    v4df vres = vector_fma_chain(v1, v2, v3, v4, v5);
    result += (int)vres[0];
    
    /* Test inline assembly with 10 operands */
    result += asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test inline assembly with 11 operands */
    result += asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test complex constant expression */
    result += complex_const_expression();
    
    /* Test vector shuffle */
    v8sf sv1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf sv2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf svres = vector_shuffle_complex(sv1, sv2);
    result += (int)svres[0];
    
#ifdef __cplusplus
    /* Test template instantiations */
    result += template_operation<int, 1>(1, 2);
    result += template_operation<int, 2>(3, 4);
    result += template_operation<float, 1>(1.5f, 2.5f);
#endif
    
    printf("Result: %d\n", result);
    return 0;
}
