/* AVX-512 blend coverage test for i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function attributes for specific ISA requirements */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
void test_avx512f_blends(void* output) {
    volatile int idx = 0; /* Prevent constant propagation */
    
    /* V16SFmode: 16 single-precision floats */
    {
        float a[16] __attribute__((aligned(64)));
        float b[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            a[i] = i * 1.0f;
            b[i] = i * 2.0f;
        }
        
        __m512 va = _mm512_load_ps(a);
        __m512 vb = _mm512_load_ps(b);
        
        /* Dynamic mask based on array index parity */
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if ((i + idx) & 1) {
                mask |= (1 << i);
            }
        }
        
        __m512 result = _mm512_mask_blend_ps(mask, va, vb);
        _mm512_store_ps((float*)output + 0, result);
    }
    
    /* V8DFmode: 8 double-precision floats */
    {
        double a[8] __attribute__((aligned(64)));
        double b[8] __attribute__((aligned(64)));
        for (int i = 0; i < 8; i++) {
            a[i] = i * 1.0;
            b[i] = i * 3.0;
        }
        
        __m512d va = _mm512_load_pd(a);
        __m512d vb = _mm512_load_pd(b);
        
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            if ((i + idx) & 2) {
                mask |= (1 << i);
            }
        }
        
        __m512d result = _mm512_mask_blend_pd(mask, va, vb);
        _mm512_store_pd((double*)output + 8, result);
    }
    
    /* V16SImode: 16 32-bit integers */
    {
        int32_t a[16] __attribute__((aligned(64)));
        int32_t b[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            a[i] = i * 10;
            b[i] = i * 20;
        }
        
        __m512i va = _mm512_load_epi32(a);
        __m512i vb = _mm512_load_epi32(b);
        
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if ((i + idx) & 4) {
                mask |= (1 << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
        _mm512_store_epi32((int32_t*)output + 16, result);
    }
    
    /* V8DImode: 8 64-bit integers */
    {
        int64_t a[8] __attribute__((aligned(64)));
        int64_t b[8] __attribute__((aligned(64)));
        for (int i = 0; i < 8; i++) {
            a[i] = i * 100LL;
            b[i] = i * 200LL;
        }
        
        __m512i va = _mm512_load_epi64(a);
        __m512i vb = _mm512_load_epi64(b);
        
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            if ((i + idx) & 1) {
                mask |= (1 << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
        _mm512_store_epi64((int64_t*)output + 24, result);
    }
}
#endif

#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
void test_avx512bw_blends(void* output) {
    volatile int idx = 1; /* Prevent constant propagation */
    
    /* V64QImode: 64 8-bit integers */
    {
        int8_t a[64] __attribute__((aligned(64)));
        int8_t b[64] __attribute__((aligned(64)));
        for (int i = 0; i < 64; i++) {
            a[i] = i;
            b[i] = i * 2;
        }
        
        __m512i va = _mm512_load_si512(a);
        __m512i vb = _mm512_load_si512(b);
        
        /* Complex mask generation to prevent optimization */
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if (((i + idx) * 1103515245) & 0x4000) {
                mask |= (1ULL << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
        _mm512_store_si512((__m512i*)((int8_t*)output + 32), result);
    }
    
    /* V32HImode: 32 16-bit integers */
    {
        int16_t a[32] __attribute__((aligned(64)));
        int16_t b[32] __attribute__((aligned(64)));
        for (int i = 0; i < 32; i++) {
            a[i] = i * 5;
            b[i] = i * 15;
        }
        
        __m512i va = _mm512_load_si512(a);
        __m512i vb = _mm512_load_si512(b);
        
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if (((i + idx) * 1103515245) & 0x8000) {
                mask |= (1U << i);
            }
        }
        
        __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
        _mm512_store_si512((__m512i*)((int16_t*)output + 64), result);
    }
}
#endif

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
void test_avx512fp16_blends(void* output) {
    volatile int idx = 2; /* Prevent constant propagation */
    
    /* V32HFmode: 32 half-precision floats */
    {
        _Float16 a[32] __attribute__((aligned(64)));
        _Float16 b[32] __attribute__((aligned(64)));
        for (int i = 0; i < 32; i++) {
            a[i] = i * 0.5f;
            b[i] = i * 1.5f;
        }
        
        __m512h va = _mm512_load_ph(a);
        __m512h vb = _mm512_load_ph(b);
        
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i + idx) & 3) {
                mask |= (1U << i);
            }
        }
        
        __m512h result = _mm512_mask_blend_ph(mask, va, vb);
        _mm512_store_ph((_Float16*)output + 96, result);
    }
}
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
void test_avx512bf16_blends(void* output) {
    volatile int idx = 3; /* Prevent constant propagation */
    
    /* V32BFmode: 32 bfloat16 values */
    {
        /* Use __bf16 type for bfloat16 */
        __bf16 a[32] __attribute__((aligned(64)));
        __bf16 b[32] __attribute__((aligned(64)));
        
        /* Initialize with float values converted to bfloat16 */
        for (int i = 0; i < 32; i++) {
            float fa = i * 0.25f;
            float fb = i * 0.75f;
            /* Simple conversion: just take top 16 bits */
            uint32_t ia = *(uint32_t*)&fa;
            uint32_t ib = *(uint32_t*)&fb;
            a[i] = (__bf16)(ia >> 16);
            b[i] = (__bf16)(ib >> 16);
        }
        
        /* Load as __m512bh for bfloat16 */
        __m512bh va = _mm512_load_si512((const __m512bh*)a);
        __m512bh vb = _mm512_load_si512((const __m512bh*)b);
        
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i + idx) & 5) {
                mask |= (1U << i);
            }
        }
        
        /* Use the same intrinsic as FP16 for blending */
        __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
        _mm512_store_si512((__m512bh*)((__bf16*)output + 128), result);
    }
}
#endif

/* Main test driver */
int main() {
    /* Large enough buffer for all outputs */
    uint8_t output_buffer[1024] __attribute__((aligned(64))) = {0};
    
    /* Execute all blend tests */
#ifdef __AVX512F__
    test_avx512f_blends(output_buffer);
#endif
    
#ifdef __AVX512BW__
    test_avx512bw_blends(output_buffer);
#endif
    
#ifdef __AVX512FP16__
    test_avx512fp16_blends(output_buffer);
#endif
    
#ifdef __AVX512BF16__
    test_avx512bf16_blends(output_buffer);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (size_t i = 0; i < sizeof(output_buffer); i++) {
        checksum += output_buffer[i];
    }
    
    printf("Blend test checksum: %lu\n", (unsigned long)checksum);
    
    /* Return non-zero if any required ISA was missing */
#if !defined(__AVX512F__) || !defined(__AVX512BW__) || \
    !defined(__AVX512FP16__) || !defined(__AVX512BF16__)
    printf("Warning: Some AVX-512 extensions not available\n");
    return 1;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif
