#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with pattern data
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend() {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50 - 100);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend() {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 1.25f));
        b[i] = bfloat16_from_float((float)(i * 2.5f));
    }
    
    // Load as epi16 for bfloat16 emulation
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison (treat as 16-bit integers)
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += float_from_bfloat16(result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 500 - 2000;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 10000LL;
        b[i] = (long long)i * 5000LL - 10000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_LE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.234;
        b[i] = (double)i * 2.345;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 1.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types Loop ==================== */
double test_mixed_blend_loop(int iterations) {
    double total_sum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // V16SF mode
        {
            float a[16] __attribute__((aligned(64)));
            float b[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = (float)(iter + i) * 0.25f;
                b[i] = (float)(iter - i) * 0.5f;
            }
            
            __m512 va = _mm512_load_ps(a);
            __m512 vb = _mm512_load_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            
            float temp[16];
            _mm512_store_ps(temp, vresult);
            for (int i = 0; i < 16; i++) {
                total_sum += temp[i];
            }
        }
        
        // V8DF mode
        {
            double a[8] __attribute__((aligned(64)));
            double b[8] __attribute__((aligned(64)));
            
            for (int i = 0; i < 8; i++) {
                a[i] = (double)(iter + i) * 0.125;
                b[i] = (double)(iter - i) * 0.25;
            }
            
            __m512d va = _mm512_load_pd(a);
            __m512d vb = _mm512_load_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
            
            double temp[8];
            _mm512_store_pd(temp, vresult);
            for (int i = 0; i < 8; i++) {
                total_sum += temp[i];
            }
        }
        
        // V16SI mode
        {
            int a[16] __attribute__((aligned(64)));
            int b[16] __attribute__((aligned(64)));
            
            for (int i = 0; i < 16; i++) {
                a[i] = iter * 100 + i;
                b[i] = iter * 50 - i;
            }
            
            __m512i va = _mm512_load_si512((__m512i*)a);
            __m512i vb = _mm512_load_si512((__m512i*)b);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
            __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
            
            int temp[16];
            _mm512_store_si512((__m512i*)temp, vresult);
            for (int i = 0; i < 16; i++) {
                total_sum += temp[i];
            }
        }
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
int scalar_test_v64qi_blend() {
    char a[64];
    char b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)(64 - i);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

int scalar_test_v32hi_blend() {
    short a[32];
    short b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 50 - 100);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Main Driver ==================== */
int main() {
    long long total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    // Test all vector modes
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (long long)test_v32bf_blend();
#endif
    
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (long long)test_v8df_blend();
    total_checksum += (long long)test_v16sf_blend();
    
    // Test mixed loop
    double mixed_result = test_mixed_blend_loop(10);
    total_checksum += (long long)mixed_result;
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
#endif
    
    printf("Total checksum: %lld\n", total_checksum);
    return (int)(total_checksum % 256);
}
