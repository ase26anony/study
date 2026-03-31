/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use target-specific vector intrinsics with many operands */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
static float complex_vector_operations(void) {
    float result = 0.0f;
    
    /* Strategy 1a: AVX-512 masked operations with many operands */
#ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __mmask16 mask = 0xAAAA;  /* Alternating pattern */
        
        /* _mm512_mask_fmadd_ps has 4 operands but expands to more during RTL */
        __m512 d = _mm512_mask_fmadd_ps(a, mask, b, c);
        
        /* Complex expression that may require many temporary operands */
        __m512 e = _mm512_mask_add_ps(
            _mm512_mask_mul_ps(a, mask, b, c),
            mask,
            _mm512_mask_sub_ps(b, mask, c, a),
            _mm512_set1_ps(4.0f)
        );
        
        /* Extract result */
        result += _mm512_reduce_add_ps(_mm512_add_ps(d, e));
    }
#endif
    
    /* Strategy 1b: ARM NEON multi-vector operations */
#ifdef __ARM_NEON
    {
        float32x4_t v1 = {1.0f, 2.0f, 3.0f, 4.0f};
        float32x4_t v2 = {5.0f, 6.0f, 7.0f, 8.0f};
        float32x4_t v3 = {9.0f, 10.0f, 11.0f, 12.0f};
        float32x4_t v4 = {13.0f, 14.0f, 15.0f, 16.0f};
        
        /* Complex FMA-like operations */
        float32x4_t r1 = vfmaq_f32(v1, v2, v3);
        float32x4_t r2 = vfmaq_laneq_f32(v4, v1, v2, 1);
        float32x4_t r3 = vmlaq_laneq_f32(r1, r2, v3, 2);
        
        result += vaddvq_f32(r3);
    }
#endif
    
    /* Strategy 2: GCC vector extensions with complex expressions */
    {
        v8sf va = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf vb = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
        v8sf vc = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf vd = {9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f};
        
        /* Complex expression that may expand to many operands */
        v8sf vr = va * vb + vc * vd - (va + vb) * (vc - vd);
        
        /* Reduction */
        for (int i = 0; i < 8; i++) {
            result += vr[i];
        }
    }
    
    return result;
}

/* Strategy 3: Inline assembly with many operands */
__attribute__((noinline))
static uint64_t multi_operand_asm(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    uint64_t k = 11, result = 0;
    
    /* 11-operand inline assembly (10 inputs + 1 output) */
    asm volatile (
        "add %[a], %[b], %[c]\n\t"
        "add %[d], %[e], %[f]\n\t"
        "mul %[g], %[h], %[i]\n\t"
        "add %[result], %[j], %[k]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Strategy 4: Built-in functions for complex math */
__attribute__((noinline))
static double complex_builtins(void) {
    double a = 1.5, b = 2.5, c = 3.5, d = 4.5;
    double e = 5.5, f = 6.5, g = 7.5, h = 8.5;
    
    /* Nested FMA operations - each __builtin_fma has 3 operands */
    double r1 = __builtin_fma(a, b, c);
    double r2 = __builtin_fma(d, e, f);
    double r3 = __builtin_fma(g, h, r1);
    double r4 = __builtin_fma(r2, r3, __builtin_fma(a, c, e));
    
    /* Complex expression preventing early folding */
    return r1 * r2 + r3 / r4 - __builtin_fma(r1, r2, r3);
}

/* Strategy 5: OpenMP SIMD reduction with vector types */
__attribute__((noinline))
static float omp_simd_reduction(void) {
    #define N 1024
    float array[N];
    float result = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (i % 10) * 0.1f;
    }
    
    /* Complex reduction that may generate multi-operand instructions */
    #pragma omp simd reduction(+:result)
    for (int i = 0; i < N; i++) {
        /* Complex expression to encourage multi-operand expansion */
        result += array[i] * 2.0f - array[(i + 1) % N] * 0.5f 
                + array[(i + 2) % N] * 1.5f - array[(i + 3) % N] * 0.75f;
    }
    
    return result;
}

int main(void) {
    float r1 = complex_vector_operations();
    uint64_t r2 = multi_operand_asm();
    double r3 = complex_builtins();
    float r4 = omp_simd_reduction();
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %lu, %f, %f\n", 
           r1, (unsigned long)r2, r3, r4);
    
    /* Return non-zero if any result is suspiciously zero */
    return (r1 == 0.0f && r2 == 0 && r3 == 0.0 && r4 == 0.0f) ? 1 : 0;
}
