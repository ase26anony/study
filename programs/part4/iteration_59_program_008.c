/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Strategy 1: Use AVX-512 intrinsics for x86 targets */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
__attribute__((always_inline, target("avx512f,fma")))
static inline __m512 test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, 
                                               __m512 d, __m512 e, __m512 f,
                                               __mmask16 k, float constant) {
    /* Complex sequence that might expand to multi-operand instructions */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);  /* 5 operands: a, k, b, c, t1 */
    __m512 t2 = _mm512_mask_fmadd_ps(d, k, e, f);  /* Another 5 operands */
    
    /* Additional operations to increase operand count in expansion */
    __m512 broadcast = _mm512_set1_ps(constant);
    __m512 result = _mm512_mask_add_ps(t1, k, t2, broadcast); /* 5 operands */
    
    /* More complex operation with immediate */
    result = _mm512_mask_mul_ps(result, k, result, 
                               _mm512_maskz_mov_ps(k, _mm512_set1_ps(2.0f)));
    
    return result;
}
#endif

/* Strategy 2: GCC vector extensions for generic testing */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Complex expression with many operands */
__attribute__((always_inline))
static inline v16sf test_vector_extension(v16sf a, v16sf b, v16sf c, 
                                         v16sf d, v16sf e, v16sf f,
                                         v16sf g, v16sf h, float scalar) {
    /* This complex expression may require many temporaries during expansion */
    v16sf t1 = a + b * c;
    v16sf t2 = d - e / f;
    v16sf t3 = g * h + a;
    v16sf t4 = t1 * t2 - t3;
    
    /* Broadcast scalar to vector */
    v16sf broadcast = (v16sf){scalar, scalar, scalar, scalar, 
                             scalar, scalar, scalar, scalar,
                             scalar, scalar, scalar, scalar,
                             scalar, scalar, scalar, scalar};
    
    /* Complex reduction-like operation */
    v16sf result = t4 * broadcast + (a + b) * (c - d);
    
    /* FMA-like pattern */
    result = result * t1 + t2;
    result = result * t3 + t4;
    
    return result;
}

/* Strategy 3: Inline assembly with many operands */
static void test_multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long op11 = 11, result = 0;
    
    /* Extended asm with 11 operands (including clobbers) */
    __asm__ volatile (
        "mov %[r], %[a]\n\t"
        "add %[r], %[b]\n\t"
        "add %[r], %[c]\n\t"
        "add %[r], %[d]\n\t"
        "add %[r], %[e]\n\t"
        "add %[r], %[f]\n\t"
        "add %[r], %[g]\n\t"
        "add %[r], %[h]\n\t"
        "add %[r], %[i]\n\t"
        "add %[r], %[j]\n\t"
        "mov %[k], %[r]"
        : [r] "=&r" (result), [k] "=r" (op11)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10)
        : "cc"
    );
    
    printf("Assembly result: %ld\n", result);
}

/* Strategy 4: Built-in functions and complex math */
__attribute__((optimize("O3")))
static float test_builtin_complex(float a, float b, float c, 
                                  float d, float e, float f,
                                  float g, float h, float i, float j) {
    /* Chain of FMA operations - each expands with multiple operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    
    /* Complex expression that might generate multi-operand pattern */
    float result = __builtin_fmaf(t1, t2, t3);
    result = __builtin_fmaf(result, a, b);
    result = __builtin_fmaf(result, c, d);
    result = __builtin_fmaf(result, e, f);
    result = __builtin_fmaf(result, g, h);
    
    /* Final operation with immediate constant */
    result = __builtin_fmaf(result, 2.0f, j);
    
    return result;
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static v16sf test_omp_reduction(v16sf *array, int size) {
    v16sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < size; i++) {
        /* Complex operation that might expand to multi-operand instruction */
        sum = sum + array[i] * array[i] - array[i] / 2.0f;
    }
    
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    printf("Testing multi-operand instruction patterns\n");
    
    /* Test inline assembly with many operands */
    test_multi_operand_asm();
    
    /* Test built-in complex math */
    float builtin_result = test_builtin_complex(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                                6.0f, 7.0f, 8.0f, 9.0f, 10.0f);
    printf("Built-in result: %f\n", builtin_result);
    
    /* Initialize vector data */
    v16sf vec_a = {1.0f}, vec_b = {2.0f}, vec_c = {3.0f};
    v16sf vec_d = {4.0f}, vec_e = {5.0f}, vec_f = {6.0f};
    v16sf vec_g = {7.0f}, vec_h = {8.0f};
    
    /* Test vector extensions */
    v16sf vec_result = test_vector_extension(vec_a, vec_b, vec_c, vec_d,
                                            vec_e, vec_f, vec_g, vec_h, 2.5f);
    
    /* Use result to prevent optimization */
    float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += vec_result[i];
    }
    printf("Vector result sum: %f\n", sum);
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __m512 avx_e = _mm512_set1_ps(5.0f);
    __m512 avx_f = _mm512_set1_ps(6.0f);
    __mmask16 mask = 0xFFFF;
    
    __m512 avx_result = test_avx512_multi_operand(avx_a, avx_b, avx_c, avx_d,
                                                  avx_e, avx_f, mask, 1.5f);
    
    /* Extract and use result */
    float avx_store[16];
    _mm512_storeu_ps(avx_store, avx_result);
    printf("AVX-512 test completed\n");
#endif
    
#ifdef _OPENMP
    /* Test OpenMP reduction */
    v16sf omp_array[4];
    for (int i = 0; i < 4; i++) {
        omp_array[i] = (v16sf){i * 1.0f};
    }
    
    v16sf omp_result = test_omp_reduction(omp_array, 4);
    printf("OpenMP test completed\n");
#endif
    
    return 0;
}
