/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature guards for AVX-512 extensions */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper function to create masks from patterns */
static __mmask64 create_mask64_pattern(int pattern) {
    uint64_t mask_data[8] __attribute__((aligned(64)));
    for (int i = 0; i < 8; i++) {
        mask_data[i] = (pattern & (1 << (i % 8))) ? 0xFFFFFFFFFFFFFFFFULL : 0x0ULL;
    }
    return _mm512_load_epi64(mask_data);
}

static __mmask32 create_mask32_pattern(int pattern) {
    uint32_t mask_data[16] __attribute__((aligned(64)));
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (pattern & (1 << (i % 8))) ? 0xFFFFFFFF : 0x0;
    }
    return _mm512_load_epi32(mask_data);
}

static __mmask16 create_mask16_pattern(int pattern) {
    uint16_t mask_data[32] __attribute__((aligned(64)));
    for (int i = 0; i < 32; i++) {
        mask_data[i] = (pattern & (1 << (i % 8))) ? 0xFFFF : 0x0;
    }
    return _mm512_load_epi32(mask_data);
}

/* ========== V64QImode (64x8-bit integers) ========== */
/* Targets: gen_avx512bw_blendmv64qi */
static int test_v64qi_blend(int argc) {
    /* Use argc to create non-constant loop count */
    int iterations = (argc > 1) ? 100 : 50;
    int result = 0;
    
    /* Aligned arrays for 64-byte vectors */
    uint8_t a64[64] __attribute__((aligned(64)));
    uint8_t b64[64] __attribute__((aligned(64)));
    volatile uint8_t out64[64] __attribute__((aligned(64)));
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a64[i] = i;
        b64[i] = 64 - i;
    }
    
    /* Create mask using comparison */
    __m512i va = _mm512_load_epi64(a64);
    __m512i vb = _mm512_load_epi64(b64);
    
    /* Generate mask: select where a[i] > 32 */
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    /* Loop to prevent optimization */
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
        __m512i blended = _mm512_mask_blend_epi8(mask, va, vb);
        
        /* Store to volatile to prevent optimization */
        _mm512_store_epi64((void*)out64, blended);
        
        /* Use result in computation */
        for (int i = 0; i < 64; i += 8) {
            result += out64[i];
        }
        
        /* Modify mask slightly each iteration */
        mask = _mm512_kxor(mask, 0xAAAAAAAAAAAAAAAAULL);
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== V32HImode (32x16-bit integers) ========== */
/* Targets: gen_avx512bw_blendmv32hi */
static int test_v32hi_blend(int argc) {
    int iterations = (argc > 2) ? 80 : 40;
    int result = 0;
    
    uint16_t a32[32] __attribute__((aligned(64)));
    uint16_t b32[32] __attribute__((aligned(64)));
    volatile uint16_t out32[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a32[i] = i * 100;
        b32[i] = i * 200;
    }
    
    __m512i va = _mm512_load_epi32(a32);
    __m512i vb = _mm512_load_epi32(b32);
    
    /* Create mask from pattern */
    __mmask32 mask = create_mask32_pattern(0x55); /* 01010101 pattern */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
        __m512i blended = _mm512_mask_blend_epi16(mask, va, vb);
        
        /* Also blend with broadcasted scalar */
        __m512i broadcast = _mm512_set1_epi16(iter);
        blended = _mm512_mask_blend_epi16(mask ^ 0xFF, blended, broadcast);
        
        _mm512_store_epi32((void*)out32, blended);
        
        for (int i = 0; i < 32; i += 4) {
            result += out32[i];
        }
        
        /* Rotate mask */
        mask = (mask << 1) | (mask >> 31);
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== V32HFmode (32x16-bit half precision floats) ========== */
/* Targets: gen_avx512bw_blendmv32hf */
static int test_v32hf_blend(int argc) {
    int iterations = (argc > 3) ? 60 : 30;
    int result = 0;
    
    uint16_t a32hf[32] __attribute__((aligned(64)));
    uint16_t b32hf[32] __attribute__((aligned(64)));
    volatile uint16_t out32hf[32] __attribute__((aligned(64)));
    
    /* Initialize with half-float pattern (as integers) */
    for (int i = 0; i < 32; i++) {
        a32hf[i] = 0x3C00 | (i & 0x3FF); /* ~1.0 with variations */
        b32hf[i] = 0x4000 | (i & 0x3FF); /* ~2.0 with variations */
    }
    
    __m512i va = _mm512_load_epi32(a32hf);
    __m512i vb = _mm512_load_epi32(b32hf);
    
    /* Cast to __m512h for blend operation */
    __m512h vah = _mm512_castsi512_ph(va);
    __m512h vbh = _mm512_castsi512_ph(vb);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(vah, vbh, _CMP_LT_OQ);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend half-precision floats - should trigger gen_avx512bw_blendmv32hf */
        __m512h blended = _mm512_mask_blend_ph(mask, vah, vbh);
        
        /* Blend with arithmetic result */
        __m512h added = _mm512_add_ph(vah, _mm512_set1_ph(1.0f));
        blended = _mm512_mask_blend_ph(mask ^ 0xFFFF, blended, added);
        
        _mm512_store_epi32((void*)out32hf, _mm512_castph_si512(blended));
        
        for (int i = 0; i < 32; i += 2) {
            result += out32hf[i] & 0xFF;
        }
        
        mask = _mm512_knot(mask);
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== V32BFmode (32x16-bit bfloat16) ========== */
/* Targets: gen_avx512bw_blendmv32bf */
static int test_v32bf_blend(int argc) {
    int iterations = (argc > 4) ? 70 : 35;
    int result = 0;
    
    uint16_t a32bf[32] __attribute__((aligned(64)));
    uint16_t b32bf[32] __attribute__((aligned(64)));
    volatile uint16_t out32bf[32] __attribute__((aligned(64)));
    
    /* Initialize bfloat16 pattern (as integers) */
    for (int i = 0; i < 32; i++) {
        a32bf[i] = (0x3F80 + i) & 0xFFFF; /* ~1.0f in bfloat16 */
        b32bf[i] = (0x4000 + i) & 0xFFFF; /* ~2.0f in bfloat16 */
    }
    
    __m512i va = _mm512_load_epi32(a32bf);
    __m512i vb = _mm512_load_epi32(b32bf);
    
    /* For bfloat16, we use integer blend since there's no direct BF16 blend intrinsic */
    __mmask32 mask = create_mask32_pattern(0xAA); /* 10101010 pattern */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend bfloat16 using epi16 blend - should trigger gen_avx512bw_blendmv32bf */
        __m512i blended = _mm512_mask_blend_epi16(mask, va, vb);
        
        /* Blend with pattern-modified vector */
        __m512i pattern = _mm512_set1_epi16(iter * 0x0101);
        blended = _mm512_mask_blend_epi16(mask ^ 0x5555, blended, pattern);
        
        _mm512_store_epi32((void*)out32bf, blended);
        
        for (int i = 0; i < 32; i += 4) {
            result += out32bf[i];
        }
        
        mask = _mm512_kshiftli_mask(mask, 1);
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ========== AVX512F-only tests (no BW required) ========== */
#ifdef __AVX512F__

/* ========== V16SImode (16x32-bit integers) ========== */
/* Targets: gen_avx512f_blendmv16si */
static int test_v16si_blend(int argc) {
    int iterations = (argc > 5) ? 90 : 45;
    int result = 0;
    
    int32_t a16si[16] __attribute__((aligned(64)));
    int32_t b16si[16] __attribute__((aligned(64)));
    volatile int32_t out16si[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a16si[i] = i * 1000;
        b16si[i] = i * 2000;
    }
    
    __m512i va = _mm512_load_epi32(a16si);
    __m512i vb = _mm512_load_epi32(b16si);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(8000));
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend 32-bit integers - should trigger gen_avx512f_blendmv16si */
        __m512i blended = _mm512_mask_blend_epi32(mask, va, vb);
        
        /* Blend with arithmetic result */
        __m512i multiplied = _mm512_mullo_epi32(va, _mm512_set1_epi32(2));
        blended = _mm512_mask_blend_epi32(mask ^ 0xAAAA, blended, multiplied);
        
        _mm512_store_epi32((void*)out16si, blended);
        
        for (int i = 0; i < 16; i += 2) {
            result += out16si[i];
        }
        
        mask = _mm512_kxor(mask, 0x5555);
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== V8DImode (8x64-bit integers) ========== */
/* Targets: gen_avx512f_blendmv8di */
static int test_v8di_blend(int argc) {
    int iterations = (argc > 6) ? 75 : 38;
    int result = 0;
    
    int64_t a8di[8] __attribute__((aligned(64)));
    int64_t b8di[8] __attribute__((aligned(64)));
    volatile int64_t out8di[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a8di[i] = i * 10000LL;
        b8di[i] = i * 20000LL;
    }
    
    __m512i va = _mm512_load_epi64(a8di);
    __m512i vb = _mm512_load_epi64(b8di);
    
    /* Create mask from pattern */
    __mmask8 mask = create_mask64_pattern(0x33) & 0xFF; /* 00110011 pattern */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend 64-bit integers - should trigger gen_avx512f_blendmv8di */
        __m512i blended = _mm512_mask_blend_epi64(mask, va, vb);
        
        /* Blend with shifted vector */
        __m512i shifted = _mm512_slli_epi64(va, 1);
        blended = _mm512_mask_blend_epi64(mask ^ 0x0F, blended, shifted);
        
        _mm512_store_epi64((void*)out8di, blended);
        
        for (int i = 0; i < 8; i++) {
            result += (int)(out8di[i] & 0xFFFF);
        }
        
        mask = (mask << 1) | (mask >> 7);
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== V8DFmode (8x64-bit doubles) ========== */
/* Targets: gen_avx512f_blendmv8df */
static int test_v8df_blend(int argc) {
    int iterations = (argc > 7) ? 65 : 33;
    double result = 0.0;
    
    double a8df[8] __attribute__((aligned(64)));
    double b8df[8] __attribute__((aligned(64)));
    volatile double out8df[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a8df[i] = 1.0 + i * 0.1;
        b8df[i] = 2.0 + i * 0.2;
    }
    
    __m512d va = _mm512_load_pd(a8df);
    __m512d vb = _mm512_load_pd(b8df);
    
    /* Create mask using floating comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(1.5), _CMP_LT_OQ);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend doubles - should trigger gen_avx512f_blendmv8df */
        __m512d blended = _mm512_mask_blend_pd(mask, va, vb);
        
        /* Blend with arithmetic result */
        __m512d multiplied = _mm512_mul_pd(va, _mm512_set1_pd(1.5));
        blended = _mm512_mask_blend_pd(mask ^ 0x0F, blended, multiplied);
        
        _mm512_store_pd((void*)out8df, blended);
        
        for (int i = 0; i < 8; i++) {
            result += out8df[i];
        }
        
        /* Modify vectors slightly */
        va = _mm512_add_pd(va, _mm512_set1_pd(0.01));
        mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    }
    
    /* Convert to int for consistency */
    __asm__ volatile("" : : "r"(result) : "memory");
    return (int)result;
}

/* ========== V16SFmode (16x32-bit floats) ========== */
/* Targets: gen_avx512f_blendmv16sf */
static int test_v16sf_blend(int argc) {
    int iterations = (argc > 8) ? 85 : 43;
    float result = 0.0f;
    
    float a16sf[16] __attribute__((aligned(64)));
    float b16sf[16] __attribute__((aligned(64)));
    volatile float out16sf[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a16sf[i] = 0.5f + i * 0.1f;
        b16sf[i] = 1.0f + i * 0.15f;
    }
    
    __m512 va = _mm512_load_ps(a16sf);
    __m512 vb = _mm512_load_ps(b16sf);
    
    /* Create mask using floating comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(1.0f), _CMP_GT_OQ);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Blend floats - should trigger gen_avx512f_blendmv16sf */
        __m512 blended = _mm512_mask_blend_ps(mask, va, vb);
        
        /* Blend with arithmetic result */
        __m512 added = _mm512_add_ps(va, _mm512_set1_ps(0.5f));
        blended = _mm512_mask_blend_ps(mask ^ 0xAAAA, blended, added);
        
        _mm512_store_ps((void*)out16sf, blended);
        
        for (int i = 0; i < 16; i += 2) {
            result += out16sf[i];
        }
        
        /* Rotate mask and modify vectors */
        mask = (mask << 1) | (mask >> 15);
        va = _mm512_add_ps(va, _mm512_set1_ps(0.01f));
    }
    
    __asm__ volatile("" : : "r"(result) : "memory");
    return (int)result;
}

#endif /* __AVX512F__ */

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    int total_result = 0;
    
    printf("Testing AVX-512 blend intrinsics...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX512F and AVX512BW detected, running all tests...\n");
    
    /* Run all blend tests with varying control flow */
    total_result += test_v64qi_blend(argc);
    printf("  V64QI blend test completed\n");
    
    total_result += test_v32hi_blend(argc);
    printf("  V32HI blend test completed\n");
    
    total_result += test_v32hf_blend(argc);
    printf("  V32HF blend test completed\n");
    
    total_result += test_v32bf_blend(argc);
    printf("  V32BF blend test completed\n");
    
    total_result += test_v16si_blend(argc);
    printf("  V16SI blend test completed\n");
    
    total_result += test_v8di_blend(argc);
    printf("  V8DI blend test completed\n");
    
    total_result += test_v8df_blend(argc);
    printf("  V8DF blend test completed\n");
    
    total_result += test_v16sf_blend(argc);
    printf("  V16SF blend test completed\n");
    
#else
    printf("AVX512F detected but AVX512BW not available, running F-only tests...\n");
    
    total_result += test_v16si_blend(argc);
    printf("  V16SI blend test completed\n");
    
    total_result += test_v8di_blend(argc);
    printf("  V8DI blend test completed\n");
    
    total_result += test_v8df_blend(argc);
    printf("  V8DF blend test completed\n");
    
    total_result += test_v16sf_blend(argc);
    printf("  V16SF blend test completed\n");
#endif
#else
    printf("AVX-512 not supported on this platform\n");
    printf("Skipping blend tests - returning default value\n");
    total_result = 42; /* Default value when no tests run */
#endif
    
    printf("Total checksum: %d\n", total_result);
    
    /* Use result to affect return code */
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
