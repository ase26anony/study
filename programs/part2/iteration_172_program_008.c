/* AVX-512 blend coverage test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Define fallbacks if compiling without AVX-512 support */
#ifndef __AVX512F__
#warning "AVX-512F not enabled - coverage may be incomplete"
#endif

#ifndef __AVX512BW__
#warning "AVX512BW not enabled - coverage may be incomplete"
#endif

#ifndef __AVX512FP16__
#warning "AVX512-FP16 not enabled - coverage may be incomplete"
#endif

#ifndef __AVX512BF16__
#warning "AVX512-BF16 not enabled - coverage may be incomplete"
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
void test_v16si_v8di_v8df_v16sf(void);
#endif

#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
void test_v64qi_v32hi(void);
#endif

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
void test_v32hf(void);
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
void test_v32bf(void);
#endif

/* Global arrays to prevent optimization */
static uint64_t g_checksum = 0;

/* Helper to generate dynamic masks */
static inline __mmask64 generate_mask64(int iter) {
    /* Create a pattern that changes with iteration */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i ^ iter) & 1) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static inline __mmask32 generate_mask32(int iter) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i ^ iter) & 1) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static inline __mmask16 generate_mask16(int iter) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i ^ iter) & 1) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static inline __mmask8 generate_mask8(int iter) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i ^ iter) & 1) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
void test_v64qi_v32hi(void) {
    /* Test V64QImode - 64 bytes */
    int8_t data1[64] __attribute__((aligned(64)));
    int8_t data2[64] __attribute__((aligned(64)));
    int8_t result[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        data1[i] = i;
        data2[i] = 64 - i;
    }
    
    /* Use multiple iterations with different masks */
    for (int iter = 0; iter < 4; iter++) {
        __m512i vec1 = _mm512_load_si512((const __m512i*)data1);
        __m512i vec2 = _mm512_load_si512((const __m512i*)data2);
        __mmask64 mask = generate_mask64(iter);
        
        /* This should trigger gen_avx512bw_blendmv64qi */
        __m512i blended = _mm512_mask_blend_epi8(mask, vec1, vec2);
        _mm512_store_si512((__m512i*)result, blended);
        
        /* Update checksum */
        for (int i = 0; i < 64; i++) {
            g_checksum += result[i];
        }
    }
    
    /* Test V32HImode - 32 half-words */
    int16_t data1_hi[32] __attribute__((aligned(64)));
    int16_t data2_hi[32] __attribute__((aligned(64)));
    int16_t result_hi[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data1_hi[i] = i * 2;
        data2_hi[i] = 1000 - i * 3;
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512i vec1 = _mm512_load_si512((const __m512i*)data1_hi);
        __m512i vec2 = _mm512_load_si512((const __m512i*)data2_hi);
        __mmask32 mask = generate_mask32(iter);
        
        /* This should trigger gen_avx512bw_blendmv32hi */
        __m512i blended = _mm512_mask_blend_epi16(mask, vec1, vec2);
        _mm512_store_si512((__m512i*)result_hi, blended);
        
        for (int i = 0; i < 32; i++) {
            g_checksum += result_hi[i];
        }
    }
}
#endif

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
void test_v32hf(void) {
    /* Test V32HFmode - 32 half-precision floats */
    _Float16 data1[32] __attribute__((aligned(64)));
    _Float16 data2[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data1[i] = (_Float16)(i * 1.5f);
        data2[i] = (_Float16)(100.0f - i * 2.0f);
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512h vec1 = _mm512_load_ph(data1);
        __m512h vec2 = _mm512_load_ph(data2);
        __mmask32 mask = generate_mask32(iter);
        
        /* This should trigger gen_avx512bw_blendmv32hf */
        __m512h blended = _mm512_mask_blend_ph(mask, vec1, vec2);
        _mm512_store_ph(result, blended);
        
        for (int i = 0; i < 32; i++) {
            g_checksum += (uint64_t)(result[i] * 1000);
        }
    }
}
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
void test_v32bf(void) {
    /* Test V32BFmode - 32 bfloat16 values */
    /* Note: Use __m512bh for bfloat16 vectors */
    __m512bh vec1, vec2, blended;
    uint16_t data1[32] __attribute__((aligned(64)));
    uint16_t data2[32] __attribute__((aligned(64)));
    uint16_t result[32] __attribute__((aligned(64)));
    
    /* Initialize with bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern */
        data1[i] = (i << 8) | 0x3F;  /* ~1.0 in bfloat16 */
        data2[i] = (i << 8) | 0x40;  /* ~2.0 in bfloat16 */
    }
    
    for (int iter = 0; iter < 4; iter++) {
        vec1 = _mm512_load_si512((const __m512i*)data1);
        vec2 = _mm512_load_si512((const __m512i*)data2);
        __mmask32 mask = generate_mask32(iter);
        
        /* This should trigger gen_avx512bw_blendmv32bf */
        blended = _mm512_mask_blend_ph(mask, vec1, vec2);
        _mm512_store_si512((__m512i*)result, (__m512i)blended);
        
        for (int i = 0; i < 32; i++) {
            g_checksum += result[i];
        }
    }
}
#endif

