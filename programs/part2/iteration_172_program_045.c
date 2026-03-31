/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Enable all required AVX-512 extensions */
#ifdef __AVX512F__
#ifdef __AVX512BW__
#ifdef __AVX512FP16__

/* Function to test V64QImode blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst) {
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
    /* Flip some bits to ensure non-constant mask */
    mask ^= 0xAAAAAAAAAAAAAAAAULL;
    
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* Function to test V32HImode blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
    mask ^= 0x55555555;  /* Make mask non-constant */
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* Function to test V32HFmode blend (requires AVX512-FP16) */
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    /* Create mask by comparing with threshold */
    __m512h threshold = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    _mm512_storeu_ph(dst, result);
}

/* Function to test V32BFmode blend (requires AVX512-BF16) */
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bfmode(__bf16* src1, __bf16* src2, __bf16* dst) {
    /* Load bfloat16 data */
    __m512bh a = _mm512_loadu_si512((__m512i*)src1);
    __m512bh b = _mm512_loadu_si512((__m512i*)src2);
    
    /* Convert to float for comparison */
    __m512 a_f32 = _mm512_cvtneobf16_ps(a);
    __m512 b_f32 = _mm512_cvtneobf16_ps(b);
    
    /* Create mask from float comparison */
    __mmask16 mask_float = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_NEQ_UQ);
    
    /* Expand 16-bit mask to 32-bit for blend */
    __mmask32 mask = _mm512_kunpackd(mask_float, mask_float);
    
    /* Use the same intrinsic as FP16 since bfloat16 uses the same */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    _mm512_storeu_si512((__m512i*)dst, (__m512i)result);
}

/* Function to test V16SImode blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_setzero_si512());
    mask ^= 0xAAAA;  /* Make non-constant */
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* Function to test V8DImode blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    /* Mask based on parity of values */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(
        _mm512_and_si512(a, _mm512_set1_epi64(1)),
        _mm512_setzero_si512()
    );
    mask ^= 0xAA;  /* Make non-constant */
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    _mm512_storeu_si512((__m512i*)dst, result);
}

/* Function to test V8DFmode blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst) {
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    /* Dynamic mask based on value range */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(0.0), _CMP_GT_OQ);
    mask ^= 0x55;  /* Make non-constant */
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    _mm512_storeu_pd(dst, result);
}

/* Function to test V16SFmode blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst) {
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    /* Dynamic mask based on comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    mask ^= 0xAAAA;  /* Make non-constant */
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    _mm512_storeu_ps(dst, result);
}

#endif /* __AVX512FP16__ */
#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main() {
    /* Initialize test data arrays */
    uint8_t src1_bytes[64], src2_bytes[64], dst_bytes[64];
    int16_t src1_words[32], src2_words[32], dst_words[32];
    int32_t src1_dwords[16], src2_dwords[16], dst_dwords[16];
    int64_t src1_qwords[8], src2_qwords[8], dst_qwords[8];
    float src1_floats[16], src2_floats[16], dst_floats[16];
    double src1_doubles[8], src2_doubles[8], dst_doubles[8];
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        src1_bytes[i] = i;
        src2_bytes[i] = 64 - i;
    }
    for (int i = 0; i < 32; i++) {
        src1_words[i] = i * 100;
        src2_words[i] = 3200 - i * 100;
    }
    for (int i = 0; i < 16; i++) {
        src1_dwords[i] = i * 1000;
        src2_dwords[i] = 16000 - i * 1000;
    }
    for (int i = 0; i < 8; i++) {
        src1_qwords[i] = i * 10000LL;
        src2_qwords[i] = 80000LL - i * 10000LL;
    }
    for (int i = 0; i < 16; i++) {
        src1_floats[i] = i * 1.5f;
        src2_floats[i] = 24.0f - i * 1.5f;
    }
    for (int i = 0; i < 8; i++) {
        src1_doubles[i] = i * 2.5;
        src2_doubles[i] = 20.0 - i * 2.5;
    }
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    /* Test all integer and float/double blends */
    test_v64qimode(src1_bytes, src2_bytes, dst_bytes);
    test_v32himode(src1_words, src2_words, dst_words);
    test_v16simode(src1_dwords, src2_dwords, dst_dwords);
    test_v8dimode(src1_qwords, src2_qwords, dst_qwords);
    test_v8dfmode(src1_doubles, src2_doubles, dst_doubles);
    test_v16sfmode(src1_floats, src2_floats, dst_floats);
    
#ifdef __AVX512FP16__
    /* Test FP16 blends */
    _Float16 src1_half[32], src2_half[32], dst_half[32];
    for (int i = 0; i < 32; i++) {
        src1_half[i] = (_Float16)(i * 0.5f);
        src2_half[i] = (_Float16)(16.0f - i * 0.5f);
    }
    test_v32hfmode(src1_half, src2_half, dst_half);
#endif
    
#ifdef __AVX512BF16__
    /* Test BF16 blends */
    __bf16 src1_bf16[32], src2_bf16[32], dst_bf16[32];
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 values */
        uint16_t val = (i & 0x7F) << 7;  /* Simple pattern */
        src1_bf16[i] = *(__bf16*)&val;
        src2_bf16[i] = *(__bf16*)&(uint16_t){0x3F80};  /* 1.0 in bfloat16 */
    }
    test_v32bfmode(src1_bf16, src2_bf16, dst_bf16);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum += dst_bytes[i];
    for (int i = 0; i < 32; i++) checksum += dst_words[i];
    for (int i = 0; i < 16; i++) checksum += dst_dwords[i];
    for (int i = 0; i < 8; i++) checksum += dst_qwords[i];
    
    printf("AVX-512 Blend Test Completed. Checksum: %lu\n", checksum);
    
#else
    printf("AVX512BW not supported. Skipping tests.\n");
#endif
#else
    printf("AVX512F not supported. Skipping tests.\n");
#endif
    
    return 0;
}
