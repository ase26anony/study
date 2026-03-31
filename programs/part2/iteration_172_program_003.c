/* AVX-512 Blend Coverage Test for i386-expand.cc */
/* Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function attributes to ensure proper code generation */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
#endif
static void test_v16si_v8di_v8df_v16sf(void) {
#ifdef __AVX512F__
    /* V16SImode: 16 x 32-bit integers */
    {
        int32_t src1[16] __attribute__((aligned(64))) = {0};
        int32_t src2[16] __attribute__((aligned(64))) = {0};
        int32_t dst[16] __attribute__((aligned(64))) = {0};
        
        /* Initialize with pattern */
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3;
        }
        
        __m512i v1 = _mm512_load_si512((const __m512i*)src1);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2);
        
        /* Dynamic mask based on element index parity */
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if (i % 2 == 0) mask |= (1 << i);
        }
        
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, result);
        
        /* Use result to prevent optimization */
        volatile int32_t sum = 0;
        for (int i = 0; i < 16; i++) sum += dst[i];
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        int64_t src1[8] __attribute__((aligned(64))) = {0};
        int64_t src2[8] __attribute__((aligned(64))) = {0};
        int64_t dst[8] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 5LL;
            src2[i] = i * 7LL;
        }
        
        __m512i v1 = _mm512_load_si512((const __m512i*)src1);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2);
        
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            mask |= (1 << i) & (0xAA); /* 0b10101010 pattern */
        }
        
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, result);
        
        volatile int64_t sum = 0;
        for (int i = 0; i < 8; i++) sum += dst[i];
    }
    
    /* V8DFmode: 8 x double precision floats */
    {
        double src1[8] __attribute__((aligned(64))) = {0};
        double src2[8] __attribute__((aligned(64))) = {0};
        double dst[8] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 1.5;
            src2[i] = i * 2.5;
        }
        
        __m512d v1 = _mm512_load_pd(src1);
        __m512d v2 = _mm512_load_pd(src2);
        
        __mmask8 mask = 0xCC; /* 0b11001100 pattern */
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        _mm512_store_pd(dst, result);
        
        volatile double sum = 0.0;
        for (int i = 0; i < 8; i++) sum += dst[i];
    }
    
    /* V16SFmode: 16 x single precision floats */
    {
        float src1[16] __attribute__((aligned(64))) = {0};
        float src2[16] __attribute__((aligned(64))) = {0};
        float dst[16] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 0.5f;
            src2[i] = i * 1.5f;
        }
        
        __m512 v1 = _mm512_load_ps(src1);
        __m512 v2 = _mm512_load_ps(src2);
        
        __mmask16 mask = 0xF0F0; /* 0b1111000011110000 pattern */
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        _mm512_store_ps(dst, result);
        
        volatile float sum = 0.0f;
        for (int i = 0; i < 16; i++) sum += dst[i];
    }
#endif
}

#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
#endif
static void test_v64qi_v32hi(void) {
#ifdef __AVX512BW__
    /* V64QImode: 64 x 8-bit integers */
    {
        int8_t src1[64] __attribute__((aligned(64))) = {0};
        int8_t src2[64] __attribute__((aligned(64))) = {0};
        int8_t dst[64] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 64; i++) {
            src1[i] = (i % 32) * 2;
            src2[i] = (i % 32) * 3;
        }
        
        __m512i v1 = _mm512_load_si512((const __m512i*)src1);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2);
        
        /* Complex dynamic mask generation */
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((i + (i >> 2)) % 3 == 0) {
                mask |= (1ULL << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 64; i++) sum += dst[i];
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        int16_t src1[32] __attribute__((aligned(64))) = {0};
        int16_t src2[32] __attribute__((aligned(64))) = {0};
        int16_t dst[32] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 10;
            src2[i] = i * 15;
        }
        
        __m512i v1 = _mm512_load_si512((const __m512i*)src1);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2);
        
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((i % 4) < 2) ? (1 << i) : 0;
        }
        
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 32; i++) sum += dst[i];
    }
#endif
}

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
#endif
static void test_v32hf(void) {
#ifdef __AVX512FP16__
    /* V32HFmode: 32 x half precision floats */
    {
        _Float16 src1[32] __attribute__((aligned(64))) = {0};
        _Float16 src2[32] __attribute__((aligned(64))) = {0};
        _Float16 dst[32] __attribute__((aligned(64))) = {0};
        
        for (int i = 0; i < 32; i++) {
            src1[i] = (_Float16)(i * 0.25f);
            src2[i] = (_Float16)(i * 0.75f);
        }
        
        __m512h v1 = _mm512_load_ph(src1);
        __m512h v2 = _mm512_load_ph(src2);
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_ph(dst, result);
        
        volatile _Float16 sum = 0.0f;
        for (int i = 0; i < 32; i++) sum += dst[i];
    }
#endif
}

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
#endif
static void test_v32bf(void) {
#ifdef __AVX512BF16__
    /* V32BFmode: 32 x bfloat16 */
    {
        __bf16 src1[32] __attribute__((aligned(64))) = {0};
        __bf16 src2[32] __attribute__((aligned(64))) = {0};
        __bf16 dst[32] __attribute__((aligned(64))) = {0};
        
        /* Initialize bfloat16 values */
        for (int i = 0; i < 32; i++) {
            float f1 = i * 0.125f;
            float f2 = i * 0.375f;
            memcpy(&src1[i], &f1, sizeof(__bf16));
            memcpy(&src2[i], &f2, sizeof(__bf16));
        }
        
        /* Load as __m512bh for bfloat16 */
        __m512bh v1 = _mm512_load_si512((const __m512i*)src1);
        __m512bh v2 = _mm512_load_si512((const __m512i*)src2);
        
        __mmask32 mask = 0x55555555; /* Alternating pattern, opposite of v32hf */
        __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, result);
        
        volatile int sum = 0;
        for (int i = 0; i < 32; i++) {
            float f;
            memcpy(&f, &dst[i], sizeof(__bf16));
            sum += (int)f;
        }
    }
#endif
}

/* Main test driver */
int main(void) {
    printf("Starting AVX-512 blend coverage test...\n");
    
    /* Test AVX512F modes */
#ifdef __AVX512F__
    printf("Testing V16SImode, V8DImode, V8DFmode, V16SFmode...\n");
    test_v16si_v8di_v8df_v16sf();
#endif
    
    /* Test AVX512BW modes */
#ifdef __AVX512BW__
    printf("Testing V64QImode, V32HImode...\n");
    test_v64qi_v32hi();
#endif
    
    /* Test AVX512-FP16 mode */
#ifdef __AVX512FP16__
    printf("Testing V32HFmode...\n");
    test_v32hf();
#endif
    
    /* Test AVX512-BF16 mode */
#ifdef __AVX512BF16__
    printf("Testing V32BFmode...\n");
    test_v32bf();
#endif
    
    printf("AVX-512 blend coverage test completed.\n");
    
    /* Force all functions to be called even if some extensions aren't available */
    /* This ensures the compiler sees all code paths during compilation */
    volatile int dummy = 0;
    if (dummy) {
        test_v16si_v8di_v8df_v16sf();
        test_v64qi_v32hi();
        test_v32hf();
        test_v32bf();
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
