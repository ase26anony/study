/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function attributes to ensure specific ISA requirements */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
#endif
static void test_v16si_v8di_v8df_v16sf(void) {
#ifdef __AVX512F__
    /* V16SImode: 16 x 32-bit integers */
    {
        int32_t src1[16] __attribute__((aligned(64)));
        int32_t src2[16] __attribute__((aligned(64)));
        int32_t dst[16] __attribute__((aligned(64)));
        
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3 + 1;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        /* Create dynamic mask based on element parity */
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if ((i % 3) == 0) {
                mask |= (1 << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_store_si512(dst, result);
        
        /* Use result to prevent optimization */
        volatile int32_t sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += dst[i];
        }
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        int64_t src1[8] __attribute__((aligned(64)));
        int64_t src2[8] __attribute__((aligned(64)));
        int64_t dst[8] __attribute__((aligned(64)));
        
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 5LL;
            src2[i] = i * 7LL + 2;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        /* Dynamic mask based on runtime calculation */
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            if ((src1[i] % 4) == 0) {
                mask |= (1 << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_store_si512(dst, result);
        
        volatile int64_t sum = 0;
        for (int i = 0; i < 8; i++) {
            sum += dst[i];
        }
    }
    
    /* V8DFmode: 8 x double precision floats */
    {
        double src1[8] __attribute__((aligned(64)));
        double src2[8] __attribute__((aligned(64)));
        double dst[8] __attribute__((aligned(64)));
        
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 1.5;
            src2[i] = i * 2.5 + 0.5;
        }
        
        __m512d v1 = _mm512_load_pd(src1);
        __m512d v2 = _mm512_load_pd(src2);
        
        /* Create mask using comparison */
        __m512d cmp = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        __m512d result = _mm512_mask_blend_pd(cmp, v1, v2);
        _mm512_store_pd(dst, result);
        
        volatile double sum = 0.0;
        for (int i = 0; i < 8; i++) {
            sum += dst[i];
        }
    }
    
    /* V16SFmode: 16 x single precision floats */
    {
        float src1[16] __attribute__((aligned(64)));
        float src2[16] __attribute__((aligned(64)));
        float dst[16] __attribute__((aligned(64)));
        
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 0.5f;
            src2[i] = i * 0.75f + 0.25f;
        }
        
        __m512 v1 = _mm512_load_ps(src1);
        __m512 v2 = _mm512_load_ps(src2);
        
        /* Dynamic mask using comparison */
        __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_LT_OQ);
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_store_ps(dst, result);
        
        volatile float sum = 0.0f;
        for (int i = 0; i < 16; i++) {
            sum += dst[i];
        }
    }
#endif /* __AVX512F__ */
}

#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
#endif
static void test_v64qi_v32hi(void) {
#ifdef __AVX512BW__
    /* V64QImode: 64 x 8-bit integers */
    {
        int8_t src1[64] __attribute__((aligned(64)));
        int8_t src2[64] __attribute__((aligned(64)));
        int8_t dst[64] __attribute__((aligned(64)));
        
        for (int i = 0; i < 64; i++) {
            src1[i] = (i % 32) - 16;
            src2[i] = (i % 16) - 8;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        /* Complex dynamic mask generation */
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((src1[i] > 0) ^ (src2[i] > 0)) {
                mask |= (1ULL << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_store_si512(dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += dst[i];
        }
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        int16_t src1[32] __attribute__((aligned(64)));
        int16_t src2[32] __attribute__((aligned(64)));
        int16_t dst[32] __attribute__((aligned(64)));
        
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 100;
            src2[i] = i * 150 + 50;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        /* Mask based on comparison result */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, v2);
        mask = ~mask;  /* Invert to make it non-constant */
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_store_si512(dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += dst[i];
        }
    }
#endif /* __AVX512BW__ */
}

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
#endif
static void test_v32hf(void) {
#ifdef __AVX512FP16__
    /* V32HFmode: 32 x half precision floats */
    {
        _Float16 src1[32] __attribute__((aligned(64)));
        _Float16 src2[32] __attribute__((aligned(64)));
        _Float16 dst[32] __attribute__((aligned(64)));
        
        for (int i = 0; i < 32; i++) {
            src1[i] = (_Float16)(i * 0.25f);
            src2[i] = (_Float16)(i * 0.375f + 0.125f);
        }
        
        __m512h v1 = _mm512_load_ph(src1);
        __m512h v2 = _mm512_load_ph(src2);
        
        /* Create mask using half-precision comparison */
        __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_ph(dst, result);
        
        volatile _Float16 sum = (_Float16)0.0;
        for (int i = 0; i < 32; i++) {
            sum += dst[i];
        }
    }
#endif /* __AVX512FP16__ */
}

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
#endif
static void test_v32bf(void) {
#ifdef __AVX512BF16__
    /* V32BFmode: 32 x bfloat16 */
    {
        __bf16 src1[32] __attribute__((aligned(64)));
        __bf16 src2[32] __attribute__((aligned(64)));
        __bf16 dst[32] __attribute__((aligned(64)));
        
        /* Initialize with simple pattern */
        for (int i = 0; i < 32; i++) {
            /* Simple values that fit in bfloat16 */
            float f1 = (i % 8) * 0.5f;
            float f2 = (i % 4) * 1.0f + 0.25f;
            
            /* Convert to bfloat16 */
            uint32_t i1 = *(uint32_t*)&f1;
            uint32_t i2 = *(uint32_t*)&f2;
            
            src1[i] = (__bf16)(i1 >> 16);
            src2[i] = (__bf16)(i2 >> 16);
        }
        
        /* Load as __m512bh for bfloat16 operations */
        __m512bh v1 = _mm512_load_si512(src1);
        __m512bh v2 = _mm512_load_si512(src2);
        
        /* Create mask - use comparison if available, otherwise pattern */
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i % 3) == 0) {
                mask |= (1 << i);
            }
        }
        
        /* Use generic blend for bfloat16 - same intrinsic as half precision */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_si512(dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += (int)dst[i];
        }
    }
#endif /* __AVX512BF16__ */
}

/* Main test driver */
int main(void) {
    printf("Testing AVX-512 blend instruction expansion coverage...\n");
    
    /* Test each group of vector modes */
    test_v16si_v8di_v8df_v16sf();
    printf("  - V16SI, V8DI, V8DF, V16SF modes tested\n");
    
    test_v64qi_v32hi();
    printf("  - V64QI, V32HI modes tested\n");
    
#ifdef __AVX512FP16__
    test_v32hf();
    printf("  - V32HF mode tested\n");
#else
    printf("  - V32HF mode skipped (AVX512-FP16 not available)\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bf();
    printf("  - V32BF mode tested\n");
#else
    printf("  - V32BF mode skipped (AVX512-BF16 not available)\n");
#endif
    
    printf("All AVX-512 blend tests completed.\n");
    return 0;
}

#ifdef __cplusplus
}
#endif
