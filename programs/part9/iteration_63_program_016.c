#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa)) 
void use_result(void* ptr) {
    /* Use inline assembly as a compiler barrier */
    asm volatile("" : : "r"(ptr) : "memory");
}

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random values */
static void init_array(void* arr, size_t size) {
    uint32_t* ptr = (uint32_t*)arr;
    size_t words = (size + 3) / 4;
    for (size_t i = 0; i < words; i++) {
        ptr[i] = lcg_rand();
    }
}

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI: 64-byte integer vectors */
void test_v64qi_blend(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 64) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create data-dependent mask: compare elements for inequality */
        __mmask64 mask = _mm512_cmpneq_epi8_mask(v1, v2);
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HI: 32-word integer vectors */
void test_v32hi_blend(uint16_t* dst, const uint16_t* src1, const uint16_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create data-dependent mask: compare for less-than */
        __mmask32 mask = _mm512_cmplt_epi16_mask(v1, v2);
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HF: 32-half-float vectors */
void test_v32hf_blend(_Float16* dst, const _Float16* src1, const _Float16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Create data-dependent mask: compare for equality */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_EQ_OQ);
        
        /* Blend based on the computed mask */
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        
        _mm512_storeu_ph(dst + i, result);
    }
}

/* V32BF: 32-bfloat16 vectors */
void test_v32bf_blend(__bf16* dst, const __bf16* src1, const __bf16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        /* Load as integers and cast to bfloat16 vectors */
        __m512bh v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512bh v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create data-dependent mask: compare for not-equal */
        __mmask32 mask = _mm512_cmpneq_ph_mask((__m512h)v1, (__m512h)v2);
        
        /* Blend based on the computed mask */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), (__m512i)result);
    }
}

/* V16SI: 16-dword integer vectors */
void test_v16si_blend(int32_t* dst, const int32_t* src1, const int32_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create data-dependent mask: compare for greater-than */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DI: 8-qword integer vectors */
void test_v8di_blend(int64_t* dst, const int64_t* src1, const int64_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create data-dependent mask: compare for equality */
        __mmask8 mask = _mm512_cmpeq_epi64_mask(v1, v2);
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DF: 8-double vectors */
void test_v8df_blend(double* dst, const double* src1, const double* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Create data-dependent mask: compare for less-than */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        
        /* Blend based on the computed mask */
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SF: 16-float vectors */
void test_v16sf_blend(float* dst, const float* src1, const float* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Create data-dependent mask: compare for greater-than-or-equal */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GE_OQ);
        
        /* Blend based on the computed mask */
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        
        _mm512_storeu_ps(dst + i, result);
    }
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main() {
    /* Runtime CPU feature detection */
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512F and AVX-512BW not supported on this CPU\n");
        return 0;
    }
    
    const size_t ARRAY_SIZE = 1024;
    const size_t ITERATIONS = 100;
    
    /* Allocate and initialize arrays for all data types */
    uint8_t* src1_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t* src2_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t* dst_v64qi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    
    uint16_t* src1_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t* src2_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t* dst_v32hi = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    
    _Float16* src1_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16* src2_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16* dst_v32hf = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    
    __bf16* src1_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16* src2_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16* dst_v32bf = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    
    int32_t* src1_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t* src2_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t* dst_v16si = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    int64_t* src1_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t* src2_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t* dst_v8di = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    
    double* src1_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* src2_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* dst_v8df = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    float* src1_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* src2_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* dst_v16sf = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
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
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    
    /* Execute blend operations multiple times to encourage optimization */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test all vector modes */
        test_v64qi_blend(dst_v64qi, src1_v64qi, src2_v64qi, ARRAY_SIZE);
        test_v32hi_blend(dst_v32hi, src1_v32hi, src2_v32hi, ARRAY_SIZE);
        test_v32hf_blend(dst_v32hf, src1_v32hf, src2_v32hf, ARRAY_SIZE);
        test_v32bf_blend(dst_v32bf, src1_v32bf, src2_v32bf, ARRAY_SIZE);
        test_v16si_blend(dst_v16si, src1_v16si, src2_v16si, ARRAY_SIZE);
        test_v8di_blend(dst_v8di, src1_v8di, src2_v8di, ARRAY_SIZE);
        test_v8df_blend(dst_v8df, src1_v8df, src2_v8df, ARRAY_SIZE);
        test_v16sf_blend(dst_v16sf, src1_v16sf, src2_v16sf, ARRAY_SIZE);
    }
    
#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */
    
    /* Prevent dead code elimination */
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
