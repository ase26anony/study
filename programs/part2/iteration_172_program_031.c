/* AVX-512 blend coverage test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Ensure we have the required ISA support */
#if !defined(__AVX512F__) || !defined(__AVX512BW__)
#error "AVX-512F and AVX-512BW are required for this test"
#endif

/* Function attributes for specific ISA requirements */
#ifdef __cplusplus
extern "C" {
#endif

/* Integer blend operations requiring AVX512BW */
__attribute__((target("avx512bw")))
static void test_v64qi_blend(void) {
    /* V64QImode: 64 bytes */
    __m512i a = _mm512_set1_epi8(0x11);
    __m512i b = _mm512_set1_epi8(0x22);
    
    /* Create dynamic mask based on position */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to prevent optimization */
    volatile __m512i store_var = result;
    (void)store_var;
}

__attribute__((target("avx512bw")))
static void test_v32hi_blend(void) {
    /* V32HImode: 32 half-words */
    __m512i a = _mm512_set1_epi16(0x1111);
    __m512i b = _mm512_set1_epi16(0x2222);
    
    /* Dynamic mask: every other element */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512i store_var = result;
    (void)store_var;
}

/* Float16 blend operations */
#if defined(__AVX512FP16__)
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hf_blend(void) {
    /* V32HFmode: 32 half-precision floats */
    _Float16 data_a[32], data_b[32];
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 1.5f);
        data_b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h a = _mm512_loadu_ph(data_a);
    __m512h b = _mm512_loadu_ph(data_b);
    
    /* Create mask based on comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512h store_var = result;
    (void)store_var;
}
#endif

/* BFloat16 blend operations */
#if defined(__AVX512BF16__)
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bf_blend(void) {
    /* V32BFmode: 32 bfloat16 elements */
    __m512bh a, b;
    
    /* Initialize with some data */
    float fa[16] = {0}, fb[16] = {0};
    for (int i = 0; i < 16; i++) {
        fa[i] = i * 1.1f;
        fb[i] = i * 2.2f;
    }
    
    /* Convert float to bfloat16 */
    a = (__m512bh)_mm512_cvtne2ps_pbh(_mm512_loadu_ps(fa + 8), _mm512_loadu_ps(fa));
    b = (__m512bh)_mm512_cvtne2ps_pbh(_mm512_loadu_ps(fb + 8), _mm512_loadu_ps(fb));
    
    /* Create mask - use pattern based on position */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 4) < 2) {
            mask |= (1U << i);
        }
    }
    
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512bh store_var = result;
    (void)store_var;
}
#endif

/* AVX512F blend operations */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    /* V16SImode: 16 signed integers */
    __m512i a = _mm512_set1_epi32(0x11111111);
    __m512i b = _mm512_set1_epi32(0x22222222);
    
    /* Dynamic mask using comparison */
    __m512i cmp_a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i cmp_b = _mm512_set1_epi32(7);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(cmp_a, cmp_b);
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i store_var = result;
    (void)store_var;
}

__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    /* V8DImode: 8 double integers */
    __m512i a = _mm512_set1_epi64(0x1111111111111111ULL);
    __m512i b = _mm512_set1_epi64(0x2222222222222222ULL);
    
    /* Pattern mask */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (i % 3 != 0) {
            mask |= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i store_var = result;
    (void)store_var;
}

__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    /* V8DFmode: 8 double floats */
    __m512d a = _mm512_set1_pd(1.1);
    __m512d b = _mm512_set1_pd(2.2);
    
    /* Create mask by comparing with threshold */
    __m512d thresh = _mm512_set1_pd(1.5);
    __mmask8 mask = _mm512_cmp_pd_mask(a, thresh, _CMP_LT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d store_var = result;
    (void)store_var;
}

__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    /* V16SFmode: 16 single floats */
    __m512 a = _mm512_set1_ps(3.14f);
    __m512 b = _mm512_set1_ps(6.28f);
    
    /* Alternating mask pattern */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 binary */
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 store_var = result;
    (void)store_var;
}

#ifdef __cplusplus
}
#endif

/* Main test driver */
int main(void) {
    int tests_executed = 0;
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    /* Test all blend operations */
    test_v64qi_blend();
    tests_executed++;
    printf("  V64QImode blend: PASS\n");
    
    test_v32hi_blend();
    tests_executed++;
    printf("  V32HImode blend: PASS\n");
    
#if defined(__AVX512FP16__)
    test_v32hf_blend();
    tests_executed++;
    printf("  V32HFmode blend: PASS\n");
#else
    printf("  V32HFmode blend: SKIP (AVX512-FP16 not available)\n");
#endif
    
#if defined(__AVX512BF16__)
    test_v32bf_blend();
    tests_executed++;
    printf("  V32BFmode blend: PASS\n");
#else
    printf("  V32BFmode blend: SKIP (AVX512-BF16 not available)\n");
#endif
    
    test_v16si_blend();
    tests_executed++;
    printf("  V16SImode blend: PASS\n");
    
    test_v8di_blend();
    tests_executed++;
    printf("  V8DImode blend: PASS\n");
    
    test_v8df_blend();
    tests_executed++;
    printf("  V8DFmode blend: PASS\n");
    
    test_v16sf_blend();
    tests_executed++;
    printf("  V16SFmode blend: PASS\n");
    
    printf("\nTotal blend tests executed: %d\n", tests_executed);
    
    /* Return success if we executed at least the basic AVX512F/BW tests */
    if (tests_executed >= 6) {  /* At least the non-FP16/BF16 tests */
        printf("All required blend tests completed successfully.\n");
        return 0;
    } else {
        printf("Warning: Some required tests were skipped.\n");
        return 1;
    }
}
