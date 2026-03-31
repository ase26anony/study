/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent aggressive optimization */
static volatile int g_volatile_mask = 0x55555555;

/* Function prototypes with target attributes */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(uint8_t* src1, uint8_t* src2, uint8_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(v1, v2);
    /* Mix with volatile to prevent constant folding */
    mask ^= (__mmask64)g_volatile_mask;
    
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask using comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, v2);
    mask ^= (__mmask32)(g_volatile_mask & 0xFFFFFFFF);
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
    mask ^= (__mmask16)(g_volatile_mask & 0xFFFF);
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __mmask8 mask = _mm512_cmpeq_epi64_mask(v1, v2);
    mask ^= (__mmask8)(g_volatile_mask & 0xFF);
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(double* src1, double* src2, double* dst) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_EQ_OQ);
    mask ^= (__mmask8)(g_volatile_mask & 0xFF);
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_storeu_pd(dst, result);
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(float* src1, float* src2, float* dst) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_EQ_OQ);
    mask ^= (__mmask16)(g_volatile_mask & 0xFFFF);
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    _mm512_storeu_ps(dst, result);
}

#endif /* __AVX512F__ */

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_EQ_OQ);
    mask ^= (__mmask32)(g_volatile_mask & 0xFFFFFFFF);
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_ph(dst, result);
}

#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__

/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(__bf16* src1, __bf16* src2, __bf16* dst) {
    /* Load bfloat16 data - need to use loadu_si512 since there's no direct bfloat16 load */
    __m512i v1_i = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2_i = _mm512_loadu_si512((__m512i*)src2);
    
    /* Convert to float for comparison */
    __m512 v1_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(v1_i));
    __m512 v2_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(v2_i));
    
    __mmask16 mask_f = _mm512_cmp_ps_mask(v1_f, v2_f, _CMP_EQ_OQ);
    __mmask32 mask = _mm512_kunpackd(mask_f, mask_f);
    mask ^= (__mmask32)(g_volatile_mask & 0xFFFFFFFF);
    
    /* Blend at bfloat16 level */
    __m512i result_i = _mm512_mask_blend_epi16(mask, v1_i, v2_i);
    _mm512_storeu_si512((__m512i*)dst, result_i);
}

#endif /* __AVX512BF16__ */

#ifdef __cplusplus
}
#endif

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    /* Data will be initialized in main to avoid large static arrays */
}

int main(void) {
    /* Initialize test data arrays */
    uint8_t src1_v64qi[64], src2_v64qi[64], dst_v64qi[64];
    int16_t src1_v32hi[32], src2_v32hi[32], dst_v32hi[32];
    int32_t src1_v16si[16], src2_v16si[16], dst_v16si[16];
    int64_t src1_v8di[8], src2_v8di[8], dst_v8di[8];
    double src1_v8df[8], src2_v8df[8], dst_v8df[8];
    float src1_v16sf[16], src2_v16sf[16], dst_v16sf[16];
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        src1_v64qi[i] = i;
        src2_v64qi[i] = 64 - i;
    }
    
    for (int i = 0; i < 32; i++) {
        src1_v32hi[i] = i * 2;
        src2_v32hi[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        src1_v16si[i] = i * 100;
        src2_v16si[i] = i * 200;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_v8di[i] = i * 1000LL;
        src2_v8di[i] = i * 2000LL;
        src1_v8df[i] = i * 1.5;
        src2_v8df[i] = i * 2.5;
    }
    
    for (int i = 0; i < 16; i++) {
        src1_v16sf[i] = i * 0.5f;
        src2_v16sf[i] = i * 1.5f;
    }
    
    /* Execute blend tests in a loop to ensure coverage */
    for (int iter = 0; iter < 10; iter++) {
        g_volatile_mask = iter * 0x1234567;
        
#ifdef __AVX512F__
#ifdef __AVX512BW__
        test_v64qi_blend(src1_v64qi, src2_v64qi, dst_v64qi);
        test_v32hi_blend(src1_v32hi, src2_v32hi, dst_v32hi);
#endif /* __AVX512BW__ */
        
        test_v16si_blend(src1_v16si, src2_v16si, dst_v16si);
        test_v8di_blend(src1_v8di, src2_v8di, dst_v8di);
        test_v8df_blend(src1_v8df, src2_v8df, dst_v8df);
        test_v16sf_blend(src1_v16sf, src2_v16sf, dst_v16sf);
#endif /* __AVX512F__ */
        
#ifdef __AVX512FP16__
        _Float16 src1_v32hf[32], src2_v32hf[32], dst_v32hf[32];
        for (int i = 0; i < 32; i++) {
            src1_v32hf[i] = (_Float16)(i * 0.25f);
            src2_v32hf[i] = (_Float16)(i * 0.75f);
        }
        test_v32hf_blend(src1_v32hf, src2_v32hf, dst_v32hf);
#endif
        
#ifdef __AVX512BF16__
        __bf16 src1_v32bf[32], src2_v32bf[32], dst_v32bf[32];
        for (int i = 0; i < 32; i++) {
            /* Simple bfloat16 pattern */
            uint16_t val1 = (i * 10) << 8;
            uint16_t val2 = (i * 20) << 8;
            memcpy(&src1_v32bf[i], &val1, sizeof(__bf16));
            memcpy(&src2_v32bf[i], &val2, sizeof(__bf16));
        }
        test_v32bf_blend(src1_v32bf, src2_v32bf, dst_v32bf);
#endif
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) checksum += dst_v64qi[i];
    for (int i = 0; i < 32; i++) checksum += dst_v32hi[i];
    for (int i = 0; i < 16; i++) checksum += dst_v16si[i];
    for (int i = 0; i < 8; i++) checksum += dst_v8di[i];
    
    /* Cast floating point results to integer for checksum */
    for (int i = 0; i < 8; i++) {
        uint64_t tmp;
        memcpy(&tmp, &dst_v8df[i], sizeof(double));
        checksum += tmp;
    }
    
    for (int i = 0; i < 16; i++) {
        uint32_t tmp;
        memcpy(&tmp, &dst_v16sf[i], sizeof(float));
        checksum += tmp;
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("All AVX-512 blend tests completed.\n");
    
    return 0;
}
