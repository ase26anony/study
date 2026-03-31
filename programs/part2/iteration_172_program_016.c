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

/* Helper function to generate dynamic masks based on data */
static inline __mmask64 generate_mask64(int seed) {
    return (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (seed * 0x5555555555555555ULL));
}

static inline __mmask32 generate_mask32(int seed) {
    return (__mmask32)(0xAAAAAAAA ^ (seed * 0x55555555));
}

static inline __mmask16 generate_mask16(int seed) {
    return (__mmask16)(0xAAAA ^ (seed * 0x5555));
}

static inline __mmask8 generate_mask8(int seed) {
    return (__mmask8)(0xAA ^ (seed * 0x55));
}

/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
void test_v64qimode_blend(uint8_t* src1, uint8_t* src2, uint8_t* dst, int iterations) {
#ifdef __AVX512BW__
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Generate dynamic mask based on iteration */
        __mmask64 mask = generate_mask64(i);
        
        /* This should trigger gen_avx512bw_blendmv64qi */
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Rotate data to prevent optimization */
        src1[0] ^= (uint8_t)i;
        src2[0] ^= (uint8_t)(i >> 8);
    }
#endif
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
void test_v32himode_blend(int16_t* src1, int16_t* src2, int16_t* dst, int iterations) {
#ifdef __AVX512BW__
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Generate dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* This should trigger gen_avx512bw_blendmv32hi */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Modify data to prevent optimization */
        src1[0] ^= (int16_t)i;
        src2[0] ^= (int16_t)(i >> 8);
    }
#endif
}

/* V32HFmode - 32 half-precision float blend (requires AVX512-FP16) */
__attribute__((target("avx512fp16,avx512bw")))
void test_v32hfmode_blend(_Float16* src1, _Float16* src2, _Float16* dst, int iterations) {
#ifdef __AVX512FP16__
    for (int i = 0; i < iterations; i++) {
        __m512h a = _mm512_loadu_ph(src1);
        __m512h b = _mm512_loadu_ph(src2);
        
        /* Generate dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* This should trigger gen_avx512bw_blendmv32hf */
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph(dst, result);
        
        /* Prevent optimization */
        src1[0] += (_Float16)(i & 0xFF);
        src2[0] += (_Float16)((i >> 8) & 0xFF);
    }
#endif
}

/* V32BFmode - 32 bfloat16 blend (requires AVX512-BF16) */
__attribute__((target("avx512bf16,avx512bw")))
void test_v32bfmode_blend(__bf16* src1, __bf16* src2, __bf16* dst, int iterations) {
#ifdef __AVX512BF16__
    for (int i = 0; i < iterations; i++) {
        /* Load bfloat16 data - use memcpy to avoid strict aliasing */
        __m512bh a, b;
        memcpy(&a, src1, 64);
        memcpy(&b, src2, 64);
        
        /* Generate dynamic mask */
        __mmask32 mask = generate_mask32(i);
        
        /* This should trigger gen_avx512bw_blendmv32bf */
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        memcpy(dst, &result, 64);
        
        /* Prevent optimization */
        uint16_t* src1_u16 = (uint16_t*)src1;
        uint16_t* src2_u16 = (uint16_t*)src2;
        src1_u16[0] ^= (uint16_t)i;
        src2_u16[0] ^= (uint16_t)(i >> 8);
    }
#endif
}

/* V16SImode - 32-bit integer blend */
__attribute__((target("avx512f")))
void test_v16simode_blend(int32_t* src1, int32_t* src2, int32_t* dst, int iterations) {
#ifdef __AVX512F__
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Generate dynamic mask using comparison */
        __mmask16 mask = _mm512_cmpneq_epi32_mask(a, b);
        mask ^= generate_mask16(i);  /* Make it data-dependent */
        
        /* This should trigger gen_avx512f_blendmv16si */
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Prevent optimization */
        src1[0] ^= i;
        src2[0] ^= (i << 1);
    }
#endif
}

