#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static uint64_t test_v64qi_blend(void) {
    char src1[64] __attribute__((aligned(64)));
    char src2[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        src1[i] = (char)(i % 16);
        src2[i] = (char)((i + 8) % 16);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static uint64_t test_v32hi_blend(void) {
    short src1[32] __attribute__((aligned(64)));
    short src2[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (short)(i * 100);
        src2[i] = (short)(i * 150);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using bitwise pattern
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (src1[i] > src2[i]) {
            mask |= (1U << i);
        }
    }
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 src1[32] __attribute__((aligned(64)));
    _Float16 src2[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(result[i] * 100);
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bf16 src1[32] __attribute__((aligned(64)));
    __bf16 src2[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bf16)(i * 1.1f);
        src2[i] = (__bf16)(i * 2.2f);
    }
    
    // Load as integers since bfloat16 blend uses epi16
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create mask based on comparison
    __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static uint64_t test_v16si_blend(void) {
    int src1[16] __attribute__((aligned(64)));
    int src2[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using runtime comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static uint64_t test_v8di_blend(void) {
    long long src1[8] __attribute__((aligned(64)));
    long long src2[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 30000LL;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Generate mask using bitwise operations
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, v2, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
static uint64_t test_v8df_blend(void) {
    double src1[8] __attribute__((aligned(64)));
    double src2[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.111;
        src2[i] = i * 2.222;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Generate mask using floating comparison
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* ==================== V16SFmode (16-single precision floats) ==================== */
static uint64_t test_v16sf_blend(void) {
    float src1[16] __attribute__((aligned(64)));
    float src2[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Generate mask using floating comparison
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 100);
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    // Process arrays with different data types
    for (int iter = 0; iter < 10; iter++) {
        // Float blend
        {
            float a[16] __attribute__((aligned(64)));
            float b[16] __attribute__((aligned(64)));
            float c[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = (iter + i) * 1.1f;
                b[i] = (iter + i) * 2.2f;
            }
            
            __m512 va = _mm512_load_ps(a);
            __m512 vb = _mm512_load_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 vc = _mm512_mask_blend_ps(mask, va, vb);
            _mm512_store_ps(c, vc);
            
            for (int i = 0; i < 16; i++) {
                total_sum += (uint32_t)(c[i] * 100);
            }
        }
        
        // Double blend
        {
            double a[8] __attribute__((aligned(64)));
            double b[8] __attribute__((aligned(64)));
            double c[8] __attribute__((aligned(64)));
            
            for (int i = 0; i < 8; i++) {
                a[i] = (iter + i) * 1.5;
                b[i] = (iter + i) * 3.0;
            }
            
            __m512d va = _mm512_load_pd(a);
            __m512d vb = _mm512_load_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
            __m512d vc = _mm512_mask_blend_pd(mask, va, vb);
            _mm512_store_pd(c, vc);
            
            for (int i = 0; i < 8; i++) {
                total_sum += (uint64_t)(c[i] * 1000);
            }
        }
        
        // Integer blend (32-bit)
        {
            int a[16] __attribute__((aligned(64)));
            int b[16] __attribute__((aligned(64)));
            int c[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = (iter + i) * 100;
                b[i] = (iter + i) * 200;
            }
            
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_GT);
            __m512i vc = _mm512_mask_blend_epi32(mask, va, vb);
            _mm512_store_si512((__m512i*)c, vc);
            
            for (int i = 0; i < 16; i++) {
                total_sum += (uint32_t)c[i];
            }
        }
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
static uint64_t scalar_test_v64qi_blend(void) {
    char src1[64];
    char src2[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (char)(i % 16);
        src2[i] = (char)((i + 8) % 16);
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short src1[32];
    short src2[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (short)(i * 100);
        src2[i] = (short)(i * 150);
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v16si_blend(void) {
    int src1[16];
    int src2[16];
    int result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v8di_blend(void) {
    long long src1[8];
    long long src2[8];
    long long result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 30000LL;
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float src1[16];
    float src2[16];
    float result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f;
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)(result[i] * 100);
    }
    return sum;
}

static uint64_t scalar_test_v8df_blend(void) {
    double src1[8];
    double src2[8];
    double result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.111;
        src2[i] = i * 2.222;
        result[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Run all vector mode tests
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed data types test completed\n");
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
#endif

#ifndef __AVX512F__
    // Run scalar fallbacks
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
    total_checksum += scalar_test_v16sf_blend();
    total_checksum += scalar_test_v8df_blend();
#endif

    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
