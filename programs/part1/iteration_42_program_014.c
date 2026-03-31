#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode (64-byte integers) ========== */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3 + 1);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)result[i];
    }
    
    return checksum;
}

/* ========== V32HImode (32-halfword integers) ========== */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15 + 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    
    return checksum;
}

/* ========== V32HFmode (32-half-precision floats) ========== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.0f + 0.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)*(uint16_t*)&result[i];
    }
    
    return checksum;
}
#endif

/* ========== V32BFmode (32-bfloat16) ========== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        float temp_a = i * 1.25f;
        float temp_b = i * 1.75f + 0.25f;
        a[i] = _mm_cvtness_sbh(temp_a);
        b[i] = _mm_cvtness_sbh(temp_b);
    }
    
    // Load as 16-bit integers for blend operation
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask - compare as 16-bit integers
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512bw_blendmv32bf
    // Use epi16 blend for bfloat16 (same size)
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)*(uint16_t*)&result[i];
    }
    
    return checksum;
}
#endif

/* ========== V16SImode (16-dword integers) ========== */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150 + 50;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)result[i];
    }
    
    return checksum;
}

/* ========== V8DImode (8-qword integers) ========== */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL + 500LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

/* ========== V8DFmode (8-double-precision floats) ========== */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 1.75 + 0.25;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)*(uint64_t*)&result[i];
    }
    
    return checksum;
}

/* ========== V16SFmode (16-single-precision floats) ========== */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f + 0.25f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)*(uint32_t*)&result[i];
    }
    
    return checksum;
}

