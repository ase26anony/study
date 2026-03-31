/* test_avx512_blend.c
 * 
 * This test program is designed to trigger the specific RTL expansion patterns
 * in i386-expand.cc lines 4303-4326 for AVX-512 blend operations.
 * 
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -o test_avx512_blend test_avx512_blend.c
 * For RTL verification: gcc -O3 -march=skylake-avx512 -S -dP test_avx512_blend.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Force alignment for all vector arrays */
#define ALIGN_64 __attribute__((aligned(64)))

/* Prevent dead code elimination */
static volatile int g_volatile_sink = 0;

#ifdef __AVX512F__

/* ==================== V16SF (16x float) ==================== */
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                               9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ALIGN_64 float src2[16] = {100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                               900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f};
    ALIGN_64 volatile float result[16] ALIGN_64;
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison: mask = v1 > 8.0f */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
    
    /* Blend using mask: result = mask ? v1 : v2 */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((void*)result, blended);
    
    /* Use in reduction with loop dependent on argc */
    float sum = 0.0f;
    int iterations = (argc > 1) ? 100 : 10;
    for (int i = 0; i < iterations; i++) {
        __m512 temp = _mm512_add_ps(blended, _mm512_set1_ps(i * 0.1f));
        sum += _mm512_reduce_add_ps(temp);
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* ==================== V8DF (8x double) ==================== */
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    ALIGN_64 double src2[8] = {100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0};
    ALIGN_64 volatile double result[8] ALIGN_64;
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask: elements where v1 > 4.0 */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(4.0), _CMP_GT_OQ);
    
    /* Blend with mask */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_store_pd((void*)result, blended);
    
    /* Use blended result in arithmetic operation */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(1.5));
    
    double sum = 0.0;
    int loop_count = argc + 5;
    for (int i = 0; i < loop_count; i++) {
        __m512d temp = _mm512_add_pd(multiplied, _mm512_set1_pd(i * 0.01));
        /* Horizontal sum */
        sum += _mm512_reduce_add_pd(temp);
    }
    
    return sum;
}

/* ==================== V16SI (16x int32_t) ==================== */
static int32_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ALIGN_64 int32_t src2[16] = {100, 200, 300, 400, 500, 600, 700, 800, 
                                 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600};
    ALIGN_64 volatile int32_t result[16] ALIGN_64;
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison: mask = v1 > 8 */
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, _mm512_set1_epi32(8), _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_store_epi32((void*)result, blended);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi32(42);
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, broadcast);
    
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i] + ((int32_t*)&blended2)[i];
    }
    
    g_volatile_sink = sum;
    return sum;
}

/* ==================== V8DI (8x int64_t) ==================== */
static int64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ALIGN_64 int64_t src2[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    ALIGN_64 volatile int64_t result[8] ALIGN_64;
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: odd elements */
    __mmask8 mask = 0xAA; /* 0b10101010 - select odd elements */
    
    /* Blend with mask */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_epi64((void*)result, blended);
    
    /* Use in loop with argc dependency */
    int64_t sum = 0;
    int iterations = (argc > 0) ? argc * 2 : 4;
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_add_epi64(blended, _mm512_set1_epi64(i));
        /* Manual reduction */
        int64_t* ptr = (int64_t*)&temp;
        for (int j = 0; j < 8; j++) {
            sum += ptr[j];
        }
    }
    
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* ==================== V64QI (64x int8_t) ==================== */
static int32_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t result[64] ALIGN_64;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 100 + i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask: select elements where (i % 3) == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Additional blend with arithmetic result */
    __m512i added = _mm512_add_epi8(v1, _mm512_set1_epi8(10));
    __m512i blended2 = _mm512_mask_blend_epi8(mask, blended, added);
    
    int32_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
        sum += ((int8_t*)&blended2)[i];
    }
    
    return sum;
}

