/* AVX-512 blend coverage test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

#ifdef __AVX512FP16__
#define HAS_AVX512FP16 1
#else
#define HAS_AVX512FP16 0
#endif

#ifdef __AVX512BF16__
#define HAS_AVX512BF16 1
#else
#define HAS_AVX512BF16 0
#endif

/* Function attributes for specific ISA requirements */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode - 64-byte integers */
__attribute__((target("avx512bw")))
static uint64_t test_v64qimode(void) {
#if HAS_AVX512BW
    /* Create dynamic mask based on data pattern */
    __m512i a = _mm512_set1_epi8(0xAA);  /* 10101010 pattern */
    __m512i b = _mm512_set1_epi8(0x55);  /* 01010101 pattern */
    
    /* Generate mask: compare bytes for inequality */
    __mmask64 mask = _mm512_cmpneq_epi8_mask(a, b);
    
    /* Blend based on dynamic mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Force use of result */
    return _mm512_reduce_add_epi64(result);
#else
    return 0;
#endif
}

/* V32HImode - 32 half-word integers */
__attribute__((target("avx512bw")))
static uint64_t test_v32himode(void) {
#if HAS_AVX512BW
    /* Initialize with alternating patterns */
    __m512i a = _mm512_set1_epi16(0x1234);
    __m512i b = _mm512_set1_epi16(0x5678);
    
    /* Create dynamic mask: compare for greater-than (will be data-dependent) */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
    
    /* Blend with dynamic mask */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    return _mm512_reduce_add_epi64(result);
#else
    return 0;
#endif
}

/* V32HFmode - 32 half-precision floats (requires AVX512-FP16) */
__attribute__((target("avx512fp16,avx512bw")))
static float test_v32hfmode(void) {
#if HAS_AVX512FP16 && HAS_AVX512BW
    /* Initialize half-precision vectors */
    __m512h a = _mm512_set1_ph(1.5f);
    __m512h b = _mm512_set1_ph(2.5f);
    
    /* Create dynamic mask: compare for equality (will be false, creating pattern) */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    
    /* Blend half-precision vectors */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Convert to float for reduction */
    __m512 result_f = _mm512_cvtph_ps(result);
    return _mm512_reduce_add_ps(result_f);
#else
    return 0.0f;
#endif
}

/* V32BFmode - 32 bfloat16 floats (requires AVX512-BF16) */
__attribute__((target("avx512bf16,avx512bw")))
static float test_v32bfmode(void) {
#if HAS_AVX512BF16 && HAS_AVX512BW
    /* Initialize bfloat16 data */
    uint16_t bf_data_a[32], bf_data_b[32];
    for (int i = 0; i < 32; i++) {
        bf_data_a[i] = (i % 2) ? 0x3F80 : 0x4000;  /* 1.0f and 2.0f in bfloat16 */
        bf_data_b[i] = (i % 3) ? 0x4040 : 0x4080;  /* 3.0f and 4.0f in bfloat16 */
    }
    
    /* Load bfloat16 vectors */
    __m512bh a = _mm512_loadu_epi16(bf_data_a);
    __m512bh b = _mm512_loadu_epi16(bf_data_b);
    
    /* Convert to float for comparison */
    __m512 a_f = _mm512_cvtneobf16_ps(a);
    __m512 b_f = _mm512_cvtneobf16_ps(b);
    
    /* Create dynamic mask from float comparison */
    __mmask16 mask_f = _mm512_cmp_ps_mask(a_f, b_f, _CMP_LT_OQ);
    
    /* Expand mask for bfloat16 (2 bfloat16 per mask bit) */
    __mmask32 mask = _mm512_kunpackd(mask_f, mask_f);
    
    /* Blend bfloat16 vectors */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Convert to float and reduce */
    __m512 result_f = _mm512_cvtneobf16_ps(result);
    return _mm512_reduce_add_ps(result_f);
#else
    return 0.0f;
#endif
}

/* V16SImode - 16 single-word integers */
__attribute__((target("avx512f")))
static uint64_t test_v16simode(void) {
#if HAS_AVX512F
    /* Initialize with pattern */
    __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,
                                 23,22,21,20,19,18,17,16);
    __m512i b = _mm512_set_epi32(15,14,13,12,11,10,9,8,
                                 7,6,5,4,3,2,1,0);
    
    /* Dynamic mask: compare for equality with shifted pattern */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(a, _mm512_srli_epi32(b, 1));
    
    /* Blend 32-bit integers */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    return _mm512_reduce_add_epi64(result);
#else
    return 0;
#endif
}

/* V8DImode - 8 double-word integers */
__attribute__((target("avx512f")))
static uint64_t test_v8dimode(void) {
#if HAS_AVX512F
    /* Initialize with pattern */
    __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi64(15,14,13,12,11,10,9,8);
    
    /* Dynamic mask: test specific bits */
    __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
    
    /* Blend 64-bit integers */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    return _mm512_reduce_add_epi64(result);
#else
    return 0;
#endif
}

/* V8DFmode - 8 double-precision floats */
__attribute__((target("avx512f")))
static double test_v8dfmode(void) {
#if HAS_AVX512F
    /* Initialize doubles */
    __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d b = _mm512_set_pd(15.0,14.0,13.0,12.0,11.0,10.0,9.0,8.0);
    
    /* Dynamic mask: compare for less-than */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* Blend doubles */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    return _mm512_reduce_add_pd(result);
#else
    return 0.0;
#endif
}

/* V16SFmode - 16 single-precision floats */
__attribute__((target("avx512f")))
static float test_v16sfmode(void) {
#if HAS_AVX512F
    /* Initialize floats */
    __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                             7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
    __m512 b = _mm512_set_ps(31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
                             23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f);
    
    /* Dynamic mask: compare for not-equal */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_NEQ_OQ);
    
    /* Blend floats */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    return _mm512_reduce_add_ps(result);
#else
    return 0.0f;
#endif
}

#ifdef __cplusplus
}
#endif

int main(void) {
    uint64_t checksum = 0;
    
    printf("Testing AVX-512 blend instruction coverage...\n");
    
    /* Test each vector mode with dynamic masks */
    checksum += test_v64qimode();
    printf("V64QImode test completed\n");
    
    checksum += test_v32himode();
    printf("V32HImode test completed\n");
    
    checksum += (uint64_t)test_v32hfmode();
    printf("V32HFmode test completed\n");
    
    checksum += (uint64_t)test_v32bfmode();
    printf("V32BFmode test completed\n");
    
    checksum += test_v16simode();
    printf("V16SImode test completed\n");
    
    checksum += test_v8dimode();
    printf("V8DImode test completed\n");
    
    checksum += (uint64_t)test_v8dfmode();
    printf("V8DFmode test completed\n");
    
    checksum += (uint64_t)test_v16sfmode();
    printf("V16SFmode test completed\n");
    
    printf("Final checksum: %lu\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0) {
        printf("Warning: Some tests may have been optimized away\n");
    }
    
    return 0;
}
