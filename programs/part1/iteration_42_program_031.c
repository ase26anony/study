#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // Blend operation for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Store and compute checksum
    char res[64] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)res[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // Blend operation for V32HImode
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    short res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)res[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // Blend operation for V32HFmode
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res[32] __attribute__((aligned(64)));
    _mm512_store_ph(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 (emulated using epi16) */
static float test_v32bf_blend(void) {
    uint16_t a[32] __attribute__((aligned(64)));
    uint16_t b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i * 0x4000);  // bfloat16 pattern
        b[i] = (uint16_t)(i * 0x4040);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // Blend operation for V32BFmode (using epi16 as bfloat16 is stored as 16-bit)
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    uint16_t res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // Blend operation for V16SImode
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int res[16] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)res[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
    
    // Blend operation for V8DImode
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long res[8] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate dynamic mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // Blend operation for V8DFmode
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    double res[8] __attribute__((aligned(64)));
    _mm512_store_pd(res, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate dynamic mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_NEQ_OQ);
    
    // Blend operation for V16SFmode
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res[16] __attribute__((aligned(64)));
    _mm512_store_ps(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Process different data types in each iteration
        if (iter % 7 == 0) {
            // V64QImode
            char a[64] __attribute__((aligned(64)));
            char b[64] __attribute__((aligned(64)));
            for (int i = 0; i < 64; i++) {
                a[i] = (char)((i + iter) & 0xFF);
                b[i] = (char)((i * 3 + iter) & 0xFF);
            }
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
            char res[64] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            for (int i = 0; i < 64; i++) total_sum += (uint8_t)res[i];
        }
        else if (iter % 7 == 1) {
            // V32HImode
            short a[32] __attribute__((aligned(64)));
            short b[32] __attribute__((aligned(64)));
            for (int i = 0; i < 32; i++) {
                a[i] = (short)((i + iter) & 0xFFFF);
                b[i] = (short)((i * 5 + iter) & 0xFFFF);
            }
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
            __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
            short res[32] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            for (int i = 0; i < 32; i++) total_sum += (uint16_t)res[i];
        }
        else if (iter % 7 == 2) {
            // V16SImode
            int a[16] __attribute__((aligned(64)));
            int b[16] __attribute__((aligned(64)));
            for (int i = 0; i < 16; i++) {
                a[i] = i + iter;
                b[i] = i * 7 + iter;
            }
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
            int res[16] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            for (int i = 0; i < 16; i++) total_sum += (uint32_t)res[i];
        }
        else if (iter % 7 == 3) {
            // V8DImode
            long long a[8] __attribute__((aligned(64)));
            long long b[8] __attribute__((aligned(64)));
            for (int i = 0; i < 8; i++) {
                a[i] = i * 11LL + iter;
                b[i] = i * 13LL + iter;
            }
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
            __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
            long long res[8] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            for (int i = 0; i < 8; i++) total_sum += (uint64_t)res[i];
        }
        else if (iter % 7 == 4) {
            // V16SFmode
            float a[16] __attribute__((aligned(64)));
            float b[16] __attribute__((aligned(64)));
            for (int i = 0; i < 16; i++) {
                a[i] = (i + iter) * 0.25f;
                b[i] = (i * 3 + iter) * 0.125f;
            }
            __m512 va = _mm512_load_ps(a);
            __m512 vb = _mm512_load_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, va, vb);
            float res[16] __attribute__((aligned(64)));
            _mm512_store_ps(res, result);
            for (int i = 0; i < 16; i++) total_sum += (uint32_t)(res[i] * 1000);
        }
        else if (iter % 7 == 5) {
            // V8DFmode
            double a[8] __attribute__((aligned(64)));
            double b[8] __attribute__((aligned(64)));
            for (int i = 0; i < 8; i++) {
                a[i] = (i + iter) * 0.5;
                b[i] = (i * 2 + iter) * 0.25;
            }
            __m512d va = _mm512_load_pd(a);
            __m512d vb = _mm512_load_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, va, vb);
            double res[8] __attribute__((aligned(64)));
            _mm512_store_pd(res, result);
            for (int i = 0; i < 8; i++) total_sum += (uint64_t)(res[i] * 1000);
        }
        else if (iter % 7 == 6) {
            // V32BFmode (emulated)
            uint16_t a[32] __attribute__((aligned(64)));
            uint16_t b[32] __attribute__((aligned(64)));
            for (int i = 0; i < 32; i++) {
                a[i] = (uint16_t)((i + iter) * 0x4000);
                b[i] = (uint16_t)((i * 2 + iter) * 0x4040);
            }
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
            __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
            uint16_t res[32] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            for (int i = 0; i < 32; i++) total_sum += res[i];
        }
    }
    
    return total_sum;
}

#else  /* Scalar fallback implementations without AVX-512BW */

static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    char res[64];
    for (int i = 0; i < 64; i++) {
        res[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)res[i];
    }
    return sum;
}

static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    short res[32];
    for (int i = 0; i < 32; i++) {
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)res[i];
    }
    return sum;
}