#ifdef __AVX512F__
__attribute__((target("avx512f")))
void test_v16si_v8di_v8df_v16sf(void) {
    /* Test V16SImode - 16 integers */
    int32_t data1_si[16] __attribute__((aligned(64)));
    int32_t data2_si[16] __attribute__((aligned(64)));
    int32_t result_si[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data1_si[i] = i * 10;
        data2_si[i] = 1000 - i * 20;
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512i vec1 = _mm512_load_si512((const __m512i*)data1_si);
        __m512i vec2 = _mm512_load_si512((const __m512i*)data2_si);
        __mmask16 mask = generate_mask16(iter);
        
        /* This should trigger gen_avx512f_blendmv16si */
        __m512i blended = _mm512_mask_blend_epi32(mask, vec1, vec2);
        _mm512_store_si512((__m512i*)result_si, blended);
        
        for (int i = 0; i < 16; i++) {
            g_checksum += result_si[i];
        }
    }
    
    /* Test V8DImode - 8 double integers */
    int64_t data1_di[8] __attribute__((aligned(64)));
    int64_t data2_di[8] __attribute__((aligned(64)));
    int64_t result_di[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data1_di[i] = i * 100LL;
        data2_di[i] = 10000LL - i * 200LL;
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512i vec1 = _mm512_load_si512((const __m512i*)data1_di);
        __m512i vec2 = _mm512_load_si512((const __m512i*)data2_di);
        __mmask8 mask = generate_mask8(iter);
        
        /* This should trigger gen_avx512f_blendmv8di */
        __m512i blended = _mm512_mask_blend_epi64(mask, vec1, vec2);
        _mm512_store_si512((__m512i*)result_di, blended);
        
        for (int i = 0; i < 8; i++) {
            g_checksum += result_di[i];
        }
    }
    
    /* Test V8DFmode - 8 doubles */
    double data1_df[8] __attribute__((aligned(64)));
    double data2_df[8] __attribute__((aligned(64)));
    double result_df[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data1_df[i] = i * 1.5;
        data2_df[i] = 100.0 - i * 2.5;
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512d vec1 = _mm512_load_pd(data1_df);
        __m512d vec2 = _mm512_load_pd(data2_df);
        __mmask8 mask = generate_mask8(iter);
        
        /* This should trigger gen_avx512f_blendmv8df */
        __m512d blended = _mm512_mask_blend_pd(mask, vec1, vec2);
        _mm512_store_pd(result_df, blended);
        
        for (int i = 0; i < 8; i++) {
            g_checksum += (uint64_t)(result_df[i] * 1000);
        }
    }
    
    /* Test V16SFmode - 16 floats */
    float data1_sf[16] __attribute__((aligned(64)));
    float data2_sf[16] __attribute__((aligned(64)));
    float result_sf[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data1_sf[i] = i * 0.5f;
        data2_sf[i] = 50.0f - i * 1.5f;
    }
    
    for (int iter = 0; iter < 4; iter++) {
        __m512 vec1 = _mm512_load_ps(data1_sf);
        __m512 vec2 = _mm512_load_ps(data2_sf);
        __mmask16 mask = generate_mask16(iter);
        
        /* This should trigger gen_avx512f_blendmv16sf */
        __m512 blended = _mm512_mask_blend_ps(mask, vec1, vec2);
        _mm512_store_ps(result_sf, blended);
        
        for (int i = 0; i < 16; i++) {
            g_checksum += (uint64_t)(result_sf[i] * 1000);
        }
    }
}
#endif

int main(void) {
    printf("Starting AVX-512 blend coverage test...\n");
    
#ifdef __AVX512F__
    test_v16si_v8di_v8df_v16sf();
    printf("AVX512F blend tests completed\n");
#endif
    
#ifdef __AVX512BW__
    test_v64qi_v32hi();
    printf("AVX512BW blend tests completed\n");
#endif
    
#ifdef __AVX512FP16__
    test_v32hf();
    printf("AVX512-FP16 blend tests completed\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bf();
    printf("AVX512-BF16 blend tests completed\n");
#endif
    
    printf("Final checksum: %lu\n", g_checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
