/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Enable all required AVX-512 extensions */
#ifdef __AVX512F__
#ifdef __AVX512BW__
#ifdef __AVX512FP16__
#ifdef __AVX512BF16__

/* Function prototypes with target attributes */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void);

__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void);

__attribute__((target("avx512f,avx512bw,avx512fp16")))
static void test_v32hf_blend(void);

__attribute__((target("avx512f,avx512bw,avx512bf16")))
static void test_v32bf_blend(void);

__attribute__((target("avx512f")))
static void test_v16si_blend(void);

__attribute__((target("avx512f")))
static void test_v8di_blend(void);

__attribute__((target("avx512f")))
static void test_v8df_blend(void);

__attribute__((target("avx512f")))
static void test_v16sf_blend(void);

/* Global checksum to prevent optimization */
volatile uint64_t global_checksum = 0;

/* Test V64QImode - 64-byte integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void) {
    uint8_t data_a[64] __attribute__((aligned(64)));
    uint8_t data_b[64] __attribute__((aligned(64)));
    uint8_t result[64] __attribute__((aligned(64)));
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;
        data_b[i] = 64 - i;
    }
    
    /* Load into registers */
    __m512i vec_a = _mm512_load_si512(data_a);
    __m512i vec_b = _mm512_load_si512(data_b);
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(vec_a, _mm512_set1_epi8(1)), 
                                           _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
    
    /* Store result */
    _mm512_store_si512(result, blended);
    
    /* Update checksum */
    for (int i = 0; i < 64; i++) {
        global_checksum += result[i];
    }
}

/* Test V32HImode - 32 half-word integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void) {
    uint16_t data_a[32] __attribute__((aligned(64)));
    uint16_t data_b[32] __attribute__((aligned(64)));
    uint16_t result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = i * 2;
        data_b[i] = i * 3;
    }
    
    __m512i vec_a = _mm512_load_si512(data_a);
    __m512i vec_b = _mm512_load_si512(data_b);
    
    /* Dynamic mask using comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(vec_a, _mm512_set1_epi16(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
    
    _mm512_store_si512(result, blended);
    
    for (int i = 0; i < 32; i++) {
        global_checksum += result[i];
    }
}

/* Test V32HFmode - 32 half-precision float blend (requires AVX512-FP16) */
__attribute__((target("avx512f,avx512bw,avx512fp16")))
static void test_v32hf_blend(void) {
    _Float16 data_a[32] __attribute__((aligned(64)));
    _Float16 data_b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 0.5f);
        data_b[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h vec_a = _mm512_load_ph(data_a);
    __m512h vec_b = _mm512_load_ph(data_b);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(vec_a, vec_b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, vec_a, vec_b);
    
    _mm512_store_ph(result, blended);
    
    for (int i = 0; i < 32; i++) {
        global_checksum += (uint16_t)result[i];
    }
}

/* Test V32BFmode - 32 bfloat16 blend (requires AVX512-BF16) */
__attribute__((target("avx512f,avx512bw,avx512bf16")))
static void test_v32bf_blend(void) {
    __bf16 data_a[32] __attribute__((aligned(64)));
    __bf16 data_b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        /* Simple pattern for bfloat16 */
        uint16_t val = i * 0x40;
        data_a[i] = (__bf16)val;
        data_b[i] = (__bf16)(val + 0x20);
    }
    
    /* Load bfloat16 data */
    __m512bh vec_a = _mm512_load_si512(data_a);
    __m512bh vec_b = _mm512_load_si512(data_b);
    
    /* Convert to float for mask generation */
    __m512 vec_a_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(vec_a));
    __m512 vec_b_f = _mm512_cvtpbh_ps(_mm512_castsi512_si256(vec_b));
    
    __mmask16 mask_lo = _mm512_cmp_ps_mask(vec_a_f, vec_b_f, _CMP_LT_OQ);
    __mmask32 mask = _mm512_kunpackd(mask_lo, mask_lo);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh blended = _mm512_mask_blend_ph(mask, vec_a, vec_b);
    
    _mm512_store_si512(result, blended);
    
    for (int i = 0; i < 32; i++) {
        global_checksum += (uint16_t)result[i];
    }
}

/* Test V16SImode - 16 single integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    int32_t data_a[16] __attribute__((aligned(64)));
    int32_t data_b[16] __attribute__((aligned(64)));
    int32_t result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 10;
        data_b[i] = i * 15;
    }
    
    __m512i vec_a = _mm512_load_si512(data_a);
    __m512i vec_b = _mm512_load_si512(data_b);
    
    /* Dynamic mask */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_si512(vec_a, _mm512_set1_epi32(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
    
    _mm512_store_si512(result, blended);
    
    for (int i = 0; i < 16; i++) {
        global_checksum += result[i];
    }
}

/* Test V8DImode - 8 double integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    int64_t data_a[8] __attribute__((aligned(64)));
    int64_t data_b[8] __attribute__((aligned(64)));
    int64_t result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 100LL;
        data_b[i] = i * 150LL;
    }
    
    __m512i vec_a = _mm512_load_si512(data_a);
    __m512i vec_b = _mm512_load_si512(data_b);
    
    /* Dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_si512(vec_a, _mm512_set1_epi64(1)), 
                                           _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
    
    _mm512_store_si512(result, blended);
    
    for (int i = 0; i < 8; i++) {
        global_checksum += result[i];
    }
}

/* Test V8DFmode - 8 double precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    double data_a[8] __attribute__((aligned(64)));
    double data_b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 1.5;
        data_b[i] = i * 2.5;
    }
    
    __m512d vec_a = _mm512_load_pd(data_a);
    __m512d vec_b = _mm512_load_pd(data_b);
    
    /* Dynamic mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, vec_a, vec_b);
    
    _mm512_store_pd(result, blended);
    
    for (int i = 0; i < 8; i++) {
        global_checksum += (uint64_t)result[i];
    }
}

/* Test V16SFmode - 16 single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    float data_a[16] __attribute__((aligned(64)));
    float data_b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 0.25f;
        data_b[i] = i * 0.35f;
    }
    
    __m512 vec_a = _mm512_load_ps(data_a);
    __m512 vec_b = _mm512_load_ps(data_b);
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, vec_a, vec_b);
    
    _mm512_store_ps(result, blended);
    
    for (int i = 0; i < 16; i++) {
        global_checksum += (uint32_t)result[i];
    }
}

#endif /* __AVX512BF16__ */
#endif /* __AVX512FP16__ */
#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Main function */
int main(void) {
    printf("Starting AVX-512 blend coverage test...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    test_v64qi_blend();
    test_v32hi_blend();
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
#ifdef __AVX512FP16__
    test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    test_v32bf_blend();
#endif
    
    printf("Global checksum: %lu\n", (unsigned long)global_checksum);
    printf("All AVX-512 blend tests completed.\n");
    
#else
    printf("AVX512BW not supported - skipping tests\n");
#endif
#else
    printf("AVX512F not supported - skipping tests\n");
#endif
    
    return 0;
}
