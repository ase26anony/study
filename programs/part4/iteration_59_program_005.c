/* Test program to cover 10 and 11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* For ARM NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
static void test_vector_operations(void) {
    /* Approach 1: Complex vector operations using GCC extensions */
    v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    /* Complex expression that might expand to multi-operand instructions */
    v8sf result = a * b + c * d + a * c + b * d;
    
    /* Use result to prevent elimination */
    volatile v8sf sink = result;
    (void)sink;
    
#ifdef __AVX512F__
    /* Approach 2: AVX-512 masked operations with many operands */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    
    /* Create a complex mask */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 pattern */
    
    /* Use masked FMA - this often expands to multiple operands */
    __m512 avx_result = _mm512_mask_fmadd_ps(avx_a, mask, avx_b, avx_c);
    avx_result = _mm512_mask_add_ps(avx_result, mask, avx_result, avx_d);
    
    volatile __m512 avx_sink = avx_result;
    (void)avx_sink;
#endif
    
#ifdef __ARM_NEON
    /* Approach 3: ARM NEON multi-vector operations */
    float32x4_t neon_a = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t neon_b = {2.0f, 3.0f, 4.0f, 5.0f};
    float32x4_t neon_c = {3.0f, 4.0f, 5.0f, 6.0f};
    float32x4_t neon_d = {4.0f, 5.0f, 6.0f, 7.0f};
    
    /* Complex NEON expression */
    float32x4_t neon_result = vmlaq_f32(vmlaq_f32(neon_a, neon_b, neon_c), neon_d, neon_a);
    
    volatile float32x4_t neon_sink = neon_result;
    (void)neon_sink;
#endif
}

/* Approach 4: Complex built-in math operations */
__attribute__((hot, noinline))
static double test_builtin_math(void) {
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
    
    /* Nested FMA operations - each FMA has 3 operands, nesting creates complex patterns */
    double result = __builtin_fma(a, b, 
                     __builtin_fma(c, d,
                       __builtin_fma(e, a,
                         __builtin_fma(b, c, d))));
    
    return result;
}

/* Approach 5: Inline assembly with many operands */
__attribute__((hot, noinline))
static void test_multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    uint64_t out1, out2, out3;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7;
    
    /* Extended asm with 10 operands (3 outputs, 7 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %8\n\t"
        "imul %0, %9\n\t"
        "imul %1, %10"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4), 
          "r" (in5), "r" (in6), "r" (in7), "r" (in1)
        : "cc"
    );
    
    volatile uint64_t sink = out1 + out2 + out3;
    (void)sink;
}

/* Approach 6: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((hot, noinline))
static float test_omp_reduction(void) {
    #define N 1024
    float array[N];
    float result = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (float)i;
    }
    
    /* Complex reduction that might use multi-operand instructions */
    #pragma omp simd reduction(+:result)
    for (int i = 0; i < N; i++) {
        result += array[i] * 2.0f + array[i] * array[i] / 3.0f;
    }
    
    return result;
}
#endif

/* Main function that exercises all approaches */
int main(void) {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Test various approaches */
    test_vector_operations();
    
    double math_result = test_builtin_math();
    printf("Built-in math result: %f\n", math_result);
    
    test_multi_operand_asm();
    
#ifdef _OPENMP
    float omp_result = test_omp_reduction();
    printf("OpenMP reduction result: %f\n", omp_result);
#endif
    
    /* Additional complex expression to encourage 11-operand patterns */
    {
        v8sf v1 = {1.0f}, v2 = {2.0f}, v3 = {3.0f}, v4 = {4.0f};
        v8sf v5 = {5.0f}, v6 = {6.0f}, v7 = {7.0f}, v8 = {8.0f};
        v8sf v9 = {9.0f}, v10 = {10.0f};
        
        /* Very complex expression that might require many temporaries */
        v8sf complex_result = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
        complex_result = complex_result * v1 + complex_result * v2;
        
        volatile v8sf sink = complex_result;
        (void)sink;
    }
    
    printf("Test completed.\n");
    return 0;
}
