/* test_avx512_blend.c - Coverage for AVX-512 blend RTL patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature guards for compilation */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

/* Volatile variable to prevent optimization */
static volatile int g_volatile_counter = 0;

/* ==================== V64QI (64 x 8-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison - alternating pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask - triggers gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Use in reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)volatile_dst[i];
    }
    
    /* Artificial dependency on argc */
    int loop_count = (argc > 1) ? 10 : 5;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i blended2 = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAAULL, 
                                                 temp, blended);
        _mm512_store_si512((__m512i*)dst, blended2);
        sum += dst[i % 64];
    }
    
    /* Memory barrier */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#else
static uint64_t test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32 x 16-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, broadcast);
    
    /* Store and use in arithmetic */
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];
    }
    
    /* Loop with blend inside */
    for (int i = 0; i < (argc + 2); i++) {
        __m512i temp = _mm512_add_epi16(v1, _mm512_set1_epi16(i));
        __m512i blended2 = _mm512_mask_blend_epi16(mask, temp, v2);
        _mm512_store_si512((__m512i*)dst, blended2);
        sum += dst[i % 32];
    }
    
    return sum;
}
#else
static uint64_t test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32 x half precision) ==================== */
#if HAS_AVX512BW
#include <x86intrin.h>  /* For _Float16 */
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __m512h cmp_val = _mm512_set1_ph(10.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - triggers gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store to volatile location */
    volatile ALIGN_64 _Float16 volatile_dst[32];
    _mm512_store_ph((void*)volatile_dst, blended);
    
    /* Reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(volatile_dst[i] * 100);
    }
    
    /* Additional blend with arithmetic result */
    __m512h multiplied = _mm512_mul_ph(v1, _mm512_set1_ph(2.0f));
    __m512h blended2 = _mm512_mask_blend_ph(mask ^ 0x55555555, multiplied, blended);
    _mm512_store_ph(dst, blended2);
    
    for (int i = 0; i < 32; i += 2) {
        sum += (uint16_t)(dst[i] * 50);
    }
    
    return sum;
}
#else
static uint64_t test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Simple bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = (i << 8) | 0x80;  /* Approx i * 1.0 */
        src2[i] = (i << 7) | 0x40;  /* Approx i * 0.5 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - every other element */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend using epi16 intrinsic (bfloat16 uses 16-bit lanes) */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with blend */
    for (int i = 0; i < (argc + 1); i++) {
        __m512i shifted = _mm512_slli_epi16(v1, 1);
        __m512i blended2 = _mm512_mask_blend_epi16(mask, shifted, v2);
        _mm512_store_si512((__m512i*)dst, blended2);
        sum += dst[i % 32];
    }
    
    return sum;
}
#else
static uint64_t test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16 x 32-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - triggers gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Store and reduce */
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)dst[i];
    }
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(argc * 100));
    __m512i blended2 = _mm512_mask_blend_epi32(mask ^ 0xAAAA, added, blended);
    
    volatile ALIGN_64 int32_t volatile_dst[16];
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}
#else
static uint64_t test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8 x 64-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL + 5000;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(30000);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Blend - triggers gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    
    /* Multiple blends in loop */
    for (int i = 0; i < (argc + 3); i++) {
        __m512i shifted = _mm512_slli_epi64(v1, 1);
        __m512i blended2 = _mm512_mask_blend_epi64(mask, shifted, v2);
        _mm512_store_si512((__m512i*)dst, blended2);
        sum += dst[i % 8];
    }
    
    return sum;
}
#else
static uint64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8 x double precision) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - triggers gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Store to volatile */
    volatile ALIGN_64 double volatile_dst[8];
    _mm512_store_pd((void*)volatile_dst, blended);
    
    /* Reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(volatile_dst[i] * 1000.0);
    }
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended2 = _mm512_mask_blend_pd(mask ^ 0xAA, multiplied, blended);
    _mm512_store_pd(dst, blended2);
    
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(dst[i] * 500.0);
    }
    
    return sum;
}
#else
static uint64_t test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF (16 x single precision) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = i * 1.5f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask */
    __m512 cmp_val = _mm512_set1_ps(6.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - triggers gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(dst[i] * 100.0f);
    }
    
    /* Loop with blend */
    for (int i = 0; i < (argc + 2); i++) {
        __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(i * 0.1f));
        __m512 blended2 = _mm512_mask_blend_ps(mask, added, v2);
        _mm512_store_ps(dst, blended2);
        sum += (uint32_t)(dst[i % 16] * 10.0f);
    }
    
    /* Memory barrier */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#else
static uint64_t test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    uint64_t total_hash = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 features detected.\n");
    
    /* Call all test functions */
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    total_hash ^= test_v8df_blend(argc);
    total_hash ^= test_v16sf_blend(argc);
    
    /* Additional computation to prevent dead code elimination */
    total_hash += g_volatile_counter;
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)total_hash);
    
    /* Return non-zero if any test produced zero (unlikely) */
    return (total_hash == 0) ? 1 : 0;
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 2;
#endif
}
