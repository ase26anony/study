#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode: 64-byte integers ========== */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i % 128);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Use result in computation to prevent elimination
    __m512i sum_vec = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    uint64_t checksum = 0;
    char temp[64];
    _mm512_store_si512(temp, sum_vec);
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)temp[i];
    }
    
    return checksum;
}

/* ========== V32HImode: 32-halfword integers ========== */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(500 - i * 10);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    // Use result
    __m512i sum_vec = _mm512_add_epi16(result, _mm512_set1_epi16(1));
    uint64_t checksum = 0;
    short temp[32];
    _mm512_store_si512(temp, sum_vec);
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)temp[i];
    }
    
    return checksum;
}

/* ========== V32HFmode: 32-half-precision floats ========== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(1.0f - i * 0.03f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    // Use result
    __m512h sum_vec = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    uint64_t checksum = 0;
    _Float16 temp[32];
    _mm512_store_ph(temp, sum_vec);
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)(temp[i] * 1000);
    }
    
    return checksum;
}
#endif

/* ========== V32BFmode: 32-bfloat16 ========== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bfloat16 a[32] __attribute__((aligned(64)));
    __bfloat16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 0.25f);
        b[i] = bfloat16_from_float(2.0f - i * 0.05f);
    }
    
    // Load as integers for bfloat16 operations
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask (treat as 16-bit integers)
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    // Use result
    uint64_t checksum = 0;
    __bfloat16 temp[32];
    _mm512_store_si512(temp, result);
    for (int i = 0; i < 32; i++) {
        checksum += temp[i].__value;
    }
    
    return checksum;
}
#endif

/* ========== V16SImode: 16-dword integers ========== */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = 10000 - i * 500;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    // Use result
    __m512i sum_vec = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    uint64_t checksum = 0;
    int temp[16];
    _mm512_store_si512(temp, sum_vec);
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)temp[i];
    }
    
    return checksum;
}

/* ========== V8DImode: 8-qword integers ========== */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = 50000LL - i * 5000LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    // Use result
    __m512i sum_vec = _mm512_add_epi64(result, _mm512_set1_epi64(1));
    uint64_t checksum = 0;
    long long temp[8];
    _mm512_store_si512(temp, sum_vec);
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)temp[i];
    }
    
    return checksum;
}

/* ========== V8DFmode: 8-double-precision floats ========== */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 0.25;
        b[i] = 2.0 - i * 0.1;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    // Use result
    __m512d sum_vec = _mm512_add_pd(result, _mm512_set1_pd(0.5));
    uint64_t checksum = 0;
    double temp[8];
    _mm512_store_pd(temp, sum_vec);
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)(temp[i] * 1000);
    }
    
    return checksum;
}

/* ========== V16SFmode: 16-single-precision floats ========== */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.1f;
        b[i] = 1.5f - i * 0.05f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    // Use result
    __m512 sum_vec = _mm512_add_ps(result, _mm512_set1_ps(0.25f));
    uint64_t checksum = 0;
    float temp[16];
    _mm512_store_ps(temp, sum_vec);
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)(temp[i] * 1000);
    }
    
    return checksum;
}

/* ========== Mixed data types in loop ========== */
static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Test different blend operations in each iteration
        if (iter % 7 == 0) {
            // V64QImode
            char a[64], b[64];
            for (int i = 0; i < 64; i++) {
                a[i] = (char)((i + iter) % 128);
                b[i] = (char)(64 - (i + iter) % 128);
            }
            __m512i va = _mm512_load_si512((const __m512i*)a);
            __m512i vb = _mm512_load_si512((const __m512i*)b);
            __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
            __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
            
            char temp[64];
            _mm512_store_si512(temp, result);
            for (int i = 0; i < 64; i++) {
                total_checksum += (uint8_t)temp[i];
            }
        }
        else if (iter % 7 == 1) {
            // V32HImode
            short a[32], b[32];
            for (int i = 0; i < 32; i++) {
                a[i] = (short)((i + iter) * 10);
                b[i] = (short)(500 - (i + iter) * 5);
            }
            __m512i va = _mm512_load_si512((const __m512i*)a);
            __m512i vb = _mm512_load_si512((const __m512i*)b);
            __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
            __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
            
            short temp[32];
            _mm512_store_si512(temp, result);
            for (int i = 0; i < 32; i++) {
                total_checksum += (uint16_t)temp[i];
            }
        }
        else if (iter % 7 == 2) {
            // V16SFmode
            float a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = (i + iter) * 0.05f;
                b[i] = 1.0f - (i + iter) * 0.02f;
            }
            __m512 va = _mm512_load_ps(a);
            __m512 vb = _mm512_load_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, va, vb);
            
            float temp[16];
            _mm512_store_ps(temp, result);
            for (int i = 0; i < 16; i++) {
                total_checksum += (uint32_t)(temp[i] * 1000);
            }
        }
        else if (iter % 7 == 3) {
            // V8DFmode
            double a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = (i + iter) * 0.1;
                b[i] = 2.0 - (i + iter) * 0.05;
            }
            __m512d va = _mm512_load_pd(a);
            __m512d vb = _mm512_load_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, va, vb);
            
            double temp[8];
            _mm512_store_pd(temp, result);
            for (int i = 0; i < 8; i++) {
                total_checksum += (uint64_t)(temp[i] * 1000);
            }
        }
        else if (iter % 7 == 4) {
            // V16SImode
            int a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = (i + iter) * 100;
                b[i] = 1000 - (i + iter) * 50;
            }
            __m512i va = _mm512_load_si512((const __m512i*)a);
            __m512i vb = _mm512_load_si512((const __m512i*)b);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
            
            int temp[16];
            _mm512_store_si512(temp, result);
            for (int i = 0; i < 16; i++) {
                total_checksum += (uint32_t)temp[i];
            }
        }
        else if (iter % 7 == 5) {
            // V8DImode
            long long a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = (i + iter) * 1000LL;
                b[i] = 10000LL - (i + iter) * 500LL;
            }
            __m512i va = _mm512_load_si512((const __m512i*)a);
            __m512i vb = _mm512_load_si512((const __m512i*)b);
            __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
            __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
            
            long long temp[8];
            _mm512_store_si512(temp, result);
            for (int i = 0; i < 8; i++) {
                total_checksum += (uint64_t)temp[i];
            }
        }
    }
    
    return total_checksum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ========== Scalar fallback implementations ========== */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64], b[64];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i % 128);
        char result = (a[i] > b[i]) ? a[i] : b[i];
        checksum += (uint8_t)(result + 1);
    }
    
    return checksum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32], b[32];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(500 - i * 10);
        short result = (a[i] < b[i]) ? a[i] : b[i];
        checksum += (uint16_t)(result + 1);
    }
    
    return checksum;
}

