/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Ensure we have the necessary ISA support */
#if !defined(__AVX512F__) || !defined(__AVX512BW__)
#error "AVX-512F and AVX-512BW are required for this test"
#endif

/* Function attributes to ensure proper ISA usage */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(v1, v2);
    /* Invert some bits to ensure non-constant mask */
    mask ^= 0xAAAAAAAAAAAAAAAAULL;
    
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, v2);
    mask ^= 0x55555555;  /* Make mask non-constant */
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V32HFmode - 32 half-precision float blend (requires AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    /* Create mask by comparing with zero */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, _mm512_setzero_ph(), _CMP_EQ_OQ);
    /* Make mask non-constant */
    mask ^= 0xAAAAAAAA;
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_ph(dst, result);
}
#endif

/* V32BFmode - 32 bfloat16 blend (requires AVX512-BF16) */
#ifdef __AVX512BF16__
#include <bfloat16.h>
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst) {
    __m512bh v1 = _mm512_loadu_bf16(src1);
    __m512bh v2 = _mm512_loadu_bf16(src2);
    
    /* Use comparison to create dynamic mask */
    __m512h v1_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(_mm512_loadu_si512((__m512i*)src1)));
    __m512h v2_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(_mm512_loadu_si512((__m512i*)src2)));
    
    __mmask32 mask = _mm512_cmp_ph_mask(v1_h, v2_h, _CMP_EQ_OQ);
    mask ^= 0x55555555;  /* Non-constant mask */
    
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_bf16(dst, result);
}
#endif

/* V16SImode - 32-bit integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
    mask ^= 0xAAAA;  /* Non-constant */
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DImode - 64-bit integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask8 mask = _mm512_cmpeq_epi64_mask(v1, v2);
    mask ^= 0xAA;  /* Non-constant mask */
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DFmode - double precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    /* Create dynamic mask by comparing with threshold */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(0.5), _CMP_GT_OQ);
    mask ^= 0x55;  /* Make non-constant */
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_storeu_pd(dst, result);
}

/* V16SFmode - single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
    mask ^= 0xAAAA;  /* Non-constant */
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    _mm512_storeu_ps(dst, result);
}

#ifdef __cplusplus
}
#endif

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    /* Data will be initialized in main to avoid large static arrays */
}

int main(void) {
    /* Allocate aligned buffers for all vector types */
    uint8_t   src1_qi[64] __attribute__((aligned(64)));
    uint8_t   src2_qi[64] __attribute__((aligned(64)));
    uint8_t   dst_qi[64]  __attribute__((aligned(64)));
    
    int16_t   src1_hi[32] __attribute__((aligned(64)));
    int16_t   src2_hi[32] __attribute__((aligned(64)));
    int16_t   dst_hi[32]  __attribute__((aligned(64)));
    
    _Float16  src1_hf[32] __attribute__((aligned(64)));
    _Float16  src2_hf[32] __attribute__((aligned(64)));
    _Float16  dst_hf[32]  __attribute__((aligned(64)));
    
    __bfloat16 src1_bf[32] __attribute__((aligned(64)));
    __bfloat16 src2_bf[32] __attribute__((aligned(64)));
    __bfloat16 dst_bf[32]  __attribute__((aligned(64)));
    
    int32_t   src1_si[16] __attribute__((aligned(64)));
    int32_t   src2_si[16] __attribute__((aligned(64)));
    int32_t   dst_si[16]  __attribute__((aligned(64)));
    
    int64_t   src1_di[8]  __attribute__((aligned(64)));
    int64_t   src2_di[8]  __attribute__((aligned(64)));
    int64_t   dst_di[8]   __attribute__((aligned(64)));
    
    double    src1_df[8]  __attribute__((aligned(64)));
    double    src2_df[8]  __attribute__((aligned(64)));
    double    dst_df[8]   __attribute__((aligned(64)));
    
    float     src1_sf[16] __attribute__((aligned(64)));
    float     src2_sf[16] __attribute__((aligned(64)));
    float     dst_sf[16]  __attribute__((aligned(64)));
    
    /* Initialize with distinct patterns to ensure non-constant masks */
    for (int i = 0; i < 64; i++) {
        src1_qi[i] = i;
        src2_qi[i] = 64 - i;
    }
    
    for (int i = 0; i < 32; i++) {
        src1_hi[i] = i * 2;
        src2_hi[i] = i * 3;
        src1_hf[i] = (_Float16)(i * 0.1f);
        src2_hf[i] = (_Float16)(i * 0.2f);
        src1_bf[i] = (__bfloat16)(i * 0.1f);
        src2_bf[i] = (__bfloat16)(i * 0.2f);
    }
    
    for (int i = 0; i < 16; i++) {
        src1_si[i] = i * 100;
        src2_si[i] = i * 200;
        src1_sf[i] = i * 0.5f;
        src2_sf[i] = i * 1.5f;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_di[i] = i * 1000LL;
        src2_di[i] = i * 2000LL;
        src1_df[i] = i * 0.25;
        src2_df[i] = i * 0.75;
    }
    
    /* Execute all blend tests in a loop to prevent optimization */
    volatile int iterations = 3;  /* Prevent loop unrolling */
    long long checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test each vector mode */
        test_v64qimode(src1_qi, src2_qi, dst_qi);
        test_v32himode(src1_hi, src2_hi, dst_hi);
        
        #ifdef __AVX512FP16__
        test_v32hfmode(src1_hf, src2_hf, dst_hf);
        #endif
        
        #ifdef __AVX512BF16__
        test_v32bfmode(src1_bf, src2_bf, dst_bf);
        #endif
        
        test_v16simode(src1_si, src2_si, dst_si);
        test_v8dimode(src1_di, src2_di, dst_di);
        test_v8dfmode(src1_df, src2_df, dst_df);
        test_v16sfmode(src1_sf, src2_sf, dst_sf);
        
        /* Compute checksum to ensure all blends are executed */
        for (int i = 0; i < 64; i++) checksum += dst_qi[i];
        for (int i = 0; i < 32; i++) checksum += dst_hi[i];
        for (int i = 0; i < 16; i++) checksum += dst_si[i];
        for (int i = 0; i < 8; i++)  checksum += (long long)dst_di[i];
    }
    
    printf("Blend test completed. Checksum: %lld\n", checksum);
    return 0;
}
