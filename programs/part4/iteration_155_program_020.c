/* Test program to trigger 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Define vector types for different architectures */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Approach 1: Complex vector operations with FMA chaining */
__attribute__((noinline))
v4df test_vector_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e) {
    /* Chain multiple FMA operations - may generate complex RTL */
    v4df result;
    
    /* Use __builtin_fma if available, otherwise emulate */
    #ifdef __FMA__
    /* This chained expression could potentially generate many operands */
    result = __builtin_fma(a, b, __builtin_fma(c, d, e));
    #else
    /* Fallback: still creates complex vector operations */
    result = a * b + c * d + e;
    #endif
    
    return result;
}

/* Approach 2: Vector shuffle with large constant mask */
__attribute__((noinline))
v8sf test_vector_shuffle(v8sf a, v8sf b) {
    /* Shuffle with a complex 8-element mask - each index is an operand */
    const int mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    
    /* Using GCC vector extensions for shuffle */
    v8sf result = __builtin_shuffle(a, b, 
        (v8si){mask[0], mask[1], mask[2], mask[3], 
               mask[4], mask[5], mask[6], mask[7]});
    
    return result;
}

/* Approach 3: Inline assembly with exactly 10 operands */
__attribute__((noinline))
uint64_t test_asm_10_operands(uint64_t a, uint64_t b, uint64_t c, 
                              uint64_t d, uint64_t e, uint64_t f,
                              uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* Dummy 10-operand instruction */\n\t"
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
    
    return result;
}

/* Approach 4: Inline assembly with exactly 11 operands */
__attribute__((noinline))
uint64_t test_asm_11_operands(uint64_t a, uint64_t b, uint64_t c, 
                              uint64_t d, uint64_t e, uint64_t f,
                              uint64_t g, uint64_t h, uint64_t i,
                              uint64_t j) {
    uint64_t result;
    
    /* 11 operands: 1 output + 10 inputs */
    asm volatile (
        "/* Dummy 11-operand instruction */\n\t"
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
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Approach 5: Complex constant expression that may not fold immediately */
__attribute__((noinline))
int test_complex_const_expression(void) {
    /* Force compiler to consider this as a complex expression */
    volatile int prevent_folding = 0;
    
    if (__builtin_constant_p(prevent_folding)) {
        /* This branch won't be taken at compile time, but forces RTL gen */
        return 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    } else {
        /* Create a complex expression with many operands */
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
        
        /* Single expression with 11 additions - may generate RTL with many ops */
        return a + b + c + d + e + f + g + h + i + j + k;
    }
}

/* Approach 6: AVX-512 style operations if available */
#ifdef __AVX512F__
__attribute__((noinline))
__m512 test_avx512_complex(__m512 a, __m512 b, __m512 c, __m512 d) {
    /* Complex AVX-512 expression that might use many operands */
    __m512 result = _mm512_fmadd_ps(a, b, _mm512_fmadd_ps(c, d, a));
    result = _mm512_add_ps(result, _mm512_mul_ps(b, c));
    result = _mm512_sub_ps(result, _mm512_div_ps(d, a));
    
    return result;
}
#endif

/* Template approach for C++ */
#ifdef __cplusplus
template<typename T, int N>
T template_complex_operation(T a, T b) {
    /* Complex template operation that might instantiate differently */
    T result = a;
    for (int i = 0; i < N; i++) {
        result = result + b + static_cast<T>(i);
    }
    return result + static_cast<T>(N);
}

/* Instantiate with many parameters */
template int template_complex_operation<int, 10>(int, int);
template int template_complex_operation<int, 11>(int, int);
template float template_complex_operation<float, 10>(float, float);
template float template_complex_operation<float, 11>(float, float);
#endif

/* Main function to ensure code is not eliminated */
int main(void) {
    volatile uint64_t vresult;  /* volatile to prevent optimization */
    
    /* Test 10-operand inline assembly */
    vresult = test_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9);
    
    /* Test 11-operand inline assembly */
    vresult = test_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test complex constant expression */
    vresult = test_complex_const_expression();
    
    /* Test vector operations */
    v4df vec_a = {1.0, 2.0, 3.0, 4.0};
    v4df vec_b = {5.0, 6.0, 7.0, 8.0};
    v4df vec_c = {9.0, 10.0, 11.0, 12.0};
    v4df vec_d = {13.0, 14.0, 15.0, 16.0};
    v4df vec_e = {17.0, 18.0, 19.0, 20.0};
    
    v4df vec_result = test_vector_fma_chain(vec_a, vec_b, vec_c, vec_d, vec_e);
    vresult = (uint64_t)vec_result[0];  /* Use result */
    
    /* Test vector shuffle */
    v8sf vec_f = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec_g = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf shuffle_result = test_vector_shuffle(vec_f, vec_g);
    vresult = (uint64_t)shuffle_result[0];
    
    #ifdef __cplusplus
    /* Test template instantiations */
    int template_result1 = template_complex_operation<int, 10>(5, 3);
    int template_result2 = template_complex_operation<int, 11>(5, 3);
    vresult = template_result1 + template_result2;
    #endif
    
    return (int)vresult % 256;  /* Return non-zero to ensure execution */
}
