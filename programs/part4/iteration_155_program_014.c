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

/* Strategy 1: Complex inline assembly with many operands */
void test_inline_asm_10_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    int o0;
    
    /* Initialize to prevent constant propagation */
    i0 = rand(); i1 = rand(); i2 = rand(); i3 = rand(); i4 = rand();
    i5 = rand(); i6 = rand(); i7 = rand(); i8 = rand(); i9 = rand();
    
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
        : [in1] "r" (i0), [in2] "r" (i1), [in3] "r" (i2),
          [in4] "r" (i3), [in5] "r" (i4), [in6] "r" (i5),
          [in7] "r" (i6), [in8] "r" (i7), [in9] "r" (i8)
        : "cc"
    );
    
    volatile int sink = o0; /* Prevent dead code elimination */
    (void)sink;
}

void test_inline_asm_11_operands(void) {
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int o0;
    
    i0 = rand(); i1 = rand(); i2 = rand(); i3 = rand(); i4 = rand();
    i5 = rand(); i6 = rand(); i7 = rand(); i8 = rand(); i9 = rand();
    i10 = rand();
    
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
        : [in1] "r" (i0), [in2] "r" (i1), [in3] "r" (i2),
          [in4] "r" (i3), [in5] "r" (i4), [in6] "r" (i5),
          [in7] "r" (i6), [in8] "r" (i7), [in9] "r" (i8),
          [in10] "r" (i9)
        : "cc"
    );
    
    volatile int sink = o0;
    (void)sink;
}

/* Strategy 2: Complex vector operations with FMA chaining */
#ifdef __FMA__
void test_vector_fma_chaining(void) {
#ifdef __AVX__
    v4df a, b, c, d, e, f, g, h, i, j;
    
    /* Initialize vectors with random-ish data */
    for (int k = 0; k < 4; k++) {
        ((double*)&a)[k] = rand() / (double)RAND_MAX;
        ((double*)&b)[k] = rand() / (double)RAND_MAX;
        ((double*)&c)[k] = rand() / (double)RAND_MAX;
        ((double*)&d)[k] = rand() / (double)RAND_MAX;
        ((double*)&e)[k] = rand() / (double)RAND_MAX;
        ((double*)&f)[k] = rand() / (double)RAND_MAX;
        ((double*)&g)[k] = rand() / (double)RAND_MAX;
        ((double*)&h)[k] = rand() / (double)RAND_MAX;
        ((double*)&i)[k] = rand() / (double)RAND_MAX;
        ((double*)&j)[k] = rand() / (double)RAND_MAX;
    }
    
    /* Complex FMA chain that might generate many operands */
    v4df result = __builtin_fma(a, b, 
                     __builtin_fma(c, d,
                         __builtin_fma(e, f,
                             __builtin_fma(g, h,
                                 __builtin_fma(i, j, a)))));
    
    volatile v4df sink = result;
    (void)sink;
#endif
}
#endif

/* Strategy 3: Complex shuffle/permute with large masks */
void test_vector_shuffle(void) {
#ifdef __AVX__
    v8sf v1, v2;
    
    for (int k = 0; k < 8; k++) {
        ((float*)&v1)[k] = rand() / (float)RAND_MAX;
        ((float*)&v2)[k] = rand() / (float)RAND_MAX;
    }
    
    /* Shuffle with a complex mask - each element is an immediate */
    v8sf shuffled = __builtin_shuffle(v1, v2, 
        (int[8]){7, 6, 5, 4, 3, 2, 1, 0});
    
    /* Additional operations to increase operand count */
    shuffled = __builtin_shuffle(shuffled, v1,
        (int[8]){0, 1, 2, 3, 4, 5, 6, 7});
    
    volatile v8sf sink = shuffled;
    (void)sink;
#endif
}

/* Strategy 4: Target-specific builtins for x86 */
#ifdef __x86_64__
#include <x86intrin.h>

void test_avx512_gather(void) {
#ifdef __AVX512F__
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float* base = (float*)malloc(64 * sizeof(float));
    
    for (int i = 0; i < 64; i++) {
        base[i] = i * 0.1f;
    }
    
    __mmask16 mask = 0xFFFF;
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
    
    volatile __m512 sink = result;
    (void)sink;
    
    free(base);
#endif
}
#endif

/* Strategy 5: Complex constant expression */
int test_complex_const_expression(void) {
    /* Force compiler to generate RTL for complex constant computation */
    int x = 
        (__builtin_constant_p(0) ? 1 : 0) +
        (__builtin_constant_p(1) ? 2 : 0) +
        (__builtin_constant_p(2) ? 3 : 0) +
        (__builtin_constant_p(3) ? 4 : 0) +
        (__builtin_constant_p(4) ? 5 : 0) +
        (__builtin_constant_p(5) ? 6 : 0) +
        (__builtin_constant_p(6) ? 7 : 0) +
        (__builtin_constant_p(7) ? 8 : 0) +
        (__builtin_constant_p(8) ? 9 : 0) +
        (__builtin_constant_p(9) ? 10 : 0) +
        (__builtin_constant_p(10) ? 11 : 0);
    
    return x;
}

/* C++ template version for more instantiations */
#ifdef __cplusplus
template<typename T, int N>
T template_operation(T a, T b) {
    /* Complex operation that might generate many operands */
    return a + b + static_cast<T>(N) + 
           static_cast<T>(N+1) + static_cast<T>(N+2) +
           static_cast<T>(N+3) + static_cast<T>(N+4) +
           static_cast<T>(N+5) + static_cast<T>(N+6) +
           static_cast<T>(N+7);
}

void test_template_instantiations(void) {
    int int_result = template_operation<int, 1>(1, 2) +
                     template_operation<int, 2>(3, 4) +
                     template_operation<int, 3>(5, 6);
    
    float float_result = template_operation<float, 10>(1.0f, 2.0f) +
                         template_operation<float, 20>(3.0f, 4.0f);
    
    volatile int sink1 = int_result;
    volatile float sink2 = float_result;
    (void)sink1; (void)sink2;
}
#endif

int main(void) {
    srand(42); /* Deterministic seed */
    
    /* Test all strategies */
    test_inline_asm_10_operands();
    test_inline_asm_11_operands();
    
#ifdef __FMA__
    test_vector_fma_chaining();
#endif
    
    test_vector_shuffle();
    
#ifdef __x86_64__
    test_avx512_gather();
#endif
    
    int const_result = test_complex_const_expression();
    
#ifdef __cplusplus
    test_template_instantiations();
#endif
    
    /* Use results to prevent dead code elimination */
    volatile int final_sink = const_result;
    (void)final_sink;
    
    return 0;
}
