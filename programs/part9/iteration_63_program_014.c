#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of results */
extern void use_result(void*) __attribute__((noinline, noipa));

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_array(void *arr, size_t size) {
    uint32_t *ptr = (uint32_t*)arr;
    size_t words = (size + 3) / 4;
    for (size_t i = 0; i < words; i++) {
        ptr[i] = lcg_rand();
    }
}

/* V64QI: 64-byte integer vectors */
#ifdef __AVX512BW__
static void test_v64qi(uint8_t *src1, uint8_t *src2, uint8_t *dst, size_t n) {
    for (size_t i = 0; i < n; i += 64) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Data-dependent mask: blend where v1[i] < v2[i] */
        __mmask64 mask = _mm512_cmplt_epi8_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}
#endif

/* V32HI: 32-word integer vectors */
#ifdef __AVX512BW__
static void test_v32hi(uint16_t *src1, uint16_t *src2, uint16_t *dst, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Data-dependent mask: blend where v1[i] > v2[i] */
        __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}
#endif

/* V32HF: 32 half-precision float vectors */
#ifdef __AVX512BW__
#ifdef __AVX512FP16__
static void test_v32hf(_Float16 *src1, _Float16 *src2, _Float16 *dst, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Data-dependent mask: blend where v1[i] < v2[i] */
        __mmask32 mask = _mm512_cmplt_ph_mask(v1, v2);
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_ph(dst + i, result);
    }
}
#endif
#endif

/* V32BF: 32 bfloat16 vectors */
#ifdef __AVX512BF16__
static void test_v32bf(__bf16 *src1, __bf16 *src2, __bf16 *dst, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        /* Load as epi16 and convert to bfloat16 */
        __m512bh v1 = _mm512_loadu_bf16((void*)(src1 + i));
        __m512bh v2 = _mm512_loadu_bf16((void*)(src2 + i));
        
        /* Convert to float for comparison */
        __m512 f1 = _mm512_cvtneobf16_ps(v1);
        __m512 f2 = _mm512_cvtneobf16_ps(v2);
        
        /* Data-dependent mask: blend where f1[i] != f2[i] */
        __mmask16 mask = _mm512_cmp_ps_mask(f1, f2, _CMP_NEQ_OQ);
        
        /* Expand mask from 16 to 32 bits for bfloat16 */
        __mmask32 mask32 = _mm512_kunpackd(mask, mask);
        
        __m512bh result = _mm512_mask_blend_ph(mask32, v1, v2);
        _mm512_storeu_bf16((void*)(dst + i), result);
    }
}
#endif

/* V16SI: 16 dword integer vectors */
#ifdef __AVX512F__
static void test_v16si(int32_t *src1, int32_t *src2, int32_t *dst, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Data-dependent mask: blend where v1[i] == v2[i] */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}
#endif

/* V8DI: 8 qword integer vectors */
#ifdef __AVX512F__
static void test_v8di(int64_t *src1, int64_t *src2, int64_t *dst, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Data-dependent mask: blend where v1[i] <= v2[i] */
        __mmask8 mask = _mm512_cmple_epi64_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}
#endif

/* V8DF: 8 double-precision float vectors */
#ifdef __AVX512F__
static void test_v8df(double *src1, double *src2, double *dst, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Data-dependent mask: blend where v1[i] >= v2[i] */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GE_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_storeu_pd(dst + i, result);
    }
}
#endif

/* V16SF: 16 single-precision float vectors */
#ifdef __AVX512F__
static void test_v16sf(float *src1, float *src2, float *dst, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Data-dependent mask: blend where v1[i] < v2[i] */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_storeu_ps(dst + i, result);
    }
}
#endif

