/* AVX-512 Blend Instruction Coverage Test
 * Targets specific uncovered lines in i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512BW__
void test_v64qi_blend(void);
void test_v32hi_blend(void);
#endif

#ifdef __AVX512FP16__
void test_v32hf_blend(void);
#endif

#ifdef __AVX512BF16__
void test_v32bf_blend(void);
#endif

#ifdef __AVX512F__
void test_v16si_blend(void);
void test_v8di_blend(void);
void test_v8df_blend(void);
void test_v16sf_blend(void);
#endif

/* Helper function to generate dynamic masks */
static inline __mmask64 generate_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((seed + i) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static inline __mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((seed + i) % 2 == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static inline __mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((seed + i) % 3 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static inline __mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((seed + i) % 2 == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512bw")))
void test_v64qi_blend(void) {
    uint8_t src1[64] __attribute__((aligned(64)));
    uint8_t src2[64] __attribute__((aligned(64)));
    uint8_t dst[64] __attribute__((aligned(64)));
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 + i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Generate dynamic mask based on array content */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (src1[i] % 4 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Use result to prevent optimization */
    volatile uint8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512bw")))
void test_v32hi_blend(void) {
    int16_t src1[32] __attribute__((aligned(64)));
    int16_t src2[32] __attribute__((aligned(64)));
    int16_t dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Dynamic mask based on loop iteration */
    static int counter = 0;
    __mmask32 mask = generate_mask32(counter++);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    volatile int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512fp16,avx512bw")))
void test_v32hf_blend(void) {
    _Float16 src1[32] __attribute__((aligned(64)));
    _Float16 src2[32] __attribute__((aligned(64)));
    _Float16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 1.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Dynamic mask */
    __mmask32 mask = generate_mask32(42);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    
    volatile _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512bf16,avx512bw")))
void test_v32bf_blend(void) {
    __bf16 src1[32] __attribute__((aligned(64)));
    __bf16 src2[32] __attribute__((aligned(64)));
    __bf16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bf16)(i * 0.25f);
        src2[i] = (__bf16)(i * 0.75f);
    }
    
    /* Load as __m512bh for bfloat16 */
    __m512bh v1 = _mm512_load_si512((const __m512i*)src1);
    __m512bh v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Dynamic mask */
    __mmask32 mask = generate_mask32(123);
    
    /* Use mask blend intrinsic - note: _mm512_mask_blend_ph works on __m512h,
       but for bfloat16 we need to use the appropriate intrinsic */
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    _mm512_store_si512((__m512i*)dst, (__m512i)result);
    
    volatile uint16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += ((uint16_t*)dst)[i];
    }
}
#endif /* __AVX512BF16__ */

#ifdef __AVX512F__
/* V16SImode: 16 single-precision integer blend */
__attribute__((target("avx512f")))
void test_v16si_blend(void) {
    int32_t src1[16] __attribute__((aligned(64)));
    int32_t src2[16] __attribute__((aligned(64)));
    int32_t dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 20;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask by comparing vectors */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, v2);
    /* Make it non-constant by ORing with dynamic value */
    mask |= generate_mask16(7);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    volatile int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
void test_v8di_blend(void) {
    int64_t src1[8] __attribute__((aligned(64)));
    int64_t src2[8] __attribute__((aligned(64)));
    int64_t dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 200LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    __mmask8 mask = generate_mask8(13);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    volatile int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
void test_v8df_blend(void) {
    double src1[8] __attribute__((aligned(64)));
    double src2[8] __attribute__((aligned(64)));
    double dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask by comparing with threshold */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(4.0), _CMP_LT_OQ);
    mask ^= generate_mask8(5); /* Make it dynamic */
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    
    volatile double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
void test_v16sf_blend(void) {
    float src1[16] __attribute__((aligned(64)));
    float src2[16] __attribute__((aligned(64)));
    float dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Dynamic mask based on array values */
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (src1[i] > 3.0f) {
            mask |= (1 << i);
        }
    }
    mask ^= generate_mask16(9); /* Add some randomness */
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    
    volatile float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
}
#endif /* __AVX512F__ */

int main(void) {
    int checksum = 0;
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode blend...\n");
    test_v64qi_blend();
    checksum += 1;
    
    printf("Testing V32HImode blend...\n");
    test_v32hi_blend();
    checksum += 2;
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode blend...\n");
    test_v32hf_blend();
    checksum += 4;
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode blend...\n");
    test_v32bf_blend();
    checksum += 8;
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode blend...\n");
    test_v16si_blend();
    checksum += 16;
    
    printf("Testing V8DImode blend...\n");
    test_v8di_blend();
    checksum += 32;
    
    printf("Testing V8DFmode blend...\n");
    test_v8df_blend();
    checksum += 64;
    
    printf("Testing V16SFmode blend...\n");
    test_v16sf_blend();
    checksum += 128;
#endif
    
    printf("All blend tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
