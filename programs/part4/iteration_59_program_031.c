/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                       __mmask16 k1, __mmask16 k2, float imm1, float imm2) {
    /* Complex sequence that may expand to multi-operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);  /* 5 operands */
    __m512 t2 = _mm512_mask_fmadd_ps(t1, k2, d, a, _mm512_set1_ps(imm1)); /* 6+ operands */
    __m512 t3 = _mm512_mask_sub_ps(t2, k1, t2, _mm512_set1_ps(imm2));
    
    /* Combined operation that might require many operands during expansion */
    return _mm512_mask3_fmadd_ps(t1, t2, t3, k1);  /* 5 operands + mask */
}

static __m512i __attribute__((always_inline))
test_avx512_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                       __mmask16 k1, __mmask16 k2, __mmask16 k3,
                       int imm1, int imm2, int imm3) {
    /* Even more complex pattern aiming for 11 operands */
    __m512i t1 = _mm512_mask_slli_epi32(a, k1, b, imm1);  /* 5 operands */
    __m512i t2 = _mm512_mask_add_epi32(t1, k2, c, d);
    __m512i t3 = _mm512_mask_srai_epi32(t2, k3, t2, imm2);
    
    /* Permutation with many operands */
    return _mm512_mask_permutexvar_epi32(t3, k1, _mm512_set_epi32(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), t2);
}
#endif

/* For AArch64 Advanced SIMD/NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>

/* GCC vector extensions for complex operations */
typedef float32x4_t v4sf __attribute__((vector_size(16)));
typedef float32x4_t v8sf __attribute__((vector_size(32)));

static v4sf __attribute__((always_inline))
test_neon_multi_operand(v4sf a, v4sf b, v4sf c, v4sf d,
                       v4sf e, v4sf f, int lane1, int lane2) {
    /* Complex lane operations and permutations */
    v4sf t1 = vfmaq_laneq_f32(a, b, c, lane1);  /* 4 operands + lane */
    v4sf t2 = vfmaq_laneq_f32(t1, d, e, lane2);
    
    /* Table lookup with multiple vectors (vqtbl4q has 4 source vectors + index) */
    uint8x16x4_t tbl = {a, b, c, d};
    uint8x16_t indices = vreinterpretq_u8_f32(f);
    
    /* This may expand to many operands */
    return vreinterpretq_f32_u8(vqtbl4q_u8(tbl, indices));
}
#endif

/* Generic approach using inline assembly with many operands */
static void __attribute__((always_inline))
test_inline_asm_11_operands(int *out1, int *out2, int *out3,
                           int in1, int in2, int in3, int in4, int in5) {
    /* Inline assembly with 11 total operands (3 outputs, 5 inputs, 3 clobbers) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "mov %1, %5\n\t"
        "sub %1, %1, %6\n\t"
        "mov %2, %7\n\t"
        "imul %2, %2, %8"
        : "=r"(*out1), "=r"(*out2), "=r"(*out3)  /* 3 outputs */
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),  /* 5 inputs */
          "m"(*out1), "m"(*out2)  /* 2 memory inputs - total 10 operands */
        : "rax", "rbx", "rcx"  /* 3 clobbers - compiler sees 13 total operands */
    );
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
static float __attribute__((hot))
test_omp_reduction(float *data, int n) {
    typedef float v8sf __attribute__((vector_size(32)));
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum) aligned(data:32)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk;
        memcpy(&chunk, &data[i], sizeof(v8sf));
        
        /* Complex expression that might expand to multi-operand pattern */
        sum = sum + chunk * (v8sf){1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f}
                  - (v8sf){0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    }
    
    /* Horizontal reduction */
    float result = 0;
    for (int i = 0; i < 8; i++) {
        result += sum[i];
    }
    return result;
}
#endif

/* Built-in functions for complex math */
static double __attribute__((always_inline))
test_builtin_fma_chain(double a, double b, double c, double d, double e) {
    /* Chain of FMA operations - each expands to 4 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(t1, d, e);
    double t3 = __builtin_fma(a, t2, __builtin_fma(b, c, d));
    
    /* Complex expression preventing early folding */
    return __builtin_fma(t1, t2, __builtin_fma(t3, a, __builtin_fma(b, c, d)));
}

/* Main test function */
int main() {
    float result = 0.0f;
    
    /* Test inline assembly path */
    int out1 = 0, out2 = 0, out3 = 0;
    test_inline_asm_11_operands(&out1, &out2, &out3, 1, 2, 3, 4, 5);
    result += out1 + out2 + out3;
    
    /* Test built-in FMA chain */
    result += test_builtin_fma_chain(1.1, 2.2, 3.3, 4.4, 5.5);
    
#ifdef __AVX512F__
    /* Test AVX-512 paths */
    __m512 vec_a = _mm512_set1_ps(1.0f);
    __m512 vec_b = _mm512_set1_ps(2.0f);
    __m512 vec_c = _mm512_set1_ps(3.0f);
    __m512 vec_d = _mm512_set1_ps(4.0f);
    
    __m512 res1 = test_avx512_10_operands(vec_a, vec_b, vec_c, vec_d,
                                         0xFFFF, 0x00FF, 5.0f, 6.0f);
    
    __m512i ivec_a = _mm512_set1_epi32(1);
    __m512i ivec_b = _mm512_set1_epi32(2);
    __m512i ivec_c = _mm512_set1_epi32(3);
    __m512i ivec_d = _mm512_set1_epi32(4);
    
    __m512i res2 = test_avx512_11_operands(ivec_a, ivec_b, ivec_c, ivec_d,
                                          0xFFFF, 0x00FF, 0x0F0F, 1, 2, 3);
    
    /* Extract some result to prevent elimination */
    float temp[16];
    _mm512_storeu_ps(temp, res1);
    result += temp[0];
#endif

#ifdef __ARM_NEON
    /* Test NEON path */
    float32x4_t neon_a = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t neon_b = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t neon_c = {9.0f, 10.0f, 11.0f, 12.0f};
    float32x4_t neon_d = {13.0f, 14.0f, 15.0f, 16.0f};
    float32x4_t neon_e = {17.0f, 18.0f, 19.0f, 20.0f};
    float32x4_t neon_f = {21.0f, 22.0f, 23.0f, 24.0f};
    
    float32x4_t neon_res = test_neon_multi_operand(neon_a, neon_b, neon_c,
                                                  neon_d, neon_e, neon_f, 1, 2);
    result += vgetq_lane_f32(neon_res, 0);
#endif

#ifdef _OPENMP
    /* Test OpenMP reduction */
    float data[1024];
    for (int i = 0; i < 1024; i++) {
        data[i] = i * 0.1f;
    }
    result += test_omp_reduction(data, 1024);
#endif

    printf("Result: %f\n", result);
    return (result > 0) ? 0 : 1;
}
