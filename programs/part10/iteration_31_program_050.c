/* test_avx512_blend.c - AVX-512 blend intrinsics test for GCC backend coverage */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Alignment macros */
#define ALIGN_64 __attribute__((aligned(64)))
#define ALIGNED_LOAD_512(type, ptr) _mm512_load_si512((const __m512i*)(ptr))
#define ALIGNED_STORE_512(ptr, val) _mm512_store_si512((__m512i*)(ptr), (val))

/* Volatile variables to prevent optimization */
static volatile int g_loop_count = 100;
static volatile float g_scale = 2.5f;

#ifdef __AVX512BW__
/* V64QImode - 64 x 8-bit integers */
static int test_v64qi_blend(int argc) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i * 3);
        src2[i] = (uint8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_limit = (argc > 1) ? g_loop_count : 10;
    for (int iter = 0; iter < loop_limit; iter++) {
        __m512i temp = _mm512_add_epi8(v1, _mm512_set1_epi8(iter));
        result = _mm512_mask_blend_epi8(mask, result, temp);
        _mm512_store_si512((__m512i*)dst, result);
        
        /* Create artificial dependency */
        __asm__ volatile("" : : "r"(dst[0]) : "memory");
    }
    
    return sum & 0xFF;
}

/* V32HImode - 32 x 16-bit integers */
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i threshold = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, threshold);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(0x7FFF);
    result = _mm512_mask_blend_epi16(0xAAAAAAAA, result, broadcast);
    
    /* Store and compute sum */
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Use in loop with arithmetic */
    for (int i = 0; i < (argc % 8 + 1); i++) {
        __m512i add_result = _mm512_add_epi16(v1, v2);
        result = _mm512_mask_blend_epi16(0x55555555, result, add_result);
    }
    
    return sum & 0xFFFF;
}

/* V32HFmode - 32 x half-precision floats */
static int test_v32hf_blend(int argc) {
#ifdef __AVX512FP16__
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask */
    __m512h zero = _mm512_set1_ph(0.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, zero, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Additional blend with arithmetic result */
    __m512h mul_result = _mm512_mul_ph(v1, _mm512_set1_ph((_Float16)g_scale));
    result = _mm512_mask_blend_ph(0xAAAAAAAA, result, mul_result);
    
    _mm512_store_ph(dst, result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        union { _Float16 f; uint16_t u; } conv;
        conv.f = dst[i];
        sum += conv.u;
    }
    
    return sum & 0xFFFF;
#else
    (void)argc;
    return 0xDEAD; /* Return sentinel if FP16 not supported */
#endif
}

/* V32BFmode - 32 x bfloat16 */
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32]; /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Initialize bfloat16 patterns */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern: sign = 0, exponent = 127, fraction = i */
        src1[i] = (uint16_t)((0x7F << 7) | (i & 0x7F));
        src2[i] = (uint16_t)((0x7F << 7) | ((i + 16) & 0x7F));
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where i % 3 == 0 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* For bfloat16, we use epi16 blend on the integer representation */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast value */
    __m512i bcast = _mm512_set1_epi16(0x3F80); /* bfloat16 1.0 */
    result = _mm512_mask_blend_epi16(0x55555555, result, bcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc-dependent iterations */
    for (int i = 0; i < (argc % 4 + 1); i++) {
        __m512i add_val = _mm512_add_epi16(v1, _mm512_set1_epi16(i));
        result = _mm512_mask_blend_epi16(mask, result, add_val);
    }
    
    return sum & 0xFFFF;
}
#endif /* __AVX512BW__ */

#ifdef __AVX512F__
/* V16SImode - 16 x 32-bit integers */
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i threshold = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, threshold);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_epi32((void*)volatile_dst, result);
    
    /* Blend with arithmetic result */
    __m512i mul_result = _mm512_mullo_epi32(v1, _mm512_set1_epi32(3));
    result = _mm512_mask_blend_epi32(0xAAAA, result, mul_result);
    
    _mm512_store_epi32(dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i] + volatile_dst[i];
    }
    
    /* Loop with side effects */
    for (int i = 0; i < (argc % 5 + 2); i++) {
        __m512i temp = _mm512_add_epi32(v1, _mm512_set1_epi32(i * 100));
        result = _mm512_mask_blend_epi32(0x5555, result, temp);
        
        /* Memory barrier */
        __asm__ volatile("" : : "r"(dst) : "memory");
    }
    
    return sum;
}

/* V8DImode - 8 x 64-bit integers */
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL - 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __m512i threshold = _mm512_set1_epi64(30000LL);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, threshold);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i bcast = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFLL);
    result = _mm512_mask_blend_epi64(0xAA, result, bcast);
    
    _mm512_store_epi64(dst, result);
    
    /* Compute hash */
    int64_t hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= dst[i];
    }
    
    /* Additional blends in loop */
    for (int i = 0; i < (argc % 3 + 1); i++) {
        __m512i add_val = _mm512_add_epi64(v1, _mm512_set1_epi64(i * 1000LL));
        result = _mm512_mask_blend_epi64(0x55, result, add_val);
    }
    
    return (int)(hash ^ (hash >> 32));
}

/* V8DFmode - 8 x double-precision floats */
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(g_scale));
    result = _mm512_mask_blend_pd(0xAA, result, mul_result);
    
    _mm512_store_pd(dst, result);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        union { double d; uint64_t u; } conv;
        conv.d = dst[i];
        checksum += conv.u;
    }
    
    /* Loop with volatile dependency */
    volatile int vol = argc;
    for (int i = 0; i < (vol % 4 + 1); i++) {
        __m512d add_val = _mm512_add_pd(v1, _mm512_set1_pd(i * 0.1));
        result = _mm512_mask_blend_pd(0x55, result, add_val);
        
        /* Prevent optimization */
        __asm__ volatile("" : : "r"(dst) : "memory");
    }
    
    return (int)(checksum ^ (checksum >> 32));
}

/* V16SFmode - 16 x single-precision floats */
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f - 1.0f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 threshold = _mm512_set1_ps(10.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Store to volatile */
    _mm512_store_ps((void*)volatile_dst, result);
    
    /* Blend with arithmetic result */
    __m512 mul_result = _mm512_mul_ps(v1, _mm512_set1_ps(g_scale));
    result = _mm512_mask_blend_ps(0xAAAA, result, mul_result);
    
    _mm512_store_ps(dst, result);
    
    /* Compute sum */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i] + volatile_dst[i];
    }
    
    /* Loop with argc-dependent iterations */
    for (int i = 0; i < (argc % 6 + 2); i++) {
        __m512 add_val = _mm512_add_ps(v1, _mm512_set1_ps(i * 0.25f));
        result = _mm512_mask_blend_ps(0x5555, result, add_val);
    }
    
    /* Convert float sum to int for return */
    union { float f; uint32_t u; } conv;
    conv.f = sum;
    return (int)conv.u;
}
#endif /* __AVX512F__ */

/* Main driver function */
int main(int argc, char *argv[]) {
    int total_hash = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#ifdef __AVX512F__
    printf("AVX512F supported\n");
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    total_hash ^= test_v8df_blend(argc);
    total_hash ^= test_v16sf_blend(argc);
#else
    printf("AVX512F NOT supported - skipping F, DI, DF, SF tests\n");
#endif
    
#ifdef __AVX512BW__
    printf("AVX512BW supported\n");
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
#else
    printf("AVX512BW NOT supported - skipping QI, HI, HF, BF tests\n");
#endif
    
    printf("Final hash: 0x%08X\n", total_hash);
    
    /* Use result to affect return code */
    return (total_hash == 0) ? 1 : 0;
}
