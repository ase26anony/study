/* test_avx512_blend.c
 * 
 * This program is designed to trigger the RTL expansion for AVX-512 blend
 * instructions in GCC's i386 backend, specifically targeting the uncovered
 * switch cases in i386-expand.cc lines 4303-4326.
 *
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -fno-tree-vectorize test_avx512_blend.c -o test_avx512_blend
 * For RTL inspection: gcc -O3 -march=skylake-avx512 -S -dP test_avx512_blend.c -o test_avx512_blend.s
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature guards to prevent compilation errors */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64 x 8-bit integers) ==================== */
/* Targets: gen_avx512bw_blendmv64qi */
static int test_v64qi_blend(int argc) {
    /* 64-byte aligned arrays */
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    volatile __attribute__((aligned(64))) int8_t volatile_dst[64];
    
    /* Initialize with pattern data */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create blend mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(100);
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Also store to regular array */
    _mm512_store_epi32(dst, blended);
    
    /* Use result in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency through asm */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    /* Loop with argc-dependent iteration count */
    int loop_count = (argc > 1) ? 10 : 5;
    for (int iter = 0; iter < loop_count; iter++) {
        /* Blend with broadcasted scalar */
        __m512i scalar = _mm512_set1_epi8((int8_t)iter);
        __m512i blended2 = _mm512_mask_blend_epi8(mask, v1, scalar);
        
        /* Store to volatile */
        _mm512_store_epi32((void*)volatile_dst, blended2);
        
        /* Update sum */
        __m512i sum_vec = _mm512_add_epi8(blended, blended2);
        _mm512_store_epi32(dst, sum_vec);
        
        for (int i = 0; i < 64; i++) {
            sum += dst[i];
        }
    }
    
    return sum;
}

/* ==================== V32HImode (32 x 16-bit integers) ==================== */
/* Targets: gen_avx512bw_blendmv32hi */
static int test_v32hi_blend(int argc) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    volatile __attribute__((aligned(64))) int16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 7);
        src2[i] = (int16_t)(i * 11 - 5);
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(200);
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    _mm512_store_epi32(dst, blended);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi16(v1, v2);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, v1, added);
    
    _mm512_store_epi32((void*)volatile_dst, blended2);
    _mm512_store_epi32(dst, blended2);
    
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ==================== V32HFmode (32 x half-precision floats) ==================== */
/* Targets: gen_avx512bw_blendmv32hf */
static float test_v32hf_blend(int argc) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* Half as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst_data[32];
    volatile __attribute__((aligned(64))) uint16_t volatile_dst[32];
    
    /* Initialize with half-precision pattern (simple integer values) */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(i * 10);      /* Simple representation */
        src2_data[i] = (uint16_t)(i * 20 + 5);
    }
    
    /* Load as integers */
    __m512i v1 = _mm512_load_epi32(src1_data);
    __m512i v2 = _mm512_load_epi32(src2_data);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi16(150);
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend using half-precision intrinsic - should trigger gen_avx512bw_blendmv32hf */
    __m512i blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    _mm512_store_epi32(dst_data, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst_data[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* ==================== V32BFmode (32 x bfloat16) ==================== */
/* Targets: gen_avx512bw_blendmv32bf */
static float test_v32bf_blend(int argc) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* BF16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst_data[32];
    volatile __attribute__((aligned(64))) uint16_t volatile_dst[32];
    
    /* BF16 values (simple pattern) */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(i * 8);      /* BF16 representation */
        src2_data[i] = (uint16_t)(i * 16 + 3);
    }
    
    __m512i v1 = _mm512_load_epi32(src1_data);
    __m512i v2 = _mm512_load_epi32(src2_data);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend using epi16 intrinsic for BF16 - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    _mm512_store_epi32(dst_data, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst_data[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

#ifdef __AVX512F__

/* ==================== V16SImode (16 x 32-bit integers) ==================== */
/* Targets: gen_avx512f_blendmv16si */
static int test_v16si_blend(int argc) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    volatile __attribute__((aligned(64))) int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 - 8;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi32(100);
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    _mm512_store_epi32(dst, blended);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    /* Loop with argc-dependent iterations */
    int loop_count = (argc > 2) ? 8 : 4;
    for (int iter = 0; iter < loop_count; iter++) {
        /* Blend with broadcast scalar */
        __m512i scalar = _mm512_set1_epi32(iter * 10);
        __m512i blended2 = _mm512_mask_blend_epi32(mask, v1, scalar);
        
        /* Multiply and blend */
        __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(2));
        __m512i blended3 = _mm512_mask_blend_epi32(mask, blended2, multiplied);
        
        _mm512_store_epi32((void*)volatile_dst, blended3);
        _mm512_store_epi32(dst, blended3);
        
        for (int i = 0; i < 16; i++) {
            sum += dst[i];
        }
    }
    
    return sum;
}

/* ==================== V8DImode (8 x 64-bit integers) ==================== */
/* Targets: gen_avx512f_blendmv8di */
static long test_v8di_blend(int argc) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    volatile __attribute__((aligned(64))) int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 23LL;
        src2[i] = (int64_t)i * 29LL - 15LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(100);
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi64((void*)volatile_dst, blended);
    _mm512_store_epi64(dst, blended);
    
    /* Reduction */
    long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* ==================== V8DFmode (8 x double-precision floats) ==================== */
/* Targets: gen_avx512f_blendmv8df */
static double test_v8df_blend(int argc) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    volatile __attribute__((aligned(64))) double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)i * 1.5;
        src2[i] = (double)i * 2.5 - 1.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd((void*)volatile_dst, blended);
    _mm512_store_pd(dst, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    _mm512_store_pd((void*)volatile_dst, blended2);
    _mm512_store_pd(dst, blended2);
    
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* ==================== V16SFmode (16 x single-precision floats) ==================== */
/* Targets: gen_avx512f_blendmv16sf */
static float test_v16sf_blend(int argc) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    volatile __attribute__((aligned(64))) float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)i * 0.7f;
        src2[i] = (float)i * 1.3f - 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(5.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps((void*)volatile_dst, blended);
    _mm512_store_ps(dst, blended);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    /* Loop with argc-dependent iterations */
    int loop_count = (argc > 1) ? 6 : 3;
    for (int iter = 0; iter < loop_count; iter++) {
        /* Blend with broadcast scalar */
        __m512 scalar = _mm512_set1_ps((float)iter * 0.2f);
        __m512 blended2 = _mm512_mask_blend_ps(mask, v1, scalar);
        
        /* Add and blend */
        __m512 added = _mm512_add_ps(v1, v2);
        __m512 blended3 = _mm512_mask_blend_ps(mask, blended2, added);
        
        _mm512_store_ps((void*)volatile_dst, blended3);
        _mm512_store_ps(dst, blended3);
        
        for (int i = 0; i < 16; i++) {
            sum += dst[i];
        }
    }
    
    return sum;
}