/* ========== Mixed data types in loop structure ========== */
static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_checksum = 0;
    
    // Process arrays with different data types
    for (int iter = 0; iter < iterations; iter++) {
        // Float blend
        {
            float fa[16] __attribute__((aligned(64)));
            float fb[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                fa[i] = (float)(iter * 16 + i) * 0.1f;
                fb[i] = (float)(iter * 16 + i) * 0.2f + 0.05f;
            }
            
            __m512 fva = _mm512_load_ps(fa);
            __m512 fvb = _mm512_load_ps(fb);
            __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_GT_OQ);
            __m512 fresult = _mm512_mask_blend_ps(fmask, fva, fvb);
            
            float fres[16];
            _mm512_store_ps(fres, fresult);
            
            for (int i = 0; i < 16; i++) {
                total_checksum += (uint32_t)*(uint32_t*)&fres[i];
            }
        }
        
        // Double blend
        {
            double da[8] __attribute__((aligned(64)));
            double db[8] __attribute__((aligned(64)));
            
            for (int i = 0; i < 8; i++) {
                da[i] = (double)(iter * 8 + i) * 0.15;
                db[i] = (double)(iter * 8 + i) * 0.25 + 0.075;
            }
            
            __m512d dva = _mm512_load_pd(da);
            __m512d dvb = _mm512_load_pd(db);
            __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_LT_OQ);
            __m512d dresult = _mm512_mask_blend_pd(dmask, dva, dvb);
            
            double dres[8];
            _mm512_store_pd(dres, dresult);
            
            for (int i = 0; i < 8; i++) {
                total_checksum += (uint64_t)*(uint64_t*)&dres[i];
            }
        }
        
        // Integer blend (32-bit)
        {
            int ia[16] __attribute__((aligned(64)));
            int ib[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                ia[i] = iter * 16 + i;
                ib[i] = iter * 16 + i + 1000;
            }
            
            __m512i iva = _mm512_load_si512((const __m512i*)ia);
            __m512i ivb = _mm512_load_si512((const __m512i*)ib);
            __mmask16 imask = _mm512_cmp_epi32_mask(iva, ivb, _MM_CMPINT_NE);
            __m512i iresult = _mm512_mask_blend_epi32(imask, iva, ivb);
            
            int ires[16];
            _mm512_store_si512((__m512i*)ires, iresult);
            
            for (int i = 0; i < 16; i++) {
                total_checksum += (uint32_t)ires[i];
            }
        }
    }
    
    return total_checksum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ========== Scalar fallback implementations ========== */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64];
    char b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3 + 1);
        // Simulate blend with mask (alternating pattern)
        char mask = (i % 2) ? 0xFF : 0x00;
        result[i] = (mask & b[i]) | (~mask & a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32];
    short b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15 + 5);
        short mask = (i % 3 == 0) ? 0xFFFF : 0x0000;
        result[i] = (mask & b[i]) | (~mask & a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v16si_blend(void) {
    int a[16];
    int b[16];
    int result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150 + 50;
        int mask = (i % 4 == 0) ? 0xFFFFFFFF : 0x00000000;
        result[i] = (mask & b[i]) | (~mask & a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v8di_blend(void) {
    long long a[8];
    long long b[8];
    long long result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1000LL;
        b[i] = i * 1500LL + 500LL;
        long long mask = (i % 2 == 0) ? 0xFFFFFFFFFFFFFFFFLL : 0x0000000000000000LL;
        result[i] = (mask & b[i]) | (~mask & a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float a[16];
    float b[16];
    float result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f + 0.25f;
        int mask = (i % 3 == 0) ? 0xFFFFFFFF : 0x00000000;
        result[i] = (*(int*)&mask & *(int*)&b[i]) | (~*(int*)&mask & *(int*)&a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)*(uint32_t*)&result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_v8df_blend(void) {
    double a[8];
    double b[8];
    double result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.25;
        b[i] = i * 1.75 + 0.25;
        long long mask = (i % 2 == 0) ? 0xFFFFFFFFFFFFFFFFLL : 0x0000000000000000LL;
        result[i] = (*(long long*)&mask & *(long long*)&b[i]) | (~*(long long*)&mask & *(long long*)&a[i]);
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)*(uint64_t*)&result[i];
    }
    
    return checksum;
}

static uint64_t scalar_test_mixed_blend_loop(int iterations) {
    uint64_t total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Float blend
        {
            float fa[16];
            float fb[16];
            float fres[16];
            
            for (int i = 0; i < 16; i++) {
                fa[i] = (float)(iter * 16 + i) * 0.1f;
                fb[i] = (float)(iter * 16 + i) * 0.2f + 0.05f;
                int mask = ((iter * 16 + i) % 5 == 0) ? 0xFFFFFFFF : 0x00000000;
                fres[i] = (mask & *(int*)&fb[i]) | (~mask & *(int*)&fa[i]);
                total_checksum += (uint32_t)*(uint32_t*)&fres[i];
            }
        }
        
        // Double blend
        {
            double da[8];
            double db[8];
            double dres[8];
            
            for (int i = 0; i < 8; i++) {
                da[i] = (double)(iter * 8 + i) * 0.15;
                db[i] = (double)(iter * 8 + i) * 0.25 + 0.075;
                long long mask = ((iter * 8 + i) % 3 == 0) ? 0xFFFFFFFFFFFFFFFFLL : 0x0000000000000000LL;
                dres[i] = (mask & *(long long*)&db[i]) | (~mask & *(long long*)&da[i]);
                total_checksum += (uint64_t)*(uint64_t*)&dres[i];
            }
        }
        
        // Integer blend
        {
            int ia[16];
            int ib[16];
            int ires[16];
            
            for (int i = 0; i < 16; i++) {
                ia[i] = iter * 16 + i;
                ib[i] = iter * 16 + i + 1000;
                int mask = ((iter * 16 + i) % 7 == 0) ? 0xFFFFFFFF : 0x00000000;
                ires[i] = (mask & ib[i]) | (~mask & ia[i]);
                total_checksum += (uint32_t)ires[i];
            }
        }
    }
    
    return total_checksum;
}

/* ========== Main driver function ========== */
int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Instruction Coverage Test\n");
    printf("=======================================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using optimized intrinsics.\n");
    
    // Test each vector mode individually
    printf("\nTesting V64QImode (64-byte integers)...\n");
    total_checksum += test_v64qi_blend();
    
    printf("Testing V32HImode (32-halfword integers)...\n");
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32-half-precision floats)...\n");
    total_checksum += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32-bfloat16)...\n");
    total_checksum += test_v32bf_blend();
#endif
    
    printf("Testing V16SImode (16-dword integers)...\n");
    total_checksum += test_v16si_blend();
    
    printf("Testing V8DImode (8-qword integers)...\n");
    total_checksum += test_v8di_blend();
    
    printf("Testing V8DFmode (8-double-precision floats)...\n");
    total_checksum += test_v8df_blend();
    
    printf("Testing V16SFmode (16-single-precision floats)...\n");
    total_checksum += test_v16sf_blend();
    
    printf("\nTesting mixed data types in loop...\n");
    total_checksum += test_mixed_blend_loop(10);
    
#else
    printf("AVX-512BW not detected. Using scalar fallback for some tests.\n");
    goto scalar_fallback;
#endif
#else
    printf("AVX-512F not detected. Using scalar fallback.\n");
    goto scalar_fallback;
#endif
    
    printf("\nFinal checksum (optimized): %lu\n", total_checksum);
    return (int)(total_checksum % 256);
    
scalar_fallback:
    printf("\nUsing scalar fallback implementations...\n");
    
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
    total_checksum += scalar_test_v16sf_blend();
    total_checksum += scalar_test_v8df_blend();
    total_checksum += scalar_test_mixed_blend_loop(5);
    
    printf("\nFinal checksum (scalar): %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
