/* AVX-512 blend instruction coverage test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
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
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on data values */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(v1, v2);
        mask = ~mask;  /* Invert to ensure non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, v2);
        mask = mask ^ 0xAAAAAAAA;  /* XOR with pattern for non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HFmode - 32 half-precision float blend (AVX512-FP16) */
#if defined(__AVX512FP16__)
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512h v1 = _mm512_loadu_ph(src1 + i);
        __m512h v2 = _mm512_loadu_ph(src2 + i);
        
        /* Dynamic mask based on comparison */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_EQ_OQ);
        mask = mask ^ 0x55555555;  /* XOR with pattern */
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_ph(dst + i, result);
    }
}
#endif

/* V32BFmode - 32 bfloat16 blend (AVX512-BF16) */
#if defined(__AVX512BF16__)
#include <bfloat16.h>
__attribute__((target("avx512bf16,avx512f")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512bh v1 = _mm512_loadu_bf16(src1 + i);
        __m512bh v2 = _mm512_loadu_bf16(src2 + i);
        
        /* Convert to float for comparison to generate dynamic mask */
        __m512 v1f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(_mm512_castph_ps(v1))));
        __m512 v2f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(_mm512_castph_ps(v2))));
        
        __mmask16 mask_float = _mm512_cmp_ps_mask(v1f, v2f, _CMP_EQ_OQ);
        __mmask32 mask = _mm512_kunpackw(mask_float, mask_float);  /* Expand to 32 bits */
        mask = mask ^ 0xAAAAAAAA;  /* Make non-constant */
        
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_storeu_bf16(dst + i, result);
    }
}
#endif

/* V16SImode - 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on data parity */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
        mask = mask ^ (__mmask16)(i & 0xFFFF);  /* XOR with loop index */
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DImode - 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512i v1 = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i v2 = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask using comparison */
        __mmask8 mask = _mm512_cmpeq_epi64_mask(v1, v2);
        mask = mask ^ (__mmask8)((i >> 3) & 0xFF);  /* XOR with scaled index */
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DFmode - 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        /* Dynamic mask based on comparison */
        __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_EQ_OQ);
        mask = mask ^ (__mmask8)(i & 0xFF);  /* XOR with index */
        
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SFmode - 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        /* Dynamic mask using comparison */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_EQ_OQ);
        mask = mask ^ (__mmask16)((i * 7) & 0xFFFF);  /* XOR with scrambled index */
        
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_storeu_ps(dst + i, result);
    }
}

#ifdef __cplusplus
}
#endif

/* Helper function to initialize test data */
static void init_test_data(void* ptr1, void* ptr2, size_t size, int seed) {
    uint8_t* p1 = (uint8_t*)ptr1;
    uint8_t* p2 = (uint8_t*)ptr2;
    
    for (size_t i = 0; i < size; i++) {
        p1[i] = (uint8_t)((i * 13 + seed) & 0xFF);
        p2[i] = (uint8_t)((i * 17 + seed * 3) & 0xFF);
    }
}

/* Compute simple checksum to prevent dead code elimination */
static uint64_t compute_checksum(void* data, size_t size) {
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)data;
    
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;  /* Multiple of all vector sizes */
    uint64_t total_checksum = 0;
    
    /* Test V64QImode */
    {
        uint8_t src1[ARRAY_SIZE];
        uint8_t src2[ARRAY_SIZE];
        uint8_t dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 1);
        test_v64qimode(src1, src2, dst, ARRAY_SIZE);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
    /* Test V32HImode */
    {
        int16_t src1[ARRAY_SIZE];
        int16_t src2[ARRAY_SIZE];
        int16_t dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 2);
        test_v32himode(src1, src2, dst, ARRAY_SIZE);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
    /* Test V16SImode */
    {
        int32_t src1[ARRAY_SIZE];
        int32_t src2[ARRAY_SIZE];
        int32_t dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 3);
        test_v16simode(src1, src2, dst, ARRAY_SIZE);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
    /* Test V8DImode */
    {
        int64_t src1[ARRAY_SIZE];
        int64_t src2[ARRAY_SIZE];
        int64_t dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 4);
        test_v8dimode(src1, src2, dst, ARRAY_SIZE / 8);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
    /* Test V8DFmode */
    {
        double src1[ARRAY_SIZE];
        double src2[ARRAY_SIZE];
        double dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 5);
        test_v8dfmode(src1, src2, dst, ARRAY_SIZE / 8);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
    /* Test V16SFmode */
    {
        float src1[ARRAY_SIZE];
        float src2[ARRAY_SIZE];
        float dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 6);
        test_v16sfmode(src1, src2, dst, ARRAY_SIZE / 4);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
    
#if defined(__AVX512FP16__)
    /* Test V32HFmode */
    {
        _Float16 src1[ARRAY_SIZE];
        _Float16 src2[ARRAY_SIZE];
        _Float16 dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 7);
        test_v32hfmode(src1, src2, dst, ARRAY_SIZE / 2);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
#endif
    
#if defined(__AVX512BF16__)
    /* Test V32BFmode */
    {
        __bfloat16 src1[ARRAY_SIZE];
        __bfloat16 src2[ARRAY_SIZE];
        __bfloat16 dst[ARRAY_SIZE];
        
        init_test_data(src1, src2, sizeof(src1), 8);
        test_v32bfmode(src1, src2, dst, ARRAY_SIZE / 2);
        total_checksum += compute_checksum(dst, sizeof(dst));
    }
#endif
    
    printf("Total checksum: %lu\n", (unsigned long)total_checksum);
    return 0;
}