#endif /* __AVX512F__ */

/* ==================== Main Driver ==================== */
int main(int argc, char **argv) {
    int total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing AVX-512 blend intrinsics (full BW+F support)...\n");
    
    /* Test all vector modes */
    total_checksum += test_v64qi_blend(argc);
    printf("  V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend(argc);
    printf("  V32HImode test completed\n");
    
    float hf_result = test_v32hf_blend(argc);
    total_checksum += (int)hf_result;
    printf("  V32HFmode test completed\n");
    
    float bf_result = test_v32bf_blend(argc);
    total_checksum += (int)bf_result;
    printf("  V32BFmode test completed\n");
    
    total_checksum += test_v16si_blend(argc);
    printf("  V16SImode test completed\n");
    
    long di_result = test_v8di_blend(argc);
    total_checksum += (int)di_result;
    printf("  V8DImode test completed\n");
    
    double df_result = test_v8df_blend(argc);
    total_checksum += (int)df_result;
    printf("  V8DFmode test completed\n");
    
    float sf_result = test_v16sf_blend(argc);
    total_checksum += (int)sf_result;
    printf("  V16SFmode test completed\n");
    
#else /* __AVX512BW__ not defined */
    printf("Testing AVX-512 blend intrinsics (F support only)...\n");
    
    /* Test only AVX512F modes */
    total_checksum += test_v16si_blend(argc);
    printf("  V16SImode test completed\n");
    
    long di_result = test_v8di_blend(argc);
    total_checksum += (int)di_result;
    printf("  V8DImode test completed\n");
    
    double df_result = test_v8df_blend(argc);
    total_checksum += (int)df_result;
    printf("  V8DFmode test completed\n");
    
    float sf_result = test_v16sf_blend(argc);
    total_checksum += (int)sf_result;
    printf("  V16SFmode test completed\n");
    
    printf("  Note: AVX512BW modes (V64QI, V32HI, V32HF, V32BF) skipped\n");
#endif /* __AVX512BW__ */
#else /* __AVX512F__ not defined */
    printf("AVX-512 not supported on this compiler/hardware\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif /* __AVX512F__ */
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use argc to affect final result (prevent dead code elimination) */
    return total_checksum % 256;
}
