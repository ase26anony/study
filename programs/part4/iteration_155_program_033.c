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

#ifdef __SSE2__
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Strategy 1: Complex vector operations with FMA chaining */
#ifdef __FMA__
__attribute__((noinline))
static void test_vector_fma_chain(void) {
#ifdef __AVX512F__
    v8df a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    v8df b = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    v8df c = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    v8df d = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    
    /* Complex FMA chain that might generate multi-operand RTL */
    a = __builtin_fma(b, c, __builtin_fma(d, a, __builtin_fma(b, d, c)));
    
    volatile v8df sink = a;
    (void)sink;
#endif
}
#endif

/* Strategy 2: Inline assembly with exactly 10 and 11 operands */
__attribute__((noinline))
static void test_inline_asm_10_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out1, out2;
    
    /* 10 operands: 1 output + 9 inputs */
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0"
        : "=r" (out1)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4),
          "r" (in5), "r" (in6), "r" (in7), "r" (in8), "r" (in9)
        : "cc"
    );
    
    /* 11 operands: 2 outputs + 9 inputs */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %2, %0\n\t"
        "mov %3, %1\n\t"
        "add %4, %0\n\t"
        "add %5, %1\n\t"
        "add %6, %0\n\t"
        "add %7, %1\n\t"
        "add %8, %0\n\t"
        "add %9, %1\n\t"
        "add %10, %0"
        : "=r" (out1), "=r" (out2)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4), "r" (in5),
          "r" (in6), "r" (in7), "r" (in8), "r" (in9)
        : "cc"
    );
    
    volatile int64_t sink = out1 + out2;
    (void)sink;
}

/* Strategy 3: Complex shuffle/permute with large masks */
#ifdef __AVX512F__
__attribute__((noinline))
static void test_large_shuffle(void) {
    v16sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v16sf v2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Large shuffle mask - 16 elements */
    const int mask[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    
    /* Use __builtin_shuffle with both vectors and large mask */
    v16sf result = __builtin_shuffle(v1, v2, mask[0], mask[1], mask[2], mask[3],
                                     mask[4], mask[5], mask[6], mask[7],
                                     mask[8], mask[9], mask[10], mask[11],
                                     mask[12], mask[13], mask[14], mask[15]);
    
    volatile v16sf sink = result;
    (void)sink;
}
#endif

/* Strategy 4: Target-specific builtins for multi-operand instructions */
#ifdef __AVX512F__
#include <immintrin.h>
__attribute__((noinline))
static void test_avx512_gather(void) {
    __m512i index = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    float base[64] = {0};
    __mmask16 mask = 0xFFFF;
    
    /* AVX-512 gather has multiple operands: source, mask, index, scale, base, etc. */
    __m512 result = _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask, index, base, 4);
    
    volatile __m512 sink = result;
    (void)sink;
}
#endif

/* Strategy 5: Complex constant expression that might not fold immediately */
__attribute__((noinline, noipa))
static int test_complex_const_expression(void) {
    /* Force compiler to consider this as non-constant initially */
    if (__builtin_constant_p(0)) {
        return 0;
    }
    
    /* Large expression with many constants - might generate RTL with many immediates */
    int x = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 
            11 + 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 + 20;
    
    /* Use in a way that prevents dead code elimination */
    volatile int sink = x;
    return sink;
}

/* C++ template version for more instantiations */
#ifdef __cplusplus
template<typename T, int N>
__attribute__((noinline))
T template_multi_operand(T a, T b) {
    /* Complex expression that might generate multi-operand RTL */
    return a + b + (T)N + (T)(N+1) + (T)(N+2) + (T)(N+3) + (T)(N+4) + 
           (T)(N+5) + (T)(N+6) + (T)(N+7) + (T)(N+8) + (T)(N+9);
}

static void test_cpp_templates(void) {
    /* Instantiate with different types and constants */
    float f1 = template_multi_operand<float, 1>(1.0f, 2.0f);
    float f2 = template_multi_operand<float, 100>(3.0f, 4.0f);
    double d1 = template_multi_operand<double, 1>(1.0, 2.0);
    double d2 = template_multi_operand<double, 100>(3.0, 4.0);
    
    volatile float fsink = f1 + f2;
    volatile double dsink = d1 + d2;
    (void)fsink;
    (void)dsink;
}
#endif

/* Main function that calls all test patterns */
int main(void) {
    /* Test inline assembly with 10/11 operands */
    test_inline_asm_10_operands();
    
#ifdef __FMA__
    /* Test vector FMA chains */
    test_vector_fma_chain();
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 specific patterns */
    test_large_shuffle();
    test_avx512_gather();
#endif
    
    /* Test complex constant expressions */
    int const_result = test_complex_const_expression();
    
#ifdef __cplusplus
    /* Test C++ template patterns */
    test_cpp_templates();
#endif
    
    /* Ensure results are used */
    volatile int final_sink = const_result;
    
    return final_sink != 0 ? 0 : 0;
}
