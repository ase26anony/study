/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
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

#ifdef __AVX512FP16__
#define HAS_AVX512FP16 1
#else
#define HAS_AVX512FP16 0
#endif

#ifdef __AVX512BF16__
#define HAS_AVX512BF16 1
#else
#define HAS_AVX512BF16 0
#endif

/* Function attributes for specific ISA requirements */
#if HAS_AVX512F && HAS_AVX512BW
__attribute__((target("avx512f,avx512bw")))
static uint64_t test_v64qi_v32hi(void) {
    uint64_t checksum = 0;
    
    /* V64QImode: 64 x 8-bit integers */
    {
        __m512i a = _mm512_set1_epi8(0x11);
        __m512i b = _mm512_set1_epi8(0x22);
        
        /* Dynamic mask based on position: even positions select a, odd select b */
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((i & 1) == 0) {
                mask |= (1ULL << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, b, a);
        
        /* Extract and sum to prevent optimization */
        uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 64; i++) {
            checksum += temp[i];
        }
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        __m512i a = _mm512_set1_epi16(0x1111);
        __m512i b = _mm512_set1_epi16(0x2222);
        
        /* Dynamic mask: select a for positions divisible by 3 */
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i % 3) == 0) {
                mask |= (1U << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi16(mask, b, a);
        
        uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 32; i++) {
            checksum += temp[i];
        }
    }
    
    return checksum;
}
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static uint64_t test_v16si_v8di_v8df_v16sf(void) {
    uint64_t checksum = 0;
    
    /* V16SImode: 16 x 32-bit integers */
    {
        __m512i a = _mm512_set1_epi32(0x11111111);
        __m512i b = _mm512_set1_epi32(0x22222222);
        
        /* Dynamic mask using comparison */
        __m512i cmp_a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i cmp_b = _mm512_set1_epi32(7);
        __mmask16 mask = _mm512_cmpgt_epi32_mask(cmp_a, cmp_b);
        
        __m512i result = _mm512_mask_blend_epi32(mask, b, a);
        
        uint32_t temp[16];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 16; i++) {
            checksum += temp[i];
        }
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        __m512i a = _mm512_set1_epi64(0x1111111111111111ULL);
        __m512i b = _mm512_set1_epi64(0x2222222222222222ULL);
        
        /* Dynamic mask: alternating pattern */
        __mmask8 mask = 0xAA; /* 0b10101010 */
        
        __m512i result = _mm512_mask_blend_epi64(mask, b, a);
        
        uint64_t temp[8];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 8; i++) {
            checksum += temp[i];
        }
    }
    
    /* V8DFmode: 8 x double precision floats */
    {
        __m512d a = _mm512_set1_pd(1.111);
        __m512d b = _mm512_set1_pd(2.222);
        
        /* Dynamic mask using comparison */
        __m512d cmp_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d cmp_b = _mm512_set1_pd(3.5);
        __mmask8 mask = _mm512_cmp_pd_mask(cmp_a, cmp_b, _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, b, a);
        
        double temp[8];
        _mm512_storeu_pd(temp, result);
        for (int i = 0; i < 8; i++) {
            checksum += (uint64_t)(temp[i] * 1000);
        }
    }
    
    /* V16SFmode: 16 x single precision floats */
    {
        __m512 a = _mm512_set1_ps(1.111f);
        __m512 b = _mm512_set1_ps(2.222f);
        
        /* Dynamic mask using comparison */
        __m512 cmp_a = _mm512_set_ps(15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 
                                      9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 
                                      3.0f, 2.0f, 1.0f, 0.0f);
        __m512 cmp_b = _mm512_set1_ps(7.5f);
        __mmask16 mask = _mm512_cmp_ps_mask(cmp_a, cmp_b, _CMP_GT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, b, a);
        
        float temp[16];
        _mm512_storeu_ps(temp, result);
        for (int i = 0; i < 16; i++) {
            checksum += (uint64_t)(temp[i] * 1000);
        }
    }
    
    return checksum;
}
#endif

#if HAS_AVX512FP16
__attribute__((target("avx512f,avx512fp16")))
static uint64_t test_v32hf(void) {
    uint64_t checksum = 0;
    
    /* V32HFmode: 32 x half precision floats */
    {
        /* Initialize with pattern */
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(1.0f + i * 0.1f);
            b_data[i] = (_Float16)(2.0f + i * 0.1f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        /* Dynamic mask: select a where position is even */
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i & 1) == 0) {
                mask |= (1U << i);
            }
        }
        
        __m512h result = _mm512_mask_blend_ph(mask, b, a);
        
        _Float16 temp[32];
        _mm512_storeu_ph(temp, result);
        for (int i = 0; i < 32; i++) {
            checksum += (uint64_t)(temp[i] * 100);
        }
    }
    
    return checksum;
}
#endif

#if HAS_AVX512BF16
__attribute__((target("avx512f,avx512bf16")))
static uint64_t test_v32bf(void) {
    uint64_t checksum = 0;
    
    /* V32BFmode: 32 x bfloat16 */
    {
        /* Use __m512bh for bfloat16 vectors */
        __m512bh a, b;
        
        /* Initialize with pattern */
        uint16_t a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            /* Simple bfloat16 pattern */
            a_data[i] = (0x3F80 + i); /* ~1.0 + small increment */
            b_data[i] = (0x4000 + i); /* ~2.0 + small increment */
        }
        
        a = _mm512_loadu_si512((void*)a_data);
        b = _mm512_loadu_si512((void*)b_data);
        
        /* Dynamic mask: pattern based on position */
        __mmask32 mask = 0x55555555; /* Alternating 0101... */
        
        /* Use integer blend for bfloat16 since there's no direct bfloat16 blend */
        __m512bh result = _mm512_mask_blend_epi16(mask, 
            (__m512i)b, (__m512i)a);
        
        uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)result);
        for (int i = 0; i < 32; i++) {
            checksum += temp[i];
        }
    }
    
    return checksum;
}
#endif

int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
#if HAS_AVX512F && HAS_AVX512BW
    printf("Testing V64QImode and V32HImode...\n");
    total_checksum += test_v64qi_v32hi();
    printf("  V64QI/V32HI checksum added\n");
#endif
    
#if HAS_AVX512F
    printf("Testing V16SImode, V8DImode, V8DFmode, V16SFmode...\n");
    total_checksum += test_v16si_v8di_v8df_v16sf();
    printf("  V16SI/V8DI/V8DF/V16SF checksum added\n");
#endif
    
#if HAS_AVX512FP16
    printf("Testing V32HFmode...\n");
    total_checksum += test_v32hf();
    printf("  V32HF checksum added\n");
#endif
    
#if HAS_AVX512BF16
    printf("Testing V32BFmode...\n");
    total_checksum += test_v32bf();
    printf("  V32BF checksum added\n");
#endif
    
    printf("\nTotal checksum: %lu\n", total_checksum);
    printf("Test completed successfully!\n");
    
    return 0;
}
