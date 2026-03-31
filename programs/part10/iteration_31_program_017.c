/* test_avx512_blend.c - AVX-512 blend intrinsics coverage test */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Feature detection macros */
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

/* Alignment helper */
#define ALIGN_64 __attribute__((aligned(64)))

/* Volatile store to prevent optimization */
static inline void force_store_volatile(void* dest, __m512i val) {
    _mm512_store_si512((__m512i*)dest, val);
}

static inline void force_store_volatile_ps(void* dest, __m512 val) {
    _mm512_store_ps((float*)dest, val);
}

static inline void force_store_volatile_pd(void* dest, __m512d val) {
    _mm512_store_pd((double*)dest, val);
}

/* ==================== V64QI (64 x 8-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 volatile uint8_t dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i * 3;
        src2[i] = i * 5 + 1;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: alternating pattern based on argc */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + argc) & 1) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile to prevent optimization */
    force_store_volatile((void*)dst, blended);
    
    /* Use result in computation */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#else
static uint64_t test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32 x 16-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 volatile uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 7;
        src2[i] = i * 11 + 2;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(argc);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(0xABCD);
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, broadcast);
    
    force_store_volatile((void*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with blend inside */
    __m512i accum = _mm512_setzero_si512();
    int loop_count = (argc > 1) ? argc % 8 : 4;
    for (int j = 0; j < loop_count; j++) {
        __m512i temp = _mm512_add_epi16(v1, _mm512_set1_epi16(j));
        accum = _mm512_mask_blend_epi16(mask, accum, temp);
    }
    
    ALIGN_64 uint16_t accum_arr[32];
    _mm512_store_si512((__m512i*)accum_arr, accum);
    for (int i = 0; i < 32; i++) {
        sum += accum_arr[i];
    }
    
    return sum;
}
#else
static uint64_t test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32 x half-precision floats) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 volatile _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 0.7f + 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using float comparison */
    __m512h cmp_val = _mm512_set1_ph((_Float16)15.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph((_Float16*)dst, blended);
    
    /* Compute sum */
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];  /* Treat as bits */
    }
    
    return sum;
}
#else
static uint64_t test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <bfloat16.h>
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 volatile __bfloat16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.5f);
        src2[i] = bfloat16_from_float(i * 2.5f);
    }
    
    /* Load as epi16 since bfloat16 uses 16-bit storage */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - blend based on position and argc */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * argc) % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend using epi16 intrinsic (bfloat16 stored as 16-bit) */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_uint32(dst[i]);
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
    ALIGN_64 volatile int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 - 5;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(argc * 10);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend vector with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(3));
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, multiplied);
    
    force_store_volatile((void*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)dst[i];
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
    ALIGN_64 volatile int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1000000000LL;
        src2[i] = i * 2000000000LL + 123;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i + argc) % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    force_store_volatile((void*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    
    return sum;
}
#else
static uint64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8 x double-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 volatile double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.414;
        src2[i] = i * 2.718 + 1.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using float comparison */
    __m512d cmp_val = _mm512_set1_pd(argc * 2.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended = _mm512_mask_blend_pd(mask, v1, multiplied);
    
    force_store_volatile_pd((void*)dst, blended);
    
    /* Compute hash from double bits */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t bits;
        memcpy(&bits, &dst[i], sizeof(bits));
        sum ^= bits;
    }
    
    return sum;
}
#else
static uint64_t test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF (16 x single-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 volatile float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f;
        src2[i] = i * 0.33f + 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(argc * 0.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend inside loop with varying mask */
    __m512 accum = _mm512_setzero_ps();
    int loop_count = (argc > 0) ? (argc % 5) + 1 : 3;
    
    for (int j = 0; j < loop_count; j++) {
        __m512 temp = _mm512_add_ps(v1, _mm512_set1_ps(j * 0.1f));
        __mmask16 loop_mask = mask ^ (j & 0xFFFF);
        accum = _mm512_mask_blend_ps(loop_mask, accum, temp);
    }
    
    /* Final blend */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, accum);
    
    force_store_volatile_ps((void*)dst, blended);
    
    /* Compute checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        uint32_t bits;
        memcpy(&bits, &dst[i], sizeof(bits));
        sum += bits;
    }
    
    return sum;
}
#else
static uint64_t test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    (void)argv; /* Unused parameter */
    
    printf("AVX-512 Blend Intrinsics Coverage Test\n");
    printf("AVX512F supported: %s\n", HAS_AVX512F ? "YES" : "NO");
    printf("AVX512BW supported: %s\n", HAS_AVX512BW ? "YES" : "NO");
    
    if (!HAS_AVX512F && !HAS_AVX512BW) {
        printf("AVX-512 not supported on this platform. Skipping tests.\n");
        return 0;
    }
    
    uint64_t total_hash = 0;
    
    /* Run all blend tests */
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    total_hash ^= test_v8df_blend(argc);
    total_hash ^= test_v16sf_blend(argc);
    
    /* Use result to prevent dead code elimination */
    volatile uint64_t sink = total_hash;
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)sink);
    
    return (sink != 0) ? 0 : 1;
}