/* ==================== V32HI (32x int16_t) ==================== */
static int32_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t result[32] ALIGN_64;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 10;
        src2[i] = 1000 + i * 20;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask using comparison: src1[i] > 150 */
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, _mm512_set1_epi16(150), _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Use in reduction with loop */
    int32_t sum = 0;
    int loop_count = (argc > 0) ? argc + 10 : 20;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_add_epi16(blended, _mm512_set1_epi16(i));
        int16_t* ptr = (int16_t*)&temp;
        for (int j = 0; j < 32; j++) {
            sum += ptr[j];
        }
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* ==================== V32HF (32x half precision float) ==================== */
static float test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* Store as uint16_t for half float */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t result[32] ALIGN_64;
    
    /* Initialize with simple half-float pattern (1.0, 2.0, ...) */
    for (int i = 0; i < 32; i++) {
        /* Simple half-float representation: 1.0 = 0x3C00 */
        src1_data[i] = 0x3C00 + i; /* Not exact float values, but valid half pattern */
        src2_data[i] = 0x4000 + i * 2; /* 2.0 = 0x4000 */
    }
    
    __m512i v1 = _mm512_load_si512(src1_data);
    __m512i v2 = _mm512_load_si512(src2_data);
    
    /* Create mask: select every other element */
    __mmask32 mask = 0xAAAAAAAA; /* 0b10101010... */
    
    /* Blend half floats using epi16 intrinsic (same size) */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Convert to float for reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Simple conversion for demonstration */
        sum += (float)(result[i] & 0x7FFF) / 16384.0f;
    }
    
    return sum;
}

/* ==================== V32BF (32x bfloat16) ==================== */
static float test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32]; /* bfloat16 stored as uint16_t */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t result[32] ALIGN_64;
    
    /* Initialize bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* bfloat16 representation: 1.0f = 0x3F80 */
        src1_data[i] = 0x3F80 + i;
        src2_data[i] = 0x4000 + i * 2; /* 2.0f = 0x4000 */
    }
    
    __m512i v1 = _mm512_load_si512(src1_data);
    __m512i v2 = _mm512_load_si512(src2_data);
    
    /* Create mask using comparison on integer representation */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((src1_data[i] & 0x7FFF) > 0x3F80) { /* > 1.0 in bfloat16 */
            mask |= (1U << i);
        }
    }
    
    /* Blend bfloat16 using epi16 intrinsic */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Convert to float for checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Convert bfloat16 to float (simplified) */
        uint32_t val = result[i] << 16;
        float f;
        memcpy(&f, &val, sizeof(float));
        sum += f;
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* ==================== Main Driver ==================== */
int main(int argc, char* argv[]) {
    uint64_t final_hash = 0;
    
#ifdef __AVX512F__
    printf("AVX512F supported, testing V16SF, V8DF, V16SI, V8DI blends...\n");
    
    /* Test each blend type and accumulate results */
    float f1 = test_v16sf_blend(argc);
    final_hash ^= *(uint32_t*)&f1;
    
    double d1 = test_v8df_blend(argc);
    final_hash ^= *(uint64_t*)&d1;
    
    int32_t i1 = test_v16si_blend(argc);
    final_hash ^= (uint64_t)i1;
    
    int64_t l1 = test_v8di_blend(argc);
    final_hash ^= (uint64_t)l1;
#else
    printf("AVX512F not supported, skipping tests...\n");
#endif

#ifdef __AVX512BW__
    printf("AVX512BW supported, testing V64QI, V32HI, V32HF, V32BF blends...\n");
    
    int32_t i2 = test_v64qi_blend(argc);
    final_hash ^= (uint64_t)i2;
    
    int32_t i3 = test_v32hi_blend(argc);
    final_hash ^= (uint64_t)i3;
    
    float f2 = test_v32hf_blend(argc);
    final_hash ^= *(uint32_t*)&f2;
    
    float f3 = test_v32bf_blend(argc);
    final_hash ^= *(uint32_t*)&f3;
#else
    printf("AVX512BW not supported, skipping tests...\n");
#endif

    /* Use final result to prevent optimization */
    g_volatile_sink = (int)final_hash;
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)final_hash);
    
    /* Return non-zero if any test produced zero result (unlikely) */
    return (final_hash == 0) ? 1 : 0;
}
