/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use AVX-512 intrinsics for x86 targets */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operand(__m512 a, __m512 b, __m512 c, __m512 d,
                       __m512 e, __m512 f, __m512 g, __m512 h,
                       __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that might expand to multi-operand instructions */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);
    __m512 t2 = _mm512_mask_mul_ps(d, k2, e, f);
    __m512 t3 = _mm512_fmadd_ps(g, h, t1);
    return _mm512_mask_add_ps(t3, k1, t2, _mm512_set1_ps(1.0f));
}

/* Test with 11 operands using masked FMA */
static __m512d __attribute__((always_inline))
test_avx512_11_operand(__m512d a, __m512d b, __m512d c, __m512d d,
                       __m512d e, __m512d f, __m512d g, __m512d h,
                       __m512d i, __mmask8 k) {
    /* Nested FMA operations that might require many operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmsub_pd(d, e, f);
    __m512d t3 = _mm512_mask_fmadd_pd(g, k, h, i);
    return _mm512_mask_add_pd(t1, k, t2, t3);
}
#endif

/* Strategy 2: GCC vector extensions for generic testing */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Complex expression with many operands */
static v16sf __attribute__((always_inline))
test_vector_extension(v16sf a, v16sf b, v16sf c, v16sf d,
                      v16sf e, v16sf f, v16sf g, v16sf h,
                      v16sf i, v16sf j) {
    /* This complex expression might generate multi-operand patterns */
    return a * b + c * d + e * f + g * h + i * j +
           (a + b) * (c + d) * (e + f) * (g + h) * (i + j);
}

/* Strategy 3: Inline assembly with many operands */
static void __attribute__((always_inline))
test_inline_asm_11_operand(uint64_t *a, uint64_t *b, uint64_t *c,
                           uint64_t *d, uint64_t *e, uint64_t *f,
                           uint64_t *g, uint64_t *h, uint64_t *i,
                           uint64_t *j, uint64_t *k) {
    /* Dummy assembly with 11 operands */
    asm volatile (
        "mov %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "sub %[e], %[f]\n\t"
        "and %[g], %[h]\n\t"
        "or %[i], %[j]\n\t"
        : [a] "=r" (*a), [b] "=r" (*b), [c] "=r" (*c),
          [d] "=r" (*d), [e] "=r" (*e)
        : [f] "r" (*f), [g] "r" (*g), [h] "r" (*h),
          [i] "r" (*i), [j] "r" (*j)
        : "memory"
    );
}

/* Strategy 4: Built-in functions for complex math */
static double __attribute__((always_inline))
test_builtin_fma_chain(double a, double b, double c,
                       double d, double e, double f,
                       double g, double h, double i,
                       double j, double k) {
    /* Chain of FMA operations - each FMA has 3 operands,
       but combined expressions might need many */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, k);
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static v16sf test_omp_reduction(v16sf *data, int n) {
    v16sf sum = {0};
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation that might expand to multi-operand pattern */
        sum = sum + data[i] * data[i] + 
              (data[i] + data[(i+1)%n]) * (data[i] - data[(i+1)%n]);
    }
    return sum;
}
#endif

/* Hot function to encourage complex instruction selection */
__attribute__((hot, noinline))
static void run_complex_operations(void) {
    /* Initialize test data */
    float fa[16] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
                    9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
    float fb[16] = {16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0,
                    8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    
    /* Test vector extensions */
    v16sf va = *(v16sf*)fa;
    v16sf vb = *(v16sf*)fb;
    v16sf vc = test_vector_extension(va, vb, va, vb, va, vb, va, vb, va, vb);
    
    /* Test built-in FMA chain */
    double fma_result = test_builtin_fma_chain(1.1, 2.2, 3.3, 4.4, 5.5,
                                               6.6, 7.7, 8.8, 9.9, 10.1, 11.1);
    
    /* Test inline assembly */
    uint64_t asm_vals[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    test_inline_asm_11_operand(&asm_vals[0], &asm_vals[1], &asm_vals[2],
                               &asm_vals[3], &asm_vals[4], &asm_vals[5],
                               &asm_vals[6], &asm_vals[7], &asm_vals[8],
                               &asm_vals[9], &asm_vals[10]);
    
#ifdef __AVX512F__
    /* Test AVX-512 if available */
    __m512 avx_a = _mm512_loadu_ps(fa);
    __m512 avx_b = _mm512_loadu_ps(fb);
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    __m512 avx_result = test_avx512_10_operand(avx_a, avx_b, avx_a, avx_b,
                                               avx_a, avx_b, avx_a, avx_b,
                                               mask1, mask2);
    
    /* Store to prevent elimination */
    _mm512_storeu_ps(fa, avx_result);
#endif
    
#ifdef _OPENMP
    /* Test OpenMP reduction */
    v16sf omp_result = test_omp_reduction(&va, 16);
    *(v16sf*)fa = omp_result;
#endif
    
    /* Use results to prevent dead code elimination */
    volatile float sink = ((float*)&vc)[0] + (float)fma_result + fa[0] + (float)asm_vals[0];
    (void)sink; /* Suppress unused warning */
}

int main(void) {
    /* Run in a loop to increase chances of optimization */
    for (int i = 0; i < 100; i++) {
        run_complex_operations();
    }
    
    printf("Test completed (compile-time coverage target)\n");
    return 0;
}
