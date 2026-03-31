/* test_avx512_blend.c - Coverage for AVX-512 blend RTL patterns */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature guards for modular compilation */
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

/* Helper to prevent optimization */
static inline void use_result(void* result) {
    __asm__ volatile("" : : "r"(result) : "memory");
}

/* ==================== V64QI (64 x int8_t) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile int8_t* volatile_dst = dst;  /* Volatile store target */
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: blend where src1[i] < 100 */
    __mmask64 mask = _mm512_cmplt_epi8_mask(v1, _mm512_set1_epi8(100));
    
    /* Blend with intrinsic that should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Use in reduction with loop dependent on argc */
    int64_t sum = 0;
    int loop_count = (argc > 1) ? 64 : 32;
    for (int i = 0; i < loop_count; i++) {
        sum += dst[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : "+r"(sum) : : "memory");
    
    return (uint64_t)sum;
}
#endif

/* ==================== V32HI (32 x int16_t) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    volatile int16_t* volatile_dst = dst;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 7);
        src2[i] = (int16_t)(i * 11 + 2);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(50));
    
    /* Should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    /* Reduction with loop */
    int64_t sum = 0;
    for (int i = 0; i < ((argc % 32) + 1); i++) {
        sum += dst[i];
    }
    
    use_result(&sum);
    return (uint64_t)sum;
}
#endif

/* ==================== V32HF (32 x _Float16) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    volatile _Float16* volatile_dst = dst;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: blend where src1 > 10.0 */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, _mm512_set1_ph(10.0f), _CMP_GT_OQ);
    
    /* Should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Additional blend with arithmetic result */
    __m512h added = _mm512_add_ph(v1, _mm512_set1_ph(5.0f));
    __m512h blended2 = _mm512_mask_blend_ph(mask, blended, added);
    
    _mm512_store_ph((void*)volatile_dst, blended2);
    
    /* Compute checksum */
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += *(uint16_t*)&dst[i];
    }
    
    return checksum;
}
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 __bf16 src1[32];
    ALIGN_64 __bf16 src2[32];
    ALIGN_64 __bf16 dst[32];
    volatile __bf16* volatile_dst = dst;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bf16)(i * 1.25f);
        src2[i] = (__bf16)(i * 1.75f + 0.5f);
    }
    
    /* Load as integers since bfloat16 blends use epi16 */
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - blend every other element */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Reduction */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += *(uint16_t*)&dst[i];
    }
    
    return sum;
}
#endif

/* ==================== V16SI (16 x int32_t) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile int32_t* volatile_dst = dst;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 + 3;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, _mm512_set1_epi32(0));
    
    /* Should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(2));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, multiplied);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    /* Loop-dependent computation */
    int64_t sum = 0;
    int iterations = (argc > 0) ? 16 : 8;
    for (int i = 0; i < iterations; i++) {
        sum += dst[i];
    }
    
    use_result(&sum);
    return (uint64_t)sum;
}
#endif

/* ==================== V8DI (8 x int64_t) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    volatile int64_t* volatile_dst = dst;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 23LL;
        src2[i] = i * 29LL + 7LL;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: blend where src1 is even */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (src1[i] % 2 == 0) mask |= (1 << i);
    }
    
    /* Should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Compute hash */
    uint64_t hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= dst[i];
        hash = (hash << 13) | (hash >> 51);
    }
    
    return hash;
}
#endif

/* ==================== V8DF (8 x double) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile double* volatile_dst = dst;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.414;
        src2[i] = i * 2.718 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(5.0), _CMP_LT_OQ);
    
    /* Should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d sqrt_result = _mm512_sqrt_pd(v1);
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, sqrt_result);
    
    _mm512_store_pd(volatile_dst, blended2);
    
    /* Compute checksum */
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= *(uint64_t*)&dst[i];
    }
    
    return checksum;
}
#endif

/* ==================== V16SF (16 x float) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile float* volatile_dst = dst;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.707f;
        src2[i] = i * 1.618f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask: blend where src1 > src2 */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Multiple blends in sequence */
    __m512 abs_result = _mm512_abs_ps(v1);
    __m512 blended2 = _mm512_mask_blend_ps(mask, blended, abs_result);
    
    __m512 sin_result = _mm512_sin_ps(v2);  /* If available */
    __m512 blended3 = _mm512_mask_blend_ps(mask, blended2, sin_result);
    
    _mm512_store_ps(volatile_dst, blended3);
    
    /* Loop with argc dependency */
    float sum = 0.0f;
    int loop_count = (argc % 16) + 1;
    for (int i = 0; i < loop_count; i++) {
        sum += dst[i];
    }
    
    return *(uint32_t*)&sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char* argv[]) {
    uint64_t final_hash = 0xDEADBEEFCAFEBABEULL;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    /* Call enabled test functions */
    #if HAS_AVX512BW
    final_hash ^= test_v64qi_blend(argc);
    final_hash = (final_hash << 17) | (final_hash >> 47);
    
    final_hash ^= test_v32hi_blend(argc);
    final_hash = (final_hash << 19) | (final_hash >> 45);
    
    #if defined(__AVX512FP16__)
    final_hash ^= test_v32hf_blend(argc);
    final_hash = (final_hash << 23) | (final_hash >> 41);
    #endif
    
    #if defined(__AVX512BF16__)
    final_hash ^= test_v32bf_blend(argc);
    final_hash = (final_hash << 29) | (final_hash >> 35);
    #endif
    #endif  /* AVX512BW */
    
    #if HAS_AVX512F
    final_hash ^= test_v16si_blend(argc);
    final_hash = (final_hash << 31) | (final_hash >> 33);
    
    final_hash ^= test_v8di_blend(argc);
    final_hash = (final_hash << 37) | (final_hash >> 27);
    
    final_hash ^= test_v8df_blend(argc);
    final_hash = (final_hash << 41) | (final_hash >> 23);
    
    final_hash ^= test_v16sf_blend(argc);
    final_hash = (final_hash << 43) | (final_hash >> 21);
    #endif  /* AVX512F */
    
    printf("Final hash: 0x%016llX\n", (unsigned long long)final_hash);
    
    /* Use result to prevent dead code elimination */
    volatile uint64_t volatile_hash = final_hash;
    if (volatile_hash == 0x123456789ABCDEF0ULL) {
        printf("Impossible branch - just for control flow\n");
    }
    
    return (int)(final_hash & 0x7FFFFFFF);
#else
    printf("AVX-512 not supported on this platform\n");
    return 1;
#endif
}
