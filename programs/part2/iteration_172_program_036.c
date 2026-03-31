/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Check for required ISA support */
#if !defined(__AVX512F__)
#error "AVX-512F is required for this test"
#endif

#if !defined(__AVX512BW__)
#error "AVX-512BW is required for this test"
#endif

/* Function attributes to ensure specific ISA usage */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Create dynamic mask based on iteration and data */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
        mask = mask ^ (__mmask64)(i * 0x5555555555555555ULL);
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Rotate data to create variation */
        uint8_t temp = src1[0];
        memmove(src1, src1 + 1, 63);
        src1[63] = temp;
    }
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask using comparison and iteration */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        mask = mask ^ (__mmask32)(i * 0x55555555U);
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Modify data slightly */
        src1[i % 32] += i;
    }
}

/* V32HFmode - 32 half-precision float blend (requires AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512h a = _mm512_loadu_ph(src1);
        __m512h b = _mm512_loadu_ph(src2);
        
        /* Create mask using comparison */
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ (__mmask32)(i * 0xAAAAAAAAU);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph(dst, result);
        
        /* Slight data modification */
        src1[i % 32] += (_Float16)(i * 0.1f);
    }
}
#endif

/* V32BFmode - 32 bfloat16 blend (requires AVX512-BF16) */
#ifdef __AVX512BF16__
#include <bfloat16.h>
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Load bfloat16 data */
        __m512bh a = _mm512_loadu_bf16(src1);
        __m512bh b = _mm512_loadu_bf16(src2);
        
        /* Convert to float for comparison */
        __m512 a_f32 = _mm512_cvtneobf16_ps(a);
        __m512 b_f32 = _mm512_cvtneobf16_ps(b);
        
        /* Create mask from float comparison */
        __mmask16 mask32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_EQ_OQ);
        __mmask32 mask = _mm512_kunpackw(mask32, mask32);
        mask = mask ^ (__mmask32)(i * 0xCCCCCCCCU);
        
        /* Blend bfloat16 vectors */
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_bf16(dst, result);
        
        /* Modify data */
        src1[i % 32].value += i & 0xFF;
    }
}
#endif

/* V16SImode - 32-bit integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask using multiple conditions */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        mask = mask ^ (__mmask16)((i * 0x5555) & 0xFFFF);
        mask = mask | (__mmask16)((i & 1) ? 0xAAAA : 0x5555);
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Update source data */
        src1[i % 16] ^= i;
    }
}

/* V8DImode - 64-bit integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Complex mask generation */
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
        mask = mask ^ (__mmask8)((i * 0x55) & 0xFF);
        mask = mask & (__mmask8)(0x0F);
        mask = mask | (__mmask8)((i % 3) ? 0xF0 : 0x0F);
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Modify data */
        src1[i % 8] += i * 1000LL;
    }
}

/* V8DFmode - double precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_loadu_pd(src1);
        __m512d b = _mm512_loadu_pd(src2);
        
        /* Mask from comparison with threshold */
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        mask = mask ^ (__mmask8)((i * 0x33) & 0xFF);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd(dst, result);
        
        /* Update source */
        src1[i % 8] += (i % 2) ? 0.1 : -0.1;
    }
}

/* V16SFmode - single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_loadu_ps(src1);
        __m512 b = _mm512_loadu_ps(src2);
        
        /* Complex mask generation */
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
        mask = mask ^ (__mmask16)((i * 0x3333) & 0xFFFF);
        mask = mask & (__mmask16)0xAAAA;
        mask = mask | (__mmask16)((i & 1) ? 0x5555 : 0xAAAA);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps(dst, result);
        
        /* Modify source data */
        src1[i % 16] += (float)((i % 5) * 0.01f);
    }
}

#ifdef __cplusplus
}
#endif

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    /* Data will be initialized in main to avoid large static arrays */
}

int main(void) {
    const int ITERATIONS = 100;
    uint64_t checksum = 0;
    
    /* Test V64QImode */
    {
        uint8_t src1[64], src2[64], dst[64];
        for (int i = 0; i < 64; i++) {
            src1[i] = (uint8_t)(i * 3);
            src2[i] = (uint8_t)(i * 5 + 1);
        }
        test_v64qimode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 64; i++) checksum += dst[i];
    }
    
    /* Test V32HImode */
    {
        int16_t src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = (int16_t)(i * 100);
            src2[i] = (int16_t)(i * 200 + 50);
        }
        test_v32himode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 32; i++) checksum += dst[i];
    }
    
    /* Test V32HFmode if supported */
#ifdef __AVX512FP16__
    {
        _Float16 src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = (_Float16)(i * 0.5f);
            src2[i] = (_Float16)(i * 0.7f + 0.1f);
        }
        test_v32hfmode(src1, src2, dst, ITERATIONS);
        /* Convert to integer for checksum */
        for (int i = 0; i < 32; i++) checksum += (uint16_t)dst[i];
    }
#endif
    
    /* Test V32BFmode if supported */
#ifdef __AVX512BF16__
    {
        __bfloat16 src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i].value = (uint16_t)(i * 0x100);
            src2[i].value = (uint16_t)(i * 0x200 + 0x80);
        }
        test_v32bfmode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 32; i++) checksum += dst[i].value;
    }
#endif
    
    /* Test V16SImode */
    {
        int32_t src1[16], src2[16], dst[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 1000;
            src2[i] = i * 2000 + 500;
        }
        test_v16simode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 16; i++) checksum += dst[i];
    }
    
    /* Test V8DImode */
    {
        int64_t src1[8], src2[8], dst[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 10000LL;
            src2[i] = i * 20000LL + 5000LL;
        }
        test_v8dimode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 8; i++) checksum += (uint64_t)dst[i];
    }
    
    /* Test V8DFmode */
    {
        double src1[8], src2[8], dst[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 1.5;
            src2[i] = i * 2.5 + 0.5;
        }
        test_v8dfmode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 8; i++) checksum += (uint64_t)(dst[i] * 1000.0);
    }
    
    /* Test V16SFmode */
    {
        float src1[16], src2[16], dst[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 0.75f;
            src2[i] = i * 1.25f + 0.25f;
        }
        test_v16sfmode(src1, src2, dst, ITERATIONS);
        for (int i = 0; i < 16; i++) checksum += (uint64_t)(dst[i] * 1000.0f);
    }
    
    printf("Blend test checksum: %lu\n", checksum);
    return 0;
}
