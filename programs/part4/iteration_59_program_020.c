/* Test program to cover 10 and 11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force aggressive optimization and inlining */
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define HOT __attribute__((hot))

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex inline assembly with many operands */
static void test_many_operand_asm(void) {
    /* 11-operand inline assembly */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    asm volatile (
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "mov %6, %7\n\t"
        "add %6, %8\n\t"
        "add %6, %9\n\t"
        "add %6, %10"
        : "=r"(result1), "=r"(result2)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "0"(result1), "r"(op6), "r"(op7), "r"(op8), "r"(op9), "r"(op10)
        : "cc"
    );
    
    printf("ASM result: %ld, %ld\n", result1, result2);
}

#ifdef __x86_64__
/* AVX-512 intrinsics with many operands */
ALWAYS_INLINE HOT
__m512 test_avx512_many_operands(__m512 a, __m512 b, __m512 c, 
                                 __m512 d, __m512 e, __mmask16 k) {
    /* Complex sequence that might expand to multi-operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(d, k, e, t1);
    __m512 t3 = _mm512_mask_sub_ps(t1, k, t2, a);
    __m512 t4 = _mm512_mask_fmadd_ps(b, k, c, t3);
    
    /* Nested FMA operations */
    __m512 result = _mm512_mask_fmadd_ps(t4, k,
        _mm512_mask_fnmadd_ps(a, k, b, c),
        _mm512_mask_fmsub_ps(d, k, e, t1));
    
    return result;
}

/* Test AVX-512 gather with many parameters */
ALWAYS_INLINE HOT
__m512i test_avx512_gather(__m512i index, __m512i mask, int scale) {
    int base[1024];
    for (int i = 0; i < 1024; i++) base[i] = i;
    
    /* Gather with multiple parameters */
    return _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), 
                                      (__mmask16)mask,
                                      index,
                                      base,
                                      scale);
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style many-operand operations */
ALWAYS_INLINE HOT
int32x4_t test_neon_many_operands(int32x4_t a, int32x4_t b, int32x4_t c,
                                  int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Complex sequence with multiple operations */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vmlaq_s32(c, d, e);
    int32x4_t t3 = vmlsq_s32(t1, t2, f);
    int32x4_t t4 = vqdmulhq_s32(t3, a);
    
    /* Table lookup can expand to many operands */
    uint8x16_t indices = vreinterpretq_u8_s32(t4);
    uint8x16_t table[4] = {vreinterpretq_u8_s32(a), 
                          vreinterpretq_u8_s32(b),
                          vreinterpretq_u8_s32(c),
                          vreinterpretq_u8_s32(d)};
    
    /* This can generate complex multi-operand patterns */
    uint8x16_t result = vqtbl4q_u8(table, indices);
    return vreinterpretq_s32_u8(result);
}
#endif

/* GCC vector extensions with complex expressions */
ALWAYS_INLINE HOT
v8sf test_gcc_vector_many_ops(v8sf a, v8sf b, v8sf c, v8sf d, 
                              v8sf e, v8sf f, v8sf g) {
    /* Complex expression that might require multi-operand expansion */
    v8sf t1 = a + b * c;
    v8sf t2 = d - e / f;
    v8sf t3 = t1 * t2 + g;
    v8sf t4 = __builtin_fmaf(a, b, c);
    v8sf t5 = __builtin_fmaf(d, e, f);
    
    /* Nested FMA calls */
    v8sf result = __builtin_fmaf(t1, t2, 
                      __builtin_fmaf(t3, t4, 
                          __builtin_fmaf(t5, a, b)));
    
    return result;
}

/* OpenMP SIMD reduction with vector types */
HOT
float test_omp_reduction_many_ops(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Complex reduction expression */
        sum += arr[i] * 2.0f - arr[i]/3.0f + 
               __builtin_fmaf(arr[i], 1.5f, 0.25f);
    }
    
    return sum;
}

/* Main test function */
int main(void) {
    const int N = 1024;
    float* data = (float*)aligned_alloc(32, N * sizeof(float));
    if (!data) return 1;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = (float)i / 10.0f;
    }
    
    /* Test inline assembly with many operands */
    test_many_operand_asm();
    
    /* Test OpenMP reduction */
    float result = test_omp_reduction_many_ops(data, N);
    printf("OpenMP reduction result: %f\n", result);
    
    #ifdef __x86_64__
    /* Test AVX-512 if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512 vec1 = _mm512_set1_ps(1.0f);
        __m512 vec2 = _mm512_set1_ps(2.0f);
        __m512 vec3 = _mm512_set1_ps(3.0f);
        __m512 vec4 = _mm512_set1_ps(4.0f);
        __m512 vec5 = _mm512_set1_ps(5.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 avx_result = test_avx512_many_operands(vec1, vec2, vec3, 
                                                      vec4, vec5, mask);
        float res_arr[16];
        _mm512_storeu_ps(res_arr, avx_result);
        printf("AVX-512 result[0]: %f\n", res_arr[0]);
    }
    #endif
    
    #ifdef __aarch64__
    /* Test NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    
    int32x4_t neon_result = test_neon_many_operands(neon_a, neon_b, neon_c,
                                                    neon_d, neon_e, neon_f);
    int32_t res_arr[4];
    vst1q_s32(res_arr, neon_result);
    printf("NEON result[0]: %d\n", res_arr[0]);
    #endif
    
    /* Test GCC vector extensions */
    v8sf gcc_vec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf gcc_vec2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf gcc_vec3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf gcc_vec4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    v8sf gcc_vec5 = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    v8sf gcc_vec6 = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f};
    v8sf gcc_vec7 = {7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f};
    
    v8sf gcc_result = test_gcc_vector_many_ops(gcc_vec1, gcc_vec2, gcc_vec3,
                                               gcc_vec4, gcc_vec5, gcc_vec6,
                                               gcc_vec7);
    float gcc_res_arr[8];
    memcpy(gcc_res_arr, &gcc_result, sizeof(gcc_result));
    printf("GCC vector result[0]: %f\n", gcc_res_arr[0]);
    
    free(data);
    return 0;
}