/* V8DImode - 64-bit integer blend */
__attribute__((target("avx512f")))
void test_v8dimode_blend(int64_t* src1, int64_t* src2, int64_t* dst, int iterations) {
#ifdef __AVX512F__
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Generate dynamic mask */
        __mmask8 mask = generate_mask8(i);
        
        /* This should trigger gen_avx512f_blendmv8di */
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Prevent optimization */
        src1[0] ^= (int64_t)i;
        src2[0] ^= (int64_t)(i << 8);
    }
#endif
}

/* V8DFmode - double precision float blend */
__attribute__((target("avx512f")))
void test_v8dfmode_blend(double* src1, double* src2, double* dst, int iterations) {
#ifdef __AVX512F__
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_loadu_pd(src1);
        __m512d b = _mm512_loadu_pd(src2);
        
        /* Generate dynamic mask using comparison */
        __mmask8 mask = _mm512_cmpneq_pd_mask(a, b);
        mask ^= generate_mask8(i);
        
        /* This should trigger gen_avx512f_blendmv8df */
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        _mm512_storeu_pd(dst, result);
        
        /* Prevent optimization */
        src1[0] += (double)(i & 0xFF);
        src2[0] += (double)((i >> 8) & 0xFF);
    }
#endif
}

/* V16SFmode - single precision float blend */
__attribute__((target("avx512f")))
void test_v16sfmode_blend(float* src1, float* src2, float* dst, int iterations) {
#ifdef __AVX512F__
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_loadu_ps(src1);
        __m512 b = _mm512_loadu_ps(src2);
        
        /* Generate dynamic mask using comparison */
        __mmask16 mask = _mm512_cmpneq_ps_mask(a, b);
        mask ^= generate_mask16(i);
        
        /* This should trigger gen_avx512f_blendmv16sf */
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        _mm512_storeu_ps(dst, result);
        
        /* Prevent optimization */
        src1[0] += (float)(i & 0xFF);
        src2[0] += (float)((i >> 8) & 0xFF);
    }
#endif
}

