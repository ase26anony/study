/* test_avx512_blend.c - Coverage for i386-expand.cc blend patterns */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Alignment and volatile for preventing optimization */
#define ALIGN_64 __attribute__((aligned(64)))
#define FORCE_USE(x) __asm__ volatile("" : : "r"(x) : "memory")

/* Feature guards for modular compilation */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QI (64x int8) ========== */
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile int8_t* volatile_dst = dst;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(32);
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Use in loop with argc dependency */
    int sum = 0;
    for (int i = 0; i < (argc % 4 + 1); i++) {
        __m512i temp = _mm512_add_epi8(blended, _mm512_set1_epi8(i));
        blended = _mm512_mask_blend_epi8(mask, blended, temp);
        
        /* Store to volatile to prevent optimization */
        _mm512_store_si512((__m512i*)volatile_dst, blended);
        FORCE_USE(volatile_dst);
    }
    
    /* Reduction */
    _mm512_store_si512((__m512i*)dst, blended);
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ========== V32HI (32x int16) ========== */
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    volatile int16_t* volatile_dst = dst;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - should trigger gen_avx512bw_blendmv32hi */
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_GT);
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(argc);
    blended = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    /* Store and compute checksum */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ========== V32HF (32x half precision) ========== */
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* Store as uint16_t for half precision */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Initialize with simple pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 0x400;  /* 2.0 increments */
        src2[i] = i * 0x3C0;  /* 1.5 increments */
    }
    
    __m512h v1 = _mm512_load_ph((const void*)src1);
    __m512h v2 = _mm512_load_ph((const void*)src2);
    
    /* Create mask using comparison - should trigger gen_avx512bw_blendmv32hf */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512h added = _mm512_add_ph(v1, v2);
    blended = _mm512_mask_blend_ph(mask, blended, added);
    
    /* Store and compute integer checksum */
    _mm512_store_ph((void*)dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst[i];
    }
    
    return sum;
}

/* ========== V32BF (32x bfloat16) ========== */
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 stored as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (i << 8) | 0x3F;  /* Simple bfloat pattern */
        src2[i] = (i << 8) | 0x40;
    }
    
    /* Load as integers for bfloat16 operations */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - use integer comparison since bfloat16 lacks direct comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_LT);
    
    /* Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Additional blend with modified vector */
    __m512i shifted = _mm512_slli_epi16(blended, 1);
    blended = _mm512_mask_blend_epi16(mask, blended, shifted);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

#ifdef __AVX512F__

/* ========== V16SI (16x int32) ========== */
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile int32_t* volatile_dst = dst;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 15;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask - should trigger gen_avx512f_blendmv16si */
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_EQ);
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in loop with argc */
    for (int i = 0; i < (argc % 3 + 1); i++) {
        __m512i temp = _mm512_add_epi32(blended, _mm512_set1_epi32(i));
        blended = _mm512_mask_blend_epi32(mask, blended, temp);
        _mm512_store_epi32(volatile_dst, blended);
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ========== V8DI (8x int64) ========== */
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 150LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask - should trigger gen_avx512f_blendmv8di */
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, v2, _MM_CMPINT_LE);
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend with arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(2));
    blended = _mm512_mask_blend_epi64(mask, blended, multiplied);
    
    _mm512_store_epi64(dst, blended);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ========== V8DF (8x double) ========== */
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile double* volatile_dst = dst;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask - should trigger gen_avx512f_blendmv8df */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with computed value */
    __m512d sqrt_val = _mm512_sqrt_pd(v1);
    blended = _mm512_mask_blend_pd(mask, blended, sqrt_val);
    
    /* Force use through volatile store */
    _mm512_store_pd(volatile_dst, blended);
    FORCE_USE(volatile_dst);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ========== V16SF (16x float) ========== */
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile float* volatile_dst = dst;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask - should trigger gen_avx512f_blendmv16sf */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Multiple blends in sequence */
    __m512 multiplied = _mm512_mul_ps(v1, v2);
    blended = _mm512_mask_blend_ps(mask, blended, multiplied);
    
    __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(1.0f));
    blended = _mm512_mask_blend_ps(mask ^ 0xAAAA, blended, added);
    
    _mm512_store_ps(volatile_dst, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512F__ */

/* ========== Main Driver ========== */
int main(int argc, char* argv[]) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend pattern coverage...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX512F and AVX512BW detected, running all tests...\n");
    
    total_checksum += test_v64qi_blend(argc);
    total_checksum += test_v32hi_blend(argc);
    total_checksum += test_v32hf_blend(argc);
    total_checksum += test_v32bf_blend(argc);
    total_checksum += test_v16si_blend(argc);
    
    long long di_sum = test_v8di_blend(argc);
    total_checksum += (int)(di_sum & 0xFFFFFFFF) + (int)(di_sum >> 32);
    
    double df_sum = test_v8df_blend(argc);
    total_checksum += (int)df_sum;
    
    float sf_sum = test_v16sf_blend(argc);
    total_checksum += (int)sf_sum;
    
#else
    printf("AVX512F detected but not AVX512BW, running limited tests...\n");
    
    total_checksum += test_v16si_blend(argc);
    
    long long di_sum = test_v8di_blend(argc);
    total_checksum += (int)(di_sum & 0xFFFFFFFF) + (int)(di_sum >> 32);
    
    double df_sum = test_v8df_blend(argc);
    total_checksum += (int)df_sum;
    
    float sf_sum = test_v16sf_blend(argc);
    total_checksum += (int)sf_sum;
    
#endif
#else
    printf("AVX-512 not supported on this compiler/hardware.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use result to affect return code */
    return (total_checksum != 0) ? 0 : 1;
}