int main(void) {
    /* Runtime CPU feature detection */
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported\n");
        return 0;
    }
    
    if (!__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512BW not supported\n");
        return 0;
    }
    
    const size_t ARRAY_SIZE = 1024;
    const size_t ITERATIONS = 100;
    
    /* Allocate and initialize arrays for each data type */
    uint8_t *src1_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t *src2_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t *dst_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    
    uint16_t *src1_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t *src2_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t *dst_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    
    _Float16 *src1_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16 *src2_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16 *dst_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    
    __bf16 *src1_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16 *src2_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16 *dst_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    
    int32_t *src1_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *src2_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *dst_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    int64_t *src1_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t *src2_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t *dst_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    
    double *src1_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *src2_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *dst_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    float *src1_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *src2_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *dst_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    /* Initialize all arrays */
    init_array(src1_v64qi, ARRAY_SIZE * sizeof(uint8_t));
    init_array(src2_v64qi, ARRAY_SIZE * sizeof(uint8_t));
    init_array(src1_v32hi, ARRAY_SIZE * sizeof(uint16_t));
    init_array(src2_v32hi, ARRAY_SIZE * sizeof(uint16_t));
    init_array(src1_v32hf, ARRAY_SIZE * sizeof(_Float16));
    init_array(src2_v32hf, ARRAY_SIZE * sizeof(_Float16));
    init_array(src1_v32bf, ARRAY_SIZE * sizeof(__bf16));
    init_array(src2_v32bf, ARRAY_SIZE * sizeof(__bf16));
    init_array(src1_v16si, ARRAY_SIZE * sizeof(int32_t));
    init_array(src2_v16si, ARRAY_SIZE * sizeof(int32_t));
    init_array(src1_v8di, ARRAY_SIZE * sizeof(int64_t));
    init_array(src2_v8di, ARRAY_SIZE * sizeof(int64_t));
    init_array(src1_v8df, ARRAY_SIZE * sizeof(double));
    init_array(src2_v8df, ARRAY_SIZE * sizeof(double));
    init_array(src1_v16sf, ARRAY_SIZE * sizeof(float));
    init_array(src2_v16sf, ARRAY_SIZE * sizeof(float));
    
    /* Execute blend operations in a loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        #ifdef __AVX512BW__
        test_v64qi(src1_v64qi, src2_v64qi, dst_v64qi, ARRAY_SIZE);
        test_v32hi(src1_v32hi, src2_v32hi, dst_v32hi, ARRAY_SIZE);
        #ifdef __AVX512FP16__
        test_v32hf(src1_v32hf, src2_v32hf, dst_v32hf, ARRAY_SIZE);
        #endif
        #endif
        
        #ifdef __AVX512BF16__
        test_v32bf(src1_v32bf, src2_v32bf, dst_v32bf, ARRAY_SIZE);
        #endif
        
        #ifdef __AVX512F__
        test_v16si(src1_v16si, src2_v16si, dst_v16si, ARRAY_SIZE);
        test_v8di(src1_v8di, src2_v8di, dst_v8di, ARRAY_SIZE);
        test_v8df(src1_v8df, src2_v8df, dst_v8df, ARRAY_SIZE);
        test_v16sf(src1_v16sf, src2_v16sf, dst_v16sf, ARRAY_SIZE);
        #endif
    }
    
    /* Pass all results to dummy function to prevent optimization */
    use_result(dst_v64qi);
    use_result(dst_v32hi);
    use_result(dst_v32hf);
    use_result(dst_v32bf);
    use_result(dst_v16si);
    use_result(dst_v8di);
    use_result(dst_v8df);
    use_result(dst_v16sf);
    
    /* Cleanup */
    free(src1_v64qi); free(src2_v64qi); free(dst_v64qi);
    free(src1_v32hi); free(src2_v32hi); free(dst_v32hi);
    free(src1_v32hf); free(src2_v32hf); free(dst_v32hf);
    free(src1_v32bf); free(src2_v32bf); free(dst_v32bf);
    free(src1_v16si); free(src2_v16si); free(dst_v16si);
    free(src1_v8di); free(src2_v8di); free(dst_v8di);
    free(src1_v8df); free(src2_v8df); free(dst_v8df);
    free(src1_v16sf); free(src2_v16sf); free(dst_v16sf);
    
    return 0;
}
