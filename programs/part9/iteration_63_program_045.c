/* avx512_blend_coverage.c - Test program for i386-expand.cc blend patterns */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dummy function to prevent optimization - defined in separate file */
extern void use_result(void*) __attribute__((noinline, noipa));

/* Simple LCG for pseudo-random values */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_array_char(char *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (char)lcg_rand();
}
static void init_array_short(short *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (short)lcg_rand();
}
static void init_array_int(int *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (int)lcg_rand();
}
static void init_array_long(long long *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (long long)lcg_rand();
}
static void init_array_float(float *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (float)lcg_rand() / (float)UINT32_MAX;
}
static void init_array_double(double *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (double)lcg_rand() / (double)UINT32_MAX;
}
#ifdef __AVX512FP16__
static void init_array_half(_Float16 *arr, size_t n) {
    for (size_t i = 0; i < n; i++) arr[i] = (_Float16)(lcg_rand() / (float)UINT32_MAX);
}
#endif
#ifdef __AVX512BF16__
static void init_array_bfloat(__bf16 *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint16_t val = lcg_rand() & 0xFFFF;
        memcpy(&arr[i], &val, sizeof(val));
    }
}
#endif

/* V64QI - 64-byte integer blend */
#ifdef __AVX512BW__
static void test_v64qi_blend(void) {
    const size_t N = 1024;
    char src1[N], src2[N], dst[N];
    
    init_array_char(src1, N);
    init_array_char(src2, N);
    
    for (size_t i = 0; i < N; i += 64) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] < src2[i] */
        __mmask64 mask = _mm512_cmplt_epi8_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V32HI - 32-word integer blend */
#ifdef __AVX512BW__
static void test_v32hi_blend(void) {
    const size_t N = 1024;
    short src1[N], src2[N], dst[N];
    
    init_array_short(src1, N);
    init_array_short(src2, N);
    
    for (size_t i = 0; i < N; i += 32) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] > src2[i] */
        __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V32HF - 32 half-float blend */
#ifdef __AVX512FP16__
static void test_v32hf_blend(void) {
    const size_t N = 1024;
    _Float16 src1[N], src2[N], dst[N];
    
    init_array_half(src1, N);
    init_array_half(src2, N);
    
    for (size_t i = 0; i < N; i += 32) {
        __m512h v1 = _mm512_loadu_ph(&src1[i]);
        __m512h v2 = _mm512_loadu_ph(&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] != src2[i] */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_NEQ_UQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_ph(&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V32BF - 32 bfloat16 blend */
#ifdef __AVX512BF16__
static void test_v32bf_blend(void) {
    const size_t N = 1024;
    __bf16 src1[N], src2[N], dst[N];
    
    init_array_bfloat(src1, N);
    init_array_bfloat(src2, N);
    
    for (size_t i = 0; i < N; i += 32) {
        /* Load as epi16 and convert to bfloat16 */
        __m512i v1_i = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2_i = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        __m512bh v1 = (__m512bh)_mm512_cvtepu16_epi32(_mm512_cvtepi16_epi32(v1_i));
        __m512bh v2 = (__m512bh)_mm512_cvtepu16_epi32(_mm512_cvtepi16_epi32(v2_i));
        
        /* Data-dependent mask using integer comparison on raw data */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(v1_i, v2_i);
        mask = ~mask; /* Blend where not equal */
        
        /* Use _Float16 blend intrinsic with cast */
        __m512h result_h = _mm512_mask_blend_ph(mask, 
            (__m512h)v1, (__m512h)v2);
        
        /* Convert back and store */
        __m512i result_i = (__m512i)result_h;
        _mm512_storeu_si512((__m512i*)&dst[i], result_i);
    }
    
    use_result(dst);
}
#endif

/* V16SI - 16 dword integer blend */
#ifdef __AVX512F__
static void test_v16si_blend(void) {
    const size_t N = 1024;
    int src1[N], src2[N], dst[N];
    
    init_array_int(src1, N);
    init_array_int(src2, N);
    
    for (size_t i = 0; i < N; i += 16) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] <= src2[i] */
        __mmask16 mask = _mm512_cmple_epi32_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V8DI - 8 qword integer blend */
#ifdef __AVX512F__
static void test_v8di_blend(void) {
    const size_t N = 1024;
    long long src1[N], src2[N], dst[N];
    
    init_array_long(src1, N);
    init_array_long(src2, N);
    
    for (size_t i = 0; i < N; i += 8) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)&src1[i]);
        __m512i v2 = _mm512_loadu_si512((__m512i*)&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] >= src2[i] */
        __mmask8 mask = _mm512_cmpge_epi64_mask(v1, v2);
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V8DF - 8 double float blend */
#ifdef __AVX512F__
static void test_v8df_blend(void) {
    const size_t N = 1024;
    double src1[N], src2[N], dst[N];
    
    init_array_double(src1, N);
    init_array_double(src2, N);
    
    for (size_t i = 0; i < N; i += 8) {
        __m512d v1 = _mm512_loadu_pd(&src1[i]);
        __m512d v2 = _mm512_loadu_pd(&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] < src2[i] */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_storeu_pd(&dst[i], result);
    }
    
    use_result(dst);
}
#endif

/* V16SF - 16 single float blend */
#ifdef __AVX512F__
static void test_v16sf_blend(void) {
    const size_t N = 1024;
    float src1[N], src2[N], dst[N];
    
    init_array_float(src1, N);
    init_array_float(src2, N);
    
    for (size_t i = 0; i < N; i += 16) {
        __m512 v1 = _mm512_loadu_ps(&src1[i]);
        __m512 v2 = _mm512_loadu_ps(&src2[i]);
        
        /* Data-dependent mask: blend where src1[i] > src2[i] */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_storeu_ps(&dst[i], result);
    }
    
    use_result(dst);
}
#endif

int main(void) {
    /* Runtime CPU feature detection */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    int has_avx512fp16 = __builtin_cpu_supports("avx512fp16");
    int has_avx512bf16 = __builtin_cpu_supports("avx512bf16");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    printf("Testing AVX-512 blend patterns...\n");
    
#ifdef __AVX512F__
    if (has_avx512f) {
        test_v16si_blend();
        test_v8di_blend();
        test_v8df_blend();
        test_v16sf_blend();
    }
#endif
    
#ifdef __AVX512BW__
    if (has_avx512f && has_avx512bw) {
        test_v64qi_blend();
        test_v32hi_blend();
    }
#endif
    
#ifdef __AVX512FP16__
    if (has_avx512f && has_avx512fp16) {
        test_v32hf_blend();
    }
#endif
    
#ifdef __AVX512BF16__
    if (has_avx512f && has_avx512bf16) {
        test_v32bf_blend();
    }
#endif
    
    printf("All blend tests completed (if supported)\n");
    return 0;
}