int main() {
    const int ITERATIONS = 100;
    const int ALIGN = 64;
    
    /* Allocate aligned memory for all test data */
    uint8_t* src1_qi = __builtin_aligned_alloc(ALIGN, 64);
    uint8_t* src2_qi = __builtin_aligned_alloc(ALIGN, 64);
    uint8_t* dst_qi = __builtin_aligned_alloc(ALIGN, 64);
    
    int16_t* src1_hi = __builtin_aligned_alloc(ALIGN, 64);
    int16_t* src2_hi = __builtin_aligned_alloc(ALIGN, 64);
    int16_t* dst_hi = __builtin_aligned_alloc(ALIGN, 64);
    
    _Float16* src1_hf = __builtin_aligned_alloc(ALIGN, 64);
    _Float16* src2_hf = __builtin_aligned_alloc(ALIGN, 64);
    _Float16* dst_hf = __builtin_aligned_alloc(ALIGN, 64);
    
    __bf16* src1_bf = __builtin_aligned_alloc(ALIGN, 64);
    __bf16* src2_bf = __builtin_aligned_alloc(ALIGN, 64);
    __bf16* dst_bf = __builtin_aligned_alloc(ALIGN, 64);
    
    int32_t* src1_si = __builtin_aligned_alloc(ALIGN, 64);
    int32_t* src2_si = __builtin_aligned_alloc(ALIGN, 64);
    int32_t* dst_si = __builtin_aligned_alloc(ALIGN, 64);
    
    int64_t* src1_di = __builtin_aligned_alloc(ALIGN, 64);
    int64_t* src2_di = __builtin_aligned_alloc(ALIGN, 64);
    int64_t* dst_di = __builtin_aligned_alloc(ALIGN, 64);
    
    double* src1_df = __builtin_aligned_alloc(ALIGN, 64);
    double* src2_df = __builtin_aligned_alloc(ALIGN, 64);
    double* dst_df = __builtin_aligned_alloc(ALIGN, 64);
    
    float* src1_sf = __builtin_aligned_alloc(ALIGN, 64);
    float* src2_sf = __builtin_aligned_alloc(ALIGN, 64);
    float* dst_sf = __builtin_aligned_alloc(ALIGN, 64);
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) src1_qi[i] = (uint8_t)(i * 3);
    for (int i = 0; i < 64; i++) src2_qi[i] = (uint8_t)(i * 5);
    
    for (int i = 0; i < 32; i++) src1_hi[i] = (int16_t)(i * 7);
    for (int i = 0; i < 32; i++) src2_hi[i] = (int16_t)(i * 11);
    
    for (int i = 0; i < 32; i++) src1_hf[i] = (_Float16)(i * 1.5f);
    for (int i = 0; i < 32; i++) src2_hf[i] = (_Float16)(i * 2.5f);
    
    for (int i = 0; i < 32; i++) {
        uint16_t* p1 = (uint16_t*)src1_bf;
        uint16_t* p2 = (uint16_t*)src2_bf;
        p1[i] = (uint16_t)(i * 13);
        p2[i] = (uint16_t)(i * 17);
    }
    
    for (int i = 0; i < 16; i++) src1_si[i] = i * 19;
    for (int i = 0; i < 16; i++) src2_si[i] = i * 23;
    
    for (int i = 0; i < 8; i++) src1_di[i] = (int64_t)i * 29;
    for (int i = 0; i < 8; i++) src2_di[i] = (int64_t)i * 31;
    
    for (int i = 0; i < 8; i++) src1_df[i] = (double)i * 1.7;
    for (int i = 0; i < 8; i++) src2_df[i] = (double)i * 2.3;
    
    for (int i = 0; i < 16; i++) src1_sf[i] = (float)i * 1.1f;
    for (int i = 0; i < 16; i++) src2_sf[i] = (float)i * 1.9f;
    
    /* Execute all blend tests */
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    test_v64qimode_blend(src1_qi, src2_qi, dst_qi, ITERATIONS);
    printf("  V64QImode blend test completed\n");
    
    test_v32himode_blend(src1_hi, src2_hi, dst_hi, ITERATIONS);
    printf("  V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    test_v32hfmode_blend(src1_hf, src2_hf, dst_hf, ITERATIONS);
    printf("  V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bfmode_blend(src1_bf, src2_bf, dst_bf, ITERATIONS);
    printf("  V32BFmode blend test completed\n");
#endif
    
    test_v16simode_blend(src1_si, src2_si, dst_si, ITERATIONS);
    printf("  V16SImode blend test completed\n");
    
    test_v8dimode_blend(src1_di, src2_di, dst_di, ITERATIONS);
    printf("  V8DImode blend test completed\n");
    
    test_v8dfmode_blend(src1_df, src2_df, dst_df, ITERATIONS);
    printf("  V8DFmode blend test completed\n");
    
    test_v16sfmode_blend(src1_sf, src2_sf, dst_sf, ITERATIONS);
    printf("  V16SFmode blend test completed\n");
    
    /* Compute checksum to ensure all blends were executed */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum += dst_qi[i];
    for (int i = 0; i < 32; i++) checksum += dst_hi[i];
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) checksum += (uint16_t)dst_hf[i];
#endif
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) checksum += ((uint16_t*)dst_bf)[i];
#endif
    for (int i = 0; i < 16; i++) checksum += dst_si[i];
    for (int i = 0; i < 8; i++) checksum += dst_di[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)dst_df[i];
    for (int i = 0; i < 16; i++) checksum += (uint32_t)dst_sf[i];
    
    printf("Final checksum: %lu\n", checksum);
    
    /* Cleanup */
    __builtin_free(src1_qi);
    __builtin_free(src2_qi);
    __builtin_free(dst_qi);
    __builtin_free(src1_hi);
    __builtin_free(src2_hi);
    __builtin_free(dst_hi);
    __builtin_free(src1_hf);
    __builtin_free(src2_hf);
    __builtin_free(dst_hf);
    __builtin_free(src1_bf);
    __builtin_free(src2_bf);
    __builtin_free(dst_bf);
    __builtin_free(src1_si);
    __builtin_free(src2_si);
    __builtin_free(dst_si);
    __builtin_free(src1_di);
    __builtin_free(src2_di);
    __builtin_free(dst_di);
    __builtin_free(src1_df);
    __builtin_free(src2_df);
    __builtin_free(dst_df);
    __builtin_free(src1_sf);
    __builtin_free(src2_sf);
    __builtin_free(dst_sf);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
