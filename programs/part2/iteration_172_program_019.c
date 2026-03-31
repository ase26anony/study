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

/* Function attributes to ensure specific ISA usage */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int len) {
    for (int i = 0; i < len; i += 64) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on data pattern */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
        mask = ~mask;  /* Invert to ensure non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        mask = mask ^ 0xAAAAAAAA;  /* XOR with pattern for non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        __m512h a = _mm512_loadu_ph(src1 + i);
        __m512h b = _mm512_loadu_ph(src2 + i);
        
        /* Dynamic mask: compare for equality */
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ 0x55555555;  /* XOR for non-constant mask */
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph(dst + i, result);
    }
}

#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__

/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bf16* src1, __bf16* src2, __bf16* dst, int len) {
    for (int i = 0; i < len; i += 32) {
        /* Load bfloat16 data */
        __m512bh a = _mm512_loadu_bf16(src1 + i);
        __m512bh b = _mm512_loadu_bf16(src2 + i);
        
        /* Convert to float for comparison to generate dynamic mask */
        __m512 a_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(_mm512_castph_ps(a))));
        __m512 b_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castps_si512(_mm512_castph_ps(b))));
        
        __mmask16 mask32 = _mm512_cmp_ps_mask(a_f, b_f, _CMP_EQ_OQ);
        __mmask32 mask = _mm512_kunpackd(mask32, mask32);  /* Expand to 32 bits */
        mask = mask ^ 0x33333333;  /* XOR for non-constant mask */
        
        /* Use blend intrinsic */
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_bf16(dst + i, result);
    }
}

#endif /* __AVX512BF16__ */

/* V16SImode: 32-bit integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on comparison */
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        mask = mask ^ 0xAAAA;  /* XOR for non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DImode: 64-bit integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        
        /* Dynamic mask based on comparison */
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
        mask = mask ^ 0x55;  /* XOR for non-constant mask */
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
}

/* V8DFmode: double precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int len) {
    for (int i = 0; i < len; i += 8) {
        __m512d a = _mm512_loadu_pd(src1 + i);
        __m512d b = _mm512_loadu_pd(src2 + i);
        
        /* Dynamic mask based on comparison */
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ 0xAA;  /* XOR for non-constant mask */
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd(dst + i, result);
    }
}

/* V16SFmode: single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int len) {
    for (int i = 0; i < len; i += 16) {
        __m512 a = _mm512_loadu_ps(src1 + i);
        __m512 b = _mm512_loadu_ps(src2 + i);
        
        /* Dynamic mask based on comparison */
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
        mask = mask ^ 0xAAAA;  /* XOR for non-constant mask */
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps(dst + i, result);
    }
}

#endif /* __AVX512F__ */

#ifdef __cplusplus
}
#endif

int main() {
    const int ARRAY_SIZE = 1024;
    
    /* Initialize test data arrays */
    uint8_t src1_u8[ARRAY_SIZE];
    uint8_t src2_u8[ARRAY_SIZE];
    uint8_t dst_u8[ARRAY_SIZE];
    
    int16_t src1_i16[ARRAY_SIZE];
    int16_t src2_i16[ARRAY_SIZE];
    int16_t dst_i16[ARRAY_SIZE];
    
    int32_t src1_i32[ARRAY_SIZE];
    int32_t src2_i32[ARRAY_SIZE];
    int32_t dst_i32[ARRAY_SIZE];
    
    int64_t src1_i64[ARRAY_SIZE];
    int64_t src2_i64[ARRAY_SIZE];
    int64_t dst_i64[ARRAY_SIZE];
    
    float src1_f32[ARRAY_SIZE];
    float src2_f32[ARRAY_SIZE];
    float dst_f32[ARRAY_SIZE];
    
    double src1_f64[ARRAY_SIZE];
    double src2_f64[ARRAY_SIZE];
    double dst_f64[ARRAY_SIZE];
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_u8[i] = i & 0xFF;
        src2_u8[i] = (i + 128) & 0xFF;
        
        src1_i16[i] = i;
        src2_i16[i] = i + 1000;
        
        src1_i32[i] = i * 2;
        src2_i32[i] = i * 3;
        
        src1_i64[i] = i * 5LL;
        src2_i64[i] = i * 7LL;
        
        src1_f32[i] = i * 1.5f;
        src2_f32[i] = i * 2.5f;
        
        src1_f64[i] = i * 1.25;
        src2_f64[i] = i * 1.75;
    }
    
#ifdef __AVX512F__
    /* Test each vector mode */
    test_v16simode(src1_i32, src2_i32, dst_i32, ARRAY_SIZE);
    test_v8dimode(src1_i64, src2_i64, dst_i64, ARRAY_SIZE);
    test_v8dfmode(src1_f64, src2_f64, dst_f64, ARRAY_SIZE);
    test_v16sfmode(src1_f32, src2_f32, dst_f32, ARRAY_SIZE);
    
#ifdef __AVX512BW__
    test_v64qimode(src1_u8, src2_u8, dst_u8, ARRAY_SIZE);
    test_v32himode(src1_i16, src2_i16, dst_i16, ARRAY_SIZE);
#endif
    
#ifdef __AVX512FP16__
    _Float16 src1_f16[ARRAY_SIZE];
    _Float16 src2_f16[ARRAY_SIZE];
    _Float16 dst_f16[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_f16[i] = i * 0.5f;
        src2_f16[i] = i * 0.75f;
    }
    test_v32hfmode(src1_f16, src2_f16, dst_f16, ARRAY_SIZE);
#endif
    
#ifdef __AVX512BF16__
    __bf16 src1_bf16[ARRAY_SIZE];
    __bf16 src2_bf16[ARRAY_SIZE];
    __bf16 dst_bf16[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1_bf16[i] = i * 0.25f;
        src2_bf16[i] = i * 0.5f;
    }
    test_v32bfmode(src1_bf16, src2_bf16, dst_bf16, ARRAY_SIZE);
#endif
    
    /* Compute checksums to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_u8[i] + dst_i16[i] + dst_i32[i] + dst_i64[i] +
                   (uint64_t)dst_f32[i] + (uint64_t)dst_f64[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("All AVX-512 blend tests completed.\n");
    
#else
    printf("AVX-512 not supported on this platform.\n");
#endif
    
    return 0;
}
