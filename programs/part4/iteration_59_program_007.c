/* Test for covering 10/11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Strategy 1: Use AVX-512 intrinsics with many operands (x86 specific) */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
__attribute__((always_inline, target("avx512f,fma")))
static inline __m512 test_avx512_10_operands(__m512 a, __m512 b, __m512 c, 
                                            __m512 d, __mmask16 k, float imm) {
    /* Complex expression that might expand to multi-operand pattern */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);  /* 5 operands: a, k, b, c, 0 */
    __m512 t2 = _mm512_mask_sub_ps(d, k, t1, _mm512_set1_ps(imm)); /* 5 operands */
    /* Combined could reach 10+ operands during expansion */
    return _mm512_mask_mul_ps(t2, k, t1, _mm512_set1_ps(2.0f));
}

__attribute__((always_inline, target("avx512f")))
static inline __m512i test_avx512_11_operands(__m512i a, __m512i b, __m512i c,
                                             __m512i d, __mmask64 k, int imm) {
    /* Multiple operations that might combine */
    __m512i t1 = _mm512_mask_slli_epi32(a, k, b, 3);  /* 4 operands */
    __m512i t2 = _mm512_mask_add_epi32(c, k, d, t1);  /* 4 operands */
    /* Additional operation to push operand count */
    return _mm512_mask_blend_epi32(k, t2, _mm512_set1_epi32(imm)); /* 3 operands */
}
#endif

/* Strategy 2: Use GCC vector extensions for complex expressions */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

__attribute__((always_inline))
static inline v16sf test_vector_extensions(v16sf a, v16sf b, v16sf c, 
                                          v16sf d, v16sf e, v16sf f) {
    /* Complex expression with many operands */
    v16sf t1 = a * b + c;
    v16sf t2 = d - e * f;
    v16sf t3 = t1 / (t2 + 1.0f);
    v16sf t4 = __builtin_fmaf(t1, t2, t3);  /* Builtin FMA */
    v16sf t5 = __builtin_fmaf(t3, t4, t1);
    
    /* Nested expressions that might require many temporaries */
    return (t1 * t2) + (t3 * t4) + (t5 * a) + (b * c) + (d * e) + f;
}

/* Strategy 3: Use inline assembly with many operands */
static inline uint64_t test_asm_10_operands(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2, result3, result4;
    
    /* Inline asm with 10 operands (6 inputs, 4 outputs) */
    asm volatile (
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[b]\n\t"
        "mov %[r2], %[c]\n\t"
        "sub %[r2], %[d]\n\t"
        "mov %[r3], %[e]\n\t"
        "and %[r3], %[f]\n\t"
        "mov %[r4], %[g]\n\t"
        "or %[r4], %[h]\n\t"
        "xor %[r1], %[i]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2),
          [r3] "=&r" (result3), [r4] "=&r" (result4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Strategy 4: Complex reduction with OpenMP SIMD */
#ifdef _OPENMP
__attribute__((noinline, hot))
static float test_omp_reduction(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(16)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might expand to multi-operand pattern */
        float t = arr[i];
        sum += t * t + __builtin_fmaf(t, 2.0f, 1.0f) / (t + 3.0f);
    }
    
    return sum;
}
#endif

/* Strategy 5: ARM NEON/SVE intrinsics (aarch64 specific) */
#ifdef __ARM_NEON
#include <arm_neon.h>

__attribute__((always_inline))
static inline float32x4_t test_neon_complex(float32x4_t a, float32x4_t b,
                                           float32x4_t c, float32x4_t d,
                                           float32x4_t e, float32x4_t f) {
    /* Complex sequence of operations */
    float32x4_t t1 = vfmaq_f32(a, b, c);      /* FMA: a + b * c */
    float32x4_t t2 = vfmsq_f32(d, e, f);      /* FMS: d - e * f */
    float32x4_t t3 = vaddq_f32(t1, t2);
    float32x4_t t4 = vmulq_f32(t3, vdupq_n_f32(2.0f));
    
    /* Lane operations that might require many operands */
    float32x4_t t5 = vmlaq_laneq_f32(t4, a, b, 1);  /* t4 + a * b[1] */
    return vfmaq_laneq_f32(t5, c, d, 2);            /* t5 + c * d[2] */
}
#endif

/* Main test function */
int main() {
    volatile int result = 0;
    
    /* Test 1: Vector extensions */
    {
        v16sf v1 = {0}, v2 = {1}, v3 = {2}, v4 = {3}, v5 = {4}, v6 = {5};
        v16sf r = test_vector_extensions(v1, v2, v3, v4, v5, v6);
        /* Use result to prevent optimization */
        result += (int)r[0];
    }
    
    /* Test 2: Inline assembly with many operands */
    {
        uint64_t r = test_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9);
        result += (int)r;
    }
    
    /* Test 3: OpenMP reduction if available */
    #ifdef _OPENMP
    {
        float arr[100];
        for (int i = 0; i < 100; i++) arr[i] = i * 0.1f;
        float sum = test_omp_reduction(arr, 100);
        result += (int)sum;
    }
    #endif
    
    /* Test 4: AVX-512 if available */
    #ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 d = _mm512_set1_ps(4.0f);
        __mmask16 k = 0xAAAA;
        
        __m512 r1 = test_avx512_10_operands(a, b, c, d, k, 5.0f);
        float f = _mm512_cvtss_f32(r1);
        result += (int)f;
        
        __m512i ai = _mm512_set1_epi32(1);
        __m512i bi = _mm512_set1_epi32(2);
        __m512i ci = _mm512_set1_epi32(3);
        __m512i di = _mm512_set1_epi32(4);
        __mmask64 ki = 0xAAAAAAAAAAAAAAAA;
        
        __m512i r2 = test_avx512_11_operands(ai, bi, ci, di, ki, 5);
        int i = _mm512_cvtsi512_si32(r2);
        result += i;
    }
    #endif
    
    /* Test 5: ARM NEON if available */
    #ifdef __ARM_NEON
    {
        float32x4_t a = vdupq_n_f32(1.0f);
        float32x4_t b = vdupq_n_f32(2.0f);
        float32x4_t c = vdupq_n_f32(3.0f);
        float32x4_t d = vdupq_n_f32(4.0f);
        float32x4_t e = vdupq_n_f32(5.0f);
        float32x4_t f = vdupq_n_f32(6.0f);
        
        float32x4_t r = test_neon_complex(a, b, c, d, e, f);
        result += (int)vgetq_lane_f32(r, 0);
    }
    #endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
