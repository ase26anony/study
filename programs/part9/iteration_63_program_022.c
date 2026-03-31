/* avx512_blend_coverage.c
 * Tests all AVX-512 blend patterns in i386-expand.cc lines 4303-4326
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512vl -fdump-rtl-expand -fdump-tree-vect -o avx512_blend_test avx512_blend_coverage.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent dead code elimination */
__attribute__((noinline, noipa))
void use_result(void* ptr) {
    /* Use inline assembly as a sink to prevent optimization */
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI: 64-byte integer blend */
void test_v64qi_blend(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 64) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare elements for inequality */
        __mmask64 mask = _mm512_cmpneq_epi8_mask(v1, v2);
        
        /* Blend based on the computed mask */
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HI: 32-word integer blend */
void test_v32hi_blend(uint16_t* dst, const uint16_t* src1, const uint16_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for less-than */
        __mmask32 mask = _mm512_cmplt_epi16_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HF: 32-half-float blend */
void test_v32hf_blend(_Float16* dst, const _Float16* src1, const _Float16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Data-dependent mask: compare for equality */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_EQ_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_ph(dst + i, result);
    }
}

/* V32BF: 32-bfloat16 blend */
void test_v32bf_blend(__bf16* dst, const __bf16* src1, const __bf16* src2, size_t n) {
    for (size_t i = 0; i < n; i += 32) {
        /* Load as integers and cast to bfloat16 vectors */
        __m512bh v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512bh v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Create a data-dependent mask using integer comparison */
        __m512i vi1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i vi2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        __mmask32 mask = _mm512_cmplt_epi16_mask(vi1, vi2);
        
        /* Use the same intrinsic as V32HF but with bfloat16 types */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), (__m512i)result);
    }
}

/* V16SI: 16-dword integer blend */
void test_v16si_blend(int32_t* dst, const int32_t* src1, const int32_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for greater-than */
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DI: 8-qword integer blend */
void test_v8di_blend(int64_t* dst, const int64_t* src1, const int64_t* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512i v1 = _mm512_loadu_si512((const __m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((const __m512i*)(src2 + i));
        
        /* Data-dependent mask: compare for inequality */
        __mmask8 mask = _mm512_cmpneq_epi64_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DF: 8-double blend */
void test_v8df_blend(double* dst, const double* src1, const double* src2, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Data-dependent mask: compare for less-than */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SF: 16-float blend */
void test_v16sf_blend(float* dst, const float* src1, const float* src2, size_t n) {
    for (size_t i = 0; i < n; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Data-dependent mask: compare for greater-than */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_storeu_ps(dst + i, result);
    }
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(void) {
    /* Runtime CPU feature detection */
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512F and AVX-512BW not supported on this CPU\n");
        return 0;
    }
    
    const size_t ARRAY_SIZE = 1024;
    const size_t ITERATIONS = 100;
    
    /* Allocate and initialize arrays for each data type */
    uint8_t* src1_u8 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t* src2_u8 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    uint8_t* dst_u8 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint8_t));
    
    uint16_t* src1_u16 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t* src2_u16 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    uint16_t* dst_u16 = aligned_alloc(64, ARRAY_SIZE * sizeof(uint16_t));
    
    _Float16* src1_f16 = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16* src2_f16 = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    _Float16* dst_f16 = aligned_alloc(64, ARRAY_SIZE * sizeof(_Float16));
    
    __bf16* src1_bf16 = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16* src2_bf16 = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    __bf16* dst_bf16 = aligned_alloc(64, ARRAY_SIZE * sizeof(__bf16));
    
    int32_t* src1_i32 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t* src2_i32 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t* dst_i32 = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    int64_t* src1_i64 = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t* src2_i64 = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    int64_t* dst_i64 = aligned_alloc(64, ARRAY_SIZE * sizeof(int64_t));
    
    double* src1_f64 = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* src2_f64 = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* dst_f64 = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    float* src1_f32 = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* src2_f32 = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* dst_f32 = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pseudo-random values */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        src1_u8[i] = lcg_rand() & 0xFF;
        src2_u8[i] = lcg_rand() & 0xFF;
        
        src1_u16[i] = lcg_rand() & 0xFFFF;
        src2_u16[i] = lcg_rand() & 0xFFFF;
        
        src1_f16[i] = (lcg_rand() & 0xFFFF) / 65535.0f;
        src2_f16[i] = (lcg_rand() & 0xFFFF) / 65535.0f;
        
        src1_bf16[i] = (lcg_rand() & 0xFFFF);
        src2_bf16[i] = (lcg_rand() & 0xFFFF);
        
        src1_i32[i] = (int32_t)lcg_rand();
        src2_i32[i] = (int32_t)lcg_rand();
        
        src1_i64[i] = ((int64_t)lcg_rand() << 32) | lcg_rand();
        src2_i64[i] = ((int64_t)lcg_rand() << 32) | lcg_rand();
        
        src1_f64[i] = (double)lcg_rand() / (double)UINT32_MAX;
        src2_f64[i] = (double)lcg_rand() / (double)UINT32_MAX;
        
        src1_f32[i] = (float)lcg_rand() / (float)UINT32_MAX;
        src2_f32[i] = (float)lcg_rand() / (float)UINT32_MAX;
    }
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    
    /* Execute blend operations multiple times to encourage vectorization */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test all vector modes from the uncovered switch */
        test_v64qi_blend(dst_u8, src1_u8, src2_u8, ARRAY_SIZE);
        test_v32hi_blend(dst_u16, src1_u16, src2_u16, ARRAY_SIZE);
        test_v32hf_blend(dst_f16, src1_f16, src2_f16, ARRAY_SIZE);
        test_v32bf_blend(dst_bf16, src1_bf16, src2_bf16, ARRAY_SIZE);
        test_v16si_blend(dst_i32, src1_i32, src2_i32, ARRAY_SIZE);
        test_v8di_blend(dst_i64, src1_i64, src2_i64, ARRAY_SIZE);
        test_v8df_blend(dst_f64, src1_f64, src2_f64, ARRAY_SIZE);
        test_v16sf_blend(dst_f32, src1_f32, src2_f32, ARRAY_SIZE);
    }
    
#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */
    
    /* Force materialization of all results to prevent dead code elimination */
    use_result(dst_u8);
    use_result(dst_u16);
    use_result(dst_f16);
    use_result(dst_bf16);
    use_result(dst_i32);
    use_result(dst_i64);
    use_result(dst_f64);
    use_result(dst_f32);
    
    /* Cleanup */
    free(src1_u8); free(src2_u8); free(dst_u8);
    free(src1_u16); free(src2_u16); free(dst_u16);
    free(src1_f16); free(src2_f16); free(dst_f16);
    free(src1_bf16); free(src2_bf16); free(dst_bf16);
    free(src1_i32); free(src2_i32); free(dst_i32);
    free(src1_i64); free(src2_i64); free(dst_i64);
    free(src1_f64); free(src2_f64); free(dst_f64);
    free(src1_f32); free(src2_f32); free(dst_f32);
    
    return 0;
}