static float test_v32bf_blend(void) {
    uint16_t a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (uint16_t)(i * 0x4000);
        b[i] = (uint16_t)(i * 0x4040);
    }
    
    uint16_t res[32];
    for (int i = 0; i < 32; i++) {
        res[i] = (a[i] != b[i]) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    int res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (a[i] == b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)res[i];
    }
    return sum;
}

static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    long long res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res[i];
    }
    return sum;
}

static double test_v8df_blend(void) {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    double res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

static float test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
    }
    
    float res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (a[i] != b[i]) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 7 == 0) {
            char a[64], b[64];
            for (int i = 0; i < 64; i++) {
                a[i] = (char)((i + iter) & 0xFF);
                b[i] = (char)((i * 3 + iter) & 0xFF);
            }
            char res[64];
            for (int i = 0; i < 64; i++) {
                res[i] = (a[i] > b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 64; i++) total_sum += (uint8_t)res[i];
        }
        else if (iter % 7 == 1) {
            short a[32], b[32];
            for (int i = 0; i < 32; i++) {
                a[i] = (short)((i + iter) & 0xFFFF);
                b[i] = (short)((i * 5 + iter) & 0xFFFF);
            }
            short res[32];
            for (int i = 0; i < 32; i++) {
                res[i] = (a[i] < b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 32; i++) total_sum += (uint16_t)res[i];
        }
        else if (iter % 7 == 2) {
            int a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = i + iter;
                b[i] = i * 7 + iter;
            }
            int res[16];
            for (int i = 0; i < 16; i++) {
                res[i] = (a[i] == b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 16; i++) total_sum += (uint32_t)res[i];
        }
        else if (iter % 7 == 3) {
            long long a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = i * 11LL + iter;
                b[i] = i * 13LL + iter;
            }
            long long res[8];
            for (int i = 0; i < 8; i++) {
                res[i] = (a[i] >= b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 8; i++) total_sum += (uint64_t)res[i];
        }
        else if (iter % 7 == 4) {
            float a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = (i + iter) * 0.25f;
                b[i] = (i * 3 + iter) * 0.125f;
            }
            float res[16];
            for (int i = 0; i < 16; i++) {
                res[i] = (a[i] > b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 16; i++) total_sum += (uint32_t)(res[i] * 1000);
        }
        else if (iter % 7 == 5) {
            double a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = (i + iter) * 0.5;
                b[i] = (i * 2 + iter) * 0.25;
            }
            double res[8];
            for (int i = 0; i < 8; i++) {
                res[i] = (a[i] < b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 8; i++) total_sum += (uint64_t)(res[i] * 1000);
        }
        else if (iter % 7 == 6) {
            uint16_t a[32], b[32];
            for (int i = 0; i < 32; i++) {
                a[i] = (uint16_t)((i + iter) * 0x4000);
                b[i] = (uint16_t)((i * 2 + iter) * 0x4040);
            }
            uint16_t res[32];
            for (int i = 0; i < 32; i++) {
                res[i] = (a[i] != b[i]) ? a[i] : b[i];
            }
            for (int i = 0; i < 32; i++) total_sum += res[i];
        }
    }
    
    return total_sum;
}

#endif  /* __AVX512BW__ */
#endif  /* __AVX512F__ */

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512 support detected\n");
#ifdef __AVX512BW__
    printf("AVX-512BW support detected\n");
    
    // Test all vector modes
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    float hf_result = test_v32hf_blend();
    total_checksum += (uint64_t)(hf_result * 1000);
    printf("V32HFmode blend test completed: %f\n", hf_result);
#endif
    
    float bf_result = test_v32bf_blend();
    total_checksum += (uint64_t)(bf_result * 1000);
    printf("V32BFmode blend test completed: %f\n", bf_result);
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode blend test completed\n");
    
    double df_result = test_v8df_blend();
    total_checksum += (uint64_t)(df_result * 1000);
    printf("V8DFmode blend test completed: %f\n", df_result);
    
    float sf_result = test_v16sf_blend();
    total_checksum += (uint64_t)(sf_result * 1000);
    printf("V16SFmode blend test completed: %f\n", sf_result);
    
    // Test mixed operations in loop
    uint64_t mixed_result = test_mixed_blend_loop(21);  // 3 iterations of each type
    total_checksum += mixed_result;
    printf("Mixed blend loop test completed: %lu\n", mixed_result);
    
#else
    printf("AVX-512BW not available, using scalar fallback\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += (uint64_t)(test_v32bf_blend() * 1000);
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (uint64_t)(test_v8df_blend() * 1000);
    total_checksum += (uint64_t)(test_v16sf_blend() * 1000);
    total_checksum += test_mixed_blend_loop(21);
#endif
#else
    printf("AVX-512 not available, using scalar fallback\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += (uint64_t)(test_v32bf_blend() * 1000);
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (uint64_t)(test_v8df_blend() * 1000);
    total_checksum += (uint64_t)(test_v16sf_blend() * 1000);
    total_checksum += test_mixed_blend_loop(21);
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
