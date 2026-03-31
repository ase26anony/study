/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                       __mmask16 k1, __mmask16 k2, float imm1, float imm2) {
    /* Complex sequence that may expand to multi-operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);  /* 4 operands + mask */
    __m512 t2 = _mm512_mask_mul_ps(t1, k2, d, _mm512_set1_ps(imm1));
    
    /* FMA with mask - can generate many operands during expansion */
    __m512 t3 = _mm512_mask_fmadd_ps(t2, k1, 
                                    _mm512_set1_ps(imm2),
                                    _mm512_set1_ps(2.0f));
    
    /* Another masked operation */
    __m512 result = _mm512_mask_sub_ps(t3, k2,
                                      _mm512_set1_ps(1.0f),
                                      _mm512_set1_ps(0.5f));
    
    return result;
}

/* Test with 11 potential operands through nested operations */
static __m512i __attribute__((always_inline))
test_avx512_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                       __mmask64 k, int imm1, int imm2, int imm3) {
    /* Complex permute/shuffle with immediate constants */
    __m512i t1 = _mm512_mask_add_epi32(a, k, b, c);
    __m512i t2 = _mm512_mask_slli_epi32(t1, k, d, imm1);
    __m512i t3 = _mm512_mask_alignr_epi32(t2, k, b, c, imm2);
    
    /* Blend with multiple sources */
    __m512i result = _mm512_mask_blend_epi32(k, t3, _mm512_set1_epi32(imm3));
    
    return result;
}
#endif

/* For AArch64/ARM NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>

/* GCC vector extensions for complex operations */
typedef float32x4_t v4sf __attribute__((vector_size(16)));
typedef float32x4_t v8sf __attribute__((vector_size(32)));

static v4sf __attribute__((always_inline))
test_neon_multi_operand(v4sf a, v4sf b, v4sf c, v4sf d,
                       v4sf e, v4sf f, v4sf g, float h, float i, float j) {
    /* Complex expression that may require many operands */
    v4sf t1 = a + b * c;
    v4sf t2 = d - e / f;
    v4sf t3 = g * (v4sf){h, i, j, h};
    v4sf t4 = vfmaq_f32(t1, t2, t3);
    v4sf t5 = vmlaq_f32(t4, a, (v4sf){i, j, h, i});
    
    return vaddq_f32(t5, vmulq_f32(b, (v4sf){j, h, i, j}));
}
#endif

/* Generic approach using inline assembly with many operands */
static void __attribute__((always_inline))
test_inline_asm_11_operands(uint64_t *out1, uint64_t *out2, uint64_t *out3,
                           uint64_t in1, uint64_t in2, uint64_t in3,
                           uint64_t in4, uint64_t in5, uint64_t in6) {
    /* Inline asm with 11 total operands (3 outputs, 6 inputs, 2 clobbers) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "sub %1, %6\n\t"
        "mov %2, %7\n\t"
        "xor %2, %8\n\t"
        : "=r"(*out1), "=r"(*out2), "=r"(*out3)  /* 3 outputs */
        : "r"(in1), "r"(in2), "r"(in3),          /* 6 inputs */
          "r"(in4), "r"(in5), "r"(in6),
          "m"(*(const uint64_t[1]){0})           /* memory input */
        : "cc", "memory"                         /* 2 clobbers */
    );
}

/* Use GCC builtins for complex math */
static double __attribute__((always_inline))
test_builtin_multi_operand(double a, double b, double c, double d,
                          double e, double f, double g, double h) {
    /* Nested FMA calls - each expands with multiple operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, t1);
    double result = __builtin_fma(t2, t3, __builtin_fma(a, c, d));
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static float omp_vector_reduction(float *arr, int n) {
    typedef float v8sf __attribute__((vector_size(32)));
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk = *(v8sf *)&arr[i];
        sum = sum + chunk * (v8sf){1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f};
    }
    
    /* Horizontal sum */
    float result = 0;
    for (int i = 0; i < 8; i++) {
        result += sum[i];
    }
    return result;
}
#endif

/* Main test driver */
int main() {
    volatile int result = 0;
    
    /* Test 1: Inline assembly with many operands */
    {
        uint64_t out1, out2, out3;
        test_inline_asm_11_operands(&out1, &out2, &out3,
                                   1, 2, 3, 4, 5, 6);
        result ^= (int)(out1 + out2 + out3);
    }
    
    /* Test 2: Builtin complex math */
    {
        double r = test_builtin_multi_operand(1.1, 2.2, 3.3, 4.4,
                                             5.5, 6.6, 7.7, 8.8);
        result ^= (int)r;
    }
    
#ifdef __AVX512F__
    /* Test 3: AVX-512 masked operations */
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __m512 d = _mm512_set1_ps(4.0f);
        __mmask16 k1 = 0xAAAA;
        __mmask16 k2 = 0x5555;
        
        __m512 r1 = test_avx512_10_operands(a, b, c, d, k1, k2, 5.0f, 6.0f);
        float f = _mm512_cvtss_f32(r1);
        result ^= (int)f;
        
        __m512i ai = _mm512_set1_epi32(1);
        __m512i bi = _mm512_set1_epi32(2);
        __m512i ci = _mm512_set1_epi32(3);
        __m512i di = _mm512_set1_epi32(4);
        __mmask64 ki = 0xF0F0F0F0;
        
        __m512i r2 = test_avx512_11_operands(ai, bi, ci, di, ki, 5, 6, 7);
        int i = _mm512_cvtsi512_si32(r2);
        result ^= i;
    }
#endif

#ifdef __ARM_NEON
    /* Test 4: ARM NEON vector operations */
    {
        float32x4_t va = {1.0f, 2.0f, 3.0f, 4.0f};
        float32x4_t vb = {5.0f, 6.0f, 7.0f, 8.0f};
        float32x4_t vc = {9.0f, 10.0f, 11.0f, 12.0f};
        float32x4_t vd = {13.0f, 14.0f, 15.0f, 16.0f};
        float32x4_t ve = {17.0f, 18.0f, 19.0f, 20.0f};
        float32x4_t vf = {21.0f, 22.0f, 23.0f, 24.0f};
        float32x4_t vg = {25.0f, 26.0f, 27.0f, 28.0f};
        
        float32x4_t vr = test_neon_multi_operand(va, vb, vc, vd, ve, vf, vg,
                                                29.0f, 30.0f, 31.0f);
        result ^= (int)vr[0];
    }
#endif

#ifdef _OPENMP
    /* Test 5: OpenMP SIMD reduction */
    {
        float arr[256];
        for (int i = 0; i < 256; i++) {
            arr[i] = (float)i;
        }
        float r = omp_vector_reduction(arr, 256);
        result ^= (int)r;
    }
#endif
    
    printf("Result: %d\n", result);
    return result != 0;
}