static uint64_t scalar_test_v16si_blend(void) {
    int a[16], b[16];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = 10000 - i * 500;
        int result = (a[i] == b[i]) ? a[i] : b[i];
        checksum += (uint32_t)(result + 1);
    }
    
    return checksum;
}

static uint64_t scalar_test_v8di_blend(void) {
    long long a[8], b[8];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = 50000LL - i * 5000LL;
        long long result = (a[i] >= b[i]) ? a[i] : b[i];
        checksum += (uint64_t)(result + 1);
    }
    
    return checksum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float a[16], b[16];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.1f;
        b[i] = 1.5f - i * 0.05f;
        float result = (a[i] > b[i]) ? a[i] : b[i];
        checksum += (uint32_t)((result + 0.25f) * 1000);
    }
    
    return checksum;
}

static uint64_t scalar_test_v8df_blend(void) {
    double a[8], b[8];
    uint64_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 0.25;
        b[i] = 2.0 - i * 0.1;
        double result = (a[i] < b[i]) ? a[i] : b[i];
        checksum += (uint64_t)((result + 0.5) * 1000);
    }
    
    return checksum;
}

static uint64_t scalar_mixed_blend_loop(int iterations) {
    uint64_t total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        if (iter % 7 == 0) {
            // V64QImode scalar
            for (int i = 0; i < 64; i++) {
                char a = (char)((i + iter) % 128);
                char b = (char)(64 - (i + iter) % 128);
                char result = (a > b) ? a : b;
                total_checksum += (uint8_t)result;
            }
        }
        else if (iter % 7 == 1) {
            // V32HImode scalar
            for (int i = 0; i < 32; i++) {
                short a = (short)((i + iter) * 10);
                short b = (short)(500 - (i + iter) * 5);
                short result = (a < b) ? a : b;
                total_checksum += (uint16_t)result;
            }
        }
        else if (iter % 7 == 2) {
            // V16SFmode scalar
            for (int i = 0; i < 16; i++) {
                float a = (i + iter) * 0.05f;
                float b = 1.0f - (i + iter) * 0.02f;
                float result = (a > b) ? a : b;
                total_checksum += (uint32_t)(result * 1000);
            }
        }
        else if (iter % 7 == 3) {
            // V8DFmode scalar
            for (int i = 0; i < 8; i++) {
                double a = (i + iter) * 0.1;
                double b = 2.0 - (i + iter) * 0.05;
                double result = (a < b) ? a : b;
                total_checksum += (uint64_t)(result * 1000);
            }
        }
        else if (iter % 7 == 4) {
            // V16SImode scalar
            for (int i = 0; i < 16; i++) {
                int a = (i + iter) * 100;
                int b = 1000 - (i + iter) * 50;
                int result = (a == b) ? a : b;
                total_checksum += (uint32_t)result;
            }
        }
        else if (iter % 7 == 5) {
            // V8DImode scalar
            for (int i = 0; i < 8; i++) {
                long long a = (i + iter) * 1000LL;
                long long b = 10000LL - (i + iter) * 500LL;
                long long result = (a >= b) ? a : b;
                total_checksum += (uint64_t)result;
            }
        }
    }
    
    return total_checksum;
}

/* ========== Main driver ========== */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized blend operations.\n");
    
    // Test each vector mode individually
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend checksum added\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend checksum added\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode blend checksum added\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode blend checksum added\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend checksum added\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode blend checksum added\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode blend checksum added\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode blend checksum added\n");
    
    // Test mixed modes in loop
    total_checksum += test_mixed_blend_loop(10);
    printf("Mixed mode blend loop checksum added\n");
    
#else
    printf("AVX-512BW not available. Using scalar fallback.\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512 not available. Using scalar fallback.\n");
    goto scalar_fallback;
#endif

    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);

scalar_fallback:
    // Scalar fallback implementations
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
    total_checksum += scalar_test_v16sf_blend();
    total_checksum += scalar_test_v8df_blend();
    total_checksum += scalar_mixed_blend_loop(10);
    
    printf("Scalar fallback total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
