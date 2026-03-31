/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Check for required ISA support */
#if !defined(__AVX512F__)
#error "AVX-512F required for this test"
#endif

#if !defined(__AVX512BW__)
#error "AVX-512BW required for this test"
#endif

/* Function attributes for specific ISA requirements */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int len) {
    for (int i = 0; i < len; i += 64) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on data values */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
        mask = ~mask;  /* Invert to ensure non-constant mask */
        
        /* Blend based on uncovered line: gen_avx512bw_blendmv64qi */
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask using comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        mask = mask ^ 0xAAAAAAAA;  /* XOR with pattern for non-constant mask */
        
        /* Blend based on uncovered line: gen_avx512bw_blendmv32hi */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HFmode - 32 half-precision float blend (requires AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512h a = _mm512_loadu_ph(src1 + i);
        __m512h b = _mm512_loadu_ph(src2 + i);
        
        /* Create dynamic mask using comparison */
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ (__mmask32)(i & 0xFFFFFFFF);  /* XOR with index */
        
        /* Blend based on uncovered line: gen_avx512bw_blendmv32hf */
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph(dst + i, result);
    }
}
#endif

/* V32BFmode - 32 bfloat16 blend (requires AVX512-BF16) */
#ifdef __AVX512BF16__
#include <bfloat16.h>
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        /* Load bfloat16 data */
        __m512bh a = _mm512_loadu_bf16(src1 + i);
        __m512bh b = _mm512_loadu_bf16(src2 + i);
        
        /* Convert to float for comparison to create mask */
        __m512 a_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(a)));
        __m512 b_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(b)));
        
        __mmask16 mask32 = _mm512_cmp_ps_mask(a_f, b_f, _CMP_EQ_OQ);
        __mmask32 mask = _mm512_kunpackw(mask32, mask32);  /* Expand to 32 bits */
        mask = mask ^ (__mmask32)(i & 0x55555555);  /* XOR with pattern */
        
        /* Blend based on uncovered line: gen_avx512bw_blendmv32bf */
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_bf16(dst + i, result);
    }
}
#endif

/* V16SImode - 32-bit integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask using comparison */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        mask = mask ^ (__mmask16)(i & 0xFFFF);  /* XOR with index */
        
        /* Blend based on uncovered line: gen_avx512f_blendmv16si */
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DImode - 64-bit integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask using comparison */
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
        mask = mask ^ (__mmask8)(i & 0xFF);  /* XOR with index */
        
        /* Blend based on uncovered line: gen_avx512f_blendmv8di */
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DFmode - double precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512d a = _mm512_loadu_pd(src1 + i);
        __m512d b = _mm512_loadu_pd(src2 + i);
        
        /* Dynamic mask using comparison */
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ (__mmask8)(i & 0xAA);  /* XOR with pattern */
        
        /* Blend based on uncovered line: gen_avx512f_blendmv8df */
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SFmode - single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512 a = _mm512_loadu_ps(src1 + i);
        __m512 b = _mm512_loadu_ps(src2 + i);
        
        /* Dynamic mask using comparison */
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ (__mmask16)(i & 0xAAAA);  /* XOR with pattern */
        
        /* Blend based on uncovered line: gen_avx512f_blendmv16sf */
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps(dst + i, result);
    }
}

#ifdef __cplusplus
}
#endif

/* Initialize test data */
static void init_data(uint8_t* data, size_t size, int seed) {
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)((i * 1103515245 + seed) & 0xFF);
    }
}

/* Simple checksum to prevent dead code elimination */
static uint64_t checksum(void* data, size_t size) {
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main() {
    const int ARRAY_SIZE = 1024;
    uint64_t total_checksum = 0;
    
    /* Allocate aligned buffers for better performance */
    uint8_t* src1 = (uint8_t*)_mm_malloc(ARRAY_SIZE * 64, 64);
    uint8_t* src2 = (uint8_t*)_mm_malloc(ARRAY_SIZE * 64, 64);
    uint8_t* dst = (uint8_t*)_mm_malloc(ARRAY_SIZE * 64, 64);
    
    if (!src1 || !src2 || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source data */
    init_data(src1, ARRAY_SIZE * 64, 1);
    init_data(src2, ARRAY_SIZE * 64, 2);
    
    printf("Testing AVX-512 blend operations...\n");
    
    /* Test each vector mode */
    test_v64qimode(src1, src2, dst, ARRAY_SIZE * 64);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
    test_v32himode((int16_t*)src1, (int16_t*)src2, (int16_t*)dst, ARRAY_SIZE * 32);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
    test_v16simode((int32_t*)src1, (int32_t*)src2, (int32_t*)dst, ARRAY_SIZE * 16);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
    test_v8dimode((int64_t*)src1, (int64_t*)src2, (int64_t*)dst, ARRAY_SIZE * 8);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
    test_v8dfmode((double*)src1, (double*)src2, (double*)dst, ARRAY_SIZE * 8);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
    test_v16sfmode((float*)src1, (float*)src2, (float*)dst, ARRAY_SIZE * 16);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    
#ifdef __AVX512FP16__
    test_v32hfmode((_Float16*)src1, (_Float16*)src2, (_Float16*)dst, ARRAY_SIZE * 32);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    printf("  V32HFmode tested (AVX512-FP16)\n");
#else
    printf("  V32HFmode skipped (AVX512-FP16 not available)\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bfmode((__bfloat16*)src1, (__bfloat16*)src2, (__bfloat16*)dst, ARRAY_SIZE * 32);
    total_checksum += checksum(dst, ARRAY_SIZE * 64);
    printf("  V32BFmode tested (AVX512-BF16)\n");
#else
    printf("  V32BFmode skipped (AVX512-BF16 not available)\n");
#endif
    
    printf("All tests completed. Total checksum: %llu\n", 
           (unsigned long long)total_checksum);
    
    /* Cleanup */
    _mm_free(src1);
    _mm_free(src2);
    _mm_free(dst);
    
    return 0;
}
