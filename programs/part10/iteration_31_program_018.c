/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper function to create masks from patterns */
static __mmask64 create_mask64(uint64_t pattern) {
    return _mm512_int2mask(pattern);
}

static __mmask32 create_mask32(uint32_t pattern) {
    return _mm512_int2mask(pattern);
}

static __mmask16 create_mask16(uint16_t pattern) {
    return _mm512_int2mask(pattern);
}

/* V64QI - 64-byte integers */
__attribute__((noinline))
uint64_t test_v64qi_blend() {
    __attribute__((aligned(64))) uint8_t src1[64];
    __attribute__((aligned(64))) uint8_t src2[64];
    __attribute__((aligned(64))) volatile uint8_t dst[64];
    
    /* Initialize with alternating patterns */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 255 - i;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create alternating mask: 0xAAAAAAAAAAAAAAAA */
    __mmask64 mask = create_mask64(0xAAAAAAAAAAAAAAAAULL);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Compute checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V32HI - 32 half-word integers */
__attribute__((noinline))
uint64_t test_v32hi_blend() {
    __attribute__((aligned(64))) uint16_t src1[32];
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) volatile uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create checkerboard mask */
    __mmask32 mask = create_mask32(0xAAAAAAAA);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HF - 32 half-precision floats */
__attribute__((noinline))
float test_v32hf_blend() {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* _Float16 storage */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) volatile uint16_t dst[32];
    
    /* Initialize as half-precision pattern */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = i * 0x100;  /* Simple pattern */
        src2_data[i] = i * 0x200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    __mmask32 mask = create_mask32(0x55555555);
    
    /* Use integer blend for half-precision (same as V32HI) */
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    
    return sum;
}

/* V32BF - 32 bfloat16 floats */
__attribute__((noinline))
float test_v32bf_blend() {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* bfloat16 storage */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) volatile uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (i << 8) | 0x80;  /* bfloat16 pattern */
        src2_data[i] = (i << 8) | 0x40;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    __mmask32 mask = create_mask32(0x33333333);
    
    /* Use integer blend for bfloat16 (same as V32HI) */
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    
    return sum;
}

/* V16SI - 16 single-word integers */
__attribute__((noinline))
uint64_t test_v16si_blend() {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) volatile int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 20;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_LT);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi32(999);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, broadcast);
    
    _mm512_store_epi32(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DI - 8 double-word integers */
__attribute__((noinline))
uint64_t test_v8di_blend() {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) volatile int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1000LL;
        src2[i] = i * 2000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    __mmask8 mask = create_mask16(0xAA);  /* Lower 8 bits */
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi64(v1, v2);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, added);
    
    _mm512_store_epi64(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DF - 8 double-precision floats */
__attribute__((noinline))
double test_v8df_blend() {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) volatile double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend with multiplied result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, multiplied);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V16SF - 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend(int iterations) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) volatile float dst[16];
    
    /* Initialize with trigonometric pattern */
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.1f;
        src2[i] = i * 0.2f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    float total = 0.0f;
    
    /* Loop to prevent optimization */
    for (int iter = 0; iter < iterations; iter++) {
        /* Varying mask based on iteration */
        __mmask16 mask = create_mask16(iter & 0xFFFF);
        
        /* Blend with added result */
        __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(iter * 0.01f));
        
        /* This should trigger gen_avx512f_blendmv16sf */
        __m512 result = _mm512_mask_blend_ps(mask, v1, added);
        
        _mm512_store_ps(dst, result);
        
        /* Reduction */
        for (int i = 0; i < 16; i++) {
            total += dst[i];
        }
        
        /* Rotate vectors */
        v1 = _mm512_permutexvar_ps(_mm512_setr_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0), v1);
    }
    
    return total;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(int argc, char** argv) {
    uint64_t total_hash = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    
    printf("Testing AVX-512 blend intrinsics...\n");
    
    /* Test all vector types */
    total_hash += test_v64qi_blend();
    printf("V64QI blend test completed\n");
    
    total_hash += test_v32hi_blend();
    printf("V32HI blend test completed\n");
    
    float hf_result = test_v32hf_blend();
    total_hash += (uint64_t)hf_result;
    printf("V32HF blend test completed: %f\n", hf_result);
    
    float bf_result = test_v32bf_blend();
    total_hash += (uint64_t)bf_result;
    printf("V32BF blend test completed: %f\n", bf_result);
    
    total_hash += test_v16si_blend();
    printf("V16SI blend test completed\n");
    
    total_hash += test_v8di_blend();
    printf("V8DI blend test completed\n");
    
    double df_result = test_v8df_blend();
    total_hash += (uint64_t)df_result;
    printf("V8DF blend test completed: %f\n", df_result);
    
    /* Use argc to control loop iterations (prevents dead code elimination) */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    float sf_result = test_v16sf_blend(iterations);
    total_hash += (uint64_t)sf_result;
    printf("V16SF blend test completed: %f (iterations=%d)\n", sf_result, iterations);
    
    printf("All AVX-512 blend tests completed. Total hash: %lu\n", total_hash);
    
#else
    printf("AVX-512BW not supported on this platform\n");
#endif
#else
    printf("AVX-512F not supported on this platform\n");
#endif
    
    return (int)(total_hash & 0x7FFFFFFF);
}
