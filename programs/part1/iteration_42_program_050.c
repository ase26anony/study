#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    __m512i a, b, result;
    __mmask64 mask;
    char data_a[64] __attribute__((aligned(64)));
    char data_b[64] __attribute__((aligned(64)));
    char data_result[64] __attribute__((aligned(64)));
    
    /* Initialize with pattern data */
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3 + 1);
    }
    
    a = _mm512_load_si512((const __m512i*)data_a);
    b = _mm512_load_si512((const __m512i*)data_b);
    
    /* Generate dynamic mask using comparison */
    mask = _mm512_cmp_epi8_mask(a, b, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)data_result, result);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data_result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    __m512i a, b, result;
    __mmask32 mask;
    short data_a[32] __attribute__((aligned(64)));
    short data_b[32] __attribute__((aligned(64)));
    short data_result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15 + 5);
    }
    
    a = _mm512_load_si512((const __m512i*)data_a);
    b = _mm512_load_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += data_result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static int test_v32hf_blend(void) {
    _Float16 data_a[32] __attribute__((aligned(64)));
    _Float16 data_b[32] __attribute__((aligned(64)));
    _Float16 data_result[32] __attribute__((aligned(64)));
    __m512h a, b, result;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 1.5f);
        data_b[i] = (_Float16)(i * 2.0f + 0.5f);
    }
    
    a = _mm512_load_ph(data_a);
    b = _mm512_load_ph(data_b);
    
    /* Generate mask using comparison */
    mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)data_result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    __m512bh a, b, result;
    __mmask32 mask;
    __bfloat16 data_a[32] __attribute__((aligned(64)));
    __bfloat16 data_b[32] __attribute__((aligned(64)));
    __bfloat16 data_result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = bfloat16_from_float((float)i * 1.2f);
        data_b[i] = bfloat16_from_float((float)i * 1.8f + 0.3f);
    }
    
    a = _mm512_load_si512((const __m512i*)data_a);
    b = _mm512_load_si512((const __m512i*)data_b);
    
    /* For bfloat16, we need to use integer blend since there's no direct bfloat16 blend */
    mask = _mm512_cmp_epi16_mask((__m512i)a, (__m512i)b, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
    _mm512_store_si512((__m512i*)data_result, (__m512i)result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_uint32(data_result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    __m512i a, b, result;
    __mmask16 mask;
    int data_a[16] __attribute__((aligned(64)));
    int data_b[16] __attribute__((aligned(64)));
    int data_result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 100;
        data_b[i] = i * 150 + 50;
    }
    
    a = _mm512_load_si512((const __m512i*)data_a);
    b = _mm512_load_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512f_blendmv16si */
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += data_result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static int test_v8di_blend(void) {
    __m512i a, b, result;
    __mmask8 mask;
    long long data_a[8] __attribute__((aligned(64)));
    long long data_b[8] __attribute__((aligned(64)));
    long long data_result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (long long)i * 1000LL;
        data_b[i] = (long long)i * 1500LL + 500LL;
    }
    
    a = _mm512_load_si512((const __m512i*)data_a);
    b = _mm512_load_si512((const __m512i*)data_b);
    
    mask = _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_LE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)(data_result[i] & 0xFFFFFFFF);
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static int test_v8df_blend(void) {
    __m512d a, b, result;
    __mmask8 mask;
    double data_a[8] __attribute__((aligned(64)));
    double data_b[8] __attribute__((aligned(64)));
    double data_result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        data_a[i] = (double)i * 1.1;
        data_b[i] = (double)i * 1.8 + 0.3;
    }
    
    a = _mm512_load_pd(data_a);
    b = _mm512_load_pd(data_b);
    
    mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)data_result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static int test_v16sf_blend(void) {
    __m512 a, b, result;
    __mmask16 mask;
    float data_a[16] __attribute__((aligned(64)));
    float data_b[16] __attribute__((aligned(64)));
    float data_result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        data_a[i] = (float)i * 0.5f;
        data_b[i] = (float)i * 0.8f + 0.2f;
    }
    
    a = _mm512_load_ps(data_a);
    b = _mm512_load_ps(data_b);
    
    mask = _mm512_cmp_ps_mask(a, b, _CMP_GE_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(data_result, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (int)data_result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    int total_sum = 0;
    
    /* Test different data types in a loop */
    for (int iter = 0; iter < 4; iter++) {
        if (iter == 0) {
            /* V16SFmode */
            float fa[16], fb[16], fr[16];
            for (int i = 0; i < 16; i++) {
                fa[i] = (float)(i + iter) * 0.3f;
                fb[i] = (float)(i + iter) * 0.7f;
            }
            __m512 a = _mm512_load_ps(fa);
            __m512 b = _mm512_load_ps(fb);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, a, b);
            _mm512_store_ps(fr, result);
            for (int i = 0; i < 16; i++) total_sum += (int)fr[i];
        }
        else if (iter == 1) {
            /* V8DFmode */
            double da[8], db[8], dr[8];
            for (int i = 0; i < 8; i++) {
                da[i] = (double)(i + iter) * 0.4;
                db[i] = (double)(i + iter) * 0.9;
            }
            __m512d a = _mm512_load_pd(da);
            __m512d b = _mm512_load_pd(db);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, a, b);
            _mm512_store_pd(dr, result);
            for (int i = 0; i < 8; i++) total_sum += (int)dr[i];
        }
        else if (iter == 2) {
            /* V16SImode */
            int ia[16], ib[16], ir[16];
            for (int i = 0; i < 16; i++) {
                ia[i] = (i + iter) * 10;
                ib[i] = (i + iter) * 20;
            }
            __m512i a = _mm512_load_si512((const __m512i*)ia);
            __m512i b = _mm512_load_si512((const __m512i*)ib);
            __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, a, b);
            _mm512_store_si512((__m512i*)ir, result);
            for (int i = 0; i < 16; i++) total_sum += ir[i];
        }
        else if (iter == 3) {
            /* V32HImode */
            short sa[32], sb[32], sr[32];
            for (int i = 0; i < 32; i++) {
                sa[i] = (short)((i + iter) * 5);
                sb[i] = (short)((i + iter) * 8);
            }
            __m512i a = _mm512_load_si512((const __m512i*)sa);
            __m512i b = _mm512_load_si512((const __m512i*)sb);
            __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi16(mask, a, b);
            _mm512_store_si512((__m512i*)sr, result);
            for (int i = 0; i < 32; i++) total_sum += sr[i];
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallback Implementations ==================== */
static int scalar_test_v64qi_blend(void) {
    char data_a[64];
    char data_b[64];
    char data_result[64];
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(i * 2);
        data_b[i] = (char)(i * 3 + 1);
        data_result[i] = (data_a[i] > data_b[i]) ? data_a[i] : data_b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data_result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short data_a[32];
    short data_b[32];
    short data_result[32];
    
    for (int i = 0; i < 32; i++) {
        data_a[i] = (short)(i * 10);
        data_b[i] = (short)(i * 15 + 5);
        data_result[i] = (data_a[i] < data_b[i]) ? data_a[i] : data_b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += data_result[i];
    }
    return sum;
}

/* ==================== Main Driver Function ==================== */
int main(void) {
    int total_checksum = 0;
    
    printf("AVX-512 Blend Instruction Coverage Test\n");
    printf("=======================================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW support detected. Using intrinsics.\n");
    
    /* Run all vector mode tests */
    total_checksum += test_v64qi_blend();
    printf("  V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("  V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("  V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("  V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("  V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("  V8DImode test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("  V8DFmode test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("  V16SFmode test completed\n");
    
    total_checksum += test_mixed_blend_loop();
    printf("  Mixed data type loop test completed\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallback.\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
#else
    printf("AVX-512F not detected. Using scalar fallback.\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    /* Return checksum modulo 256 to ensure program has observable output */
    return total_checksum & 0xFF;
}
