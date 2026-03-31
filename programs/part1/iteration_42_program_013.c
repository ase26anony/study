#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
int test_v64qi_blend() {
    __m512i a, b, result;
    __mmask64 mask;
    uint8_t a_data[64] __attribute__((aligned(64)));
    uint8_t b_data[64] __attribute__((aligned(64)));
    uint8_t result_data[64] __attribute__((aligned(64)));
    
    // Initialize with pattern
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = 64 - i;
    }
    
    a = _mm512_load_si512((__m512i*)a_data);
    b = _mm512_load_si512((__m512i*)b_data);
    
    // Generate dynamic mask using comparison
    mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(32));
    
    // This should trigger gen_avx512bw_blendmv64qi
    result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)result_data, result);
    
    // Compute checksum to prevent optimization
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
int test_v32hi_blend() {
    __m512i a, b, result;
    __mmask32 mask;
    int16_t a_data[32] __attribute__((aligned(64)));
    int16_t b_data[32] __attribute__((aligned(64)));
    int16_t result_data[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = i * 100;
        b_data[i] = -i * 100;
    }
    
    a = _mm512_load_si512((__m512i*)a_data);
    b = _mm512_load_si512((__m512i*)b_data);
    
    // Generate mask: select a where a > 0
    mask = _mm512_cmpgt_epi16_mask(a, _mm512_setzero_si512());
    
    // This should trigger gen_avx512bw_blendmv32hi
    result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)result_data, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
float test_v32hf_blend() {
    __m512h a, b, result;
    __mmask32 mask;
    _Float16 a_data[32] __attribute__((aligned(64)));
    _Float16 b_data[32] __attribute__((aligned(64)));
    _Float16 result_data[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i * 1.5f);
        b_data[i] = (_Float16)(i * 2.5f);
    }
    
    a = _mm512_load_ph(a_data);
    b = _mm512_load_ph(b_data);
    
    // Compare for mask generation
    mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(result_data, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result_data[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
float test_v32bf_blend() {
    __m512bh a, b, result;
    __mmask32 mask;
    __bfloat16 a_data[32] __attribute__((aligned(64)));
    __bfloat16 b_data[32] __attribute__((aligned(64)));
    __bfloat16 result_data[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = bfloat16_from_float((float)i * 1.1f);
        b_data[i] = bfloat16_from_float((float)i * 2.2f);
    }
    
    a = _mm512_load_si512((__m512i*)a_data);
    b = _mm512_load_si512((__m512i*)b_data);
    
    // For bfloat16, we need to emulate blend using integer operations
    // This should still trigger the blend logic
    mask = _mm512_cmplt_epi16_mask((__m512i)a, (__m512i)b);
    
    // Use integer blend for bfloat16 emulation
    result = (__m512bh)_mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
    _mm512_store_si512((__m512i*)result_data, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result_data[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
int test_v16si_blend() {
    __m512i a, b, result;
    __mmask16 mask;
    int32_t a_data[16] __attribute__((aligned(64)));
    int32_t b_data[16] __attribute__((aligned(64)));
    int32_t result_data[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = i * 1000;
        b_data[i] = i * 2000;
    }
    
    a = _mm512_load_si512((__m512i*)a_data);
    b = _mm512_load_si512((__m512i*)b_data);
    
    mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(8000));
    
    // This should trigger gen_avx512f_blendmv16si
    result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)result_data, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
long long test_v8di_blend() {
    __m512i a, b, result;
    __mmask8 mask;
    int64_t a_data[8] __attribute__((aligned(64)));
    int64_t b_data[8] __attribute__((aligned(64)));
    int64_t result_data[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = 1LL << (i + 10);
        b_data[i] = 1LL << (i + 20);
    }
    
    a = _mm512_load_si512((__m512i*)a_data);
    b = _mm512_load_si512((__m512i*)b_data);
    
    mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(1LL << 12));
    
    // This should trigger gen_avx512f_blendmv8di
    result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)result_data, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
double test_v8df_blend() {
    __m512d a, b, result;
    __mmask8 mask;
    double a_data[8] __attribute__((aligned(64)));
    double b_data[8] __attribute__((aligned(64)));
    double result_data[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = (double)i * 1.1;
        b_data[i] = (double)i * 2.2;
    }
    
    a = _mm512_load_pd(a_data);
    b = _mm512_load_pd(b_data);
    
    mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(result_data, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
float test_v16sf_blend() {
    __m512 a, b, result;
    __mmask16 mask;
    float a_data[16] __attribute__((aligned(64)));
    float b_data[16] __attribute__((aligned(64)));
    float result_data[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = (float)i * 0.5f;
        b_data[i] = (float)i * 1.5f;
    }
    
    a = _mm512_load_ps(a_data);
    b = _mm512_load_ps(b_data);
    
    mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(result_data, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
void test_mixed_blends(int iterations) {
    float float_sum = 0.0f;
    double double_sum = 0.0;
    int int_sum = 0;
    short short_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // V16SFmode
        {
            __m512 a = _mm512_set1_ps((float)iter * 0.1f);
            __m512 b = _mm512_set1_ps((float)iter * 0.2f);
            __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, a, b);
            float temp[16];
            _mm512_store_ps(temp, result);
            for (int i = 0; i < 16; i++) float_sum += temp[i];
        }
        
        // V8DFmode
        {
            __m512d a = _mm512_set1_pd((double)iter * 0.1);
            __m512d b = _mm512_set1_pd((double)iter * 0.2);
            __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, a, b);
            double temp[8];
            _mm512_store_pd(temp, result);
            for (int i = 0; i < 8; i++) double_sum += temp[i];
        }
        
        // V16SImode
        {
            __m512i a = _mm512_set1_epi32(iter * 10);
            __m512i b = _mm512_set1_epi32(iter * 20);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(iter * 5));
            __m512i result = _mm512_mask_blend_epi32(mask, a, b);
            int temp[16];
            _mm512_store_si512((__m512i*)temp, result);
            for (int i = 0; i < 16; i++) int_sum += temp[i];
        }
        
        // V32HImode
        {
            __m512i a = _mm512_set1_epi16((short)iter);
            __m512i b = _mm512_set1_epi16((short)(iter * 2));
            __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16((short)(iter / 2)));
            __m512i result = _mm512_mask_blend_epi16(mask, a, b);
            short temp[32];
            _mm512_store_si512((__m512i*)temp, result);
            for (int i = 0; i < 32; i++) short_sum += temp[i];
        }
    }
    
    printf("Mixed blends - Float sum: %f, Double sum: %f, Int sum: %d, Short sum: %d\n",
           float_sum, double_sum, int_sum, short_sum);
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
#ifndef __AVX512F__

int test_v64qi_blend() {
    uint8_t a_data[64];
    uint8_t b_data[64];
    uint8_t result_data[64];
    
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = 64 - i;
        result_data[i] = (i > 32) ? a_data[i] : b_data[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result_data[i];
    }
    return sum;
}

int test_v32hi_blend() {
    int16_t a_data[32];
    int16_t b_data[32];
    int16_t result_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = i * 100;
        b_data[i] = -i * 100;
        result_data[i] = (a_data[i] > 0) ? a_data[i] : b_data[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result_data[i];
    }
    return sum;
}

float test_v16sf_blend() {
    float a_data[16];
    float b_data[16];
    float result_data[16];
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = (float)i * 0.5f;
        b_data[i] = (float)i * 1.5f;
        result_data[i] = (a_data[i] < b_data[i]) ? a_data[i] : b_data[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    return sum;
}

double test_v8df_blend() {
    double a_data[8];
    double b_data[8];
    double result_data[8];
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = (double)i * 1.1;
        b_data[i] = (double)i * 2.2;
        result_data[i] = (a_data[i] < b_data[i]) ? a_data[i] : b_data[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    return sum;
}

int test_v16si_blend() {
    int32_t a_data[16];
    int32_t b_data[16];
    int32_t result_data[16];
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = i * 1000;
        b_data[i] = i * 2000;
        result_data[i] = (a_data[i] > 8000) ? a_data[i] : b_data[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result_data[i];
    }
    return sum;
}

long long test_v8di_blend() {
    int64_t a_data[8];
    int64_t b_data[8];
    int64_t result_data[8];
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = 1LL << (i + 10);
        b_data[i] = 1LL << (i + 20);
        result_data[i] = (a_data[i] > (1LL << 12)) ? a_data[i] : b_data[i];
    }
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result_data[i];
    }
    return sum;
}

void test_mixed_blends(int iterations) {
    printf("Scalar fallback - no AVX-512 support\n");
}

#endif

/* ==================== Main Driver ==================== */
int main() {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected, using intrinsics\n");
    
    // Test each vector mode individually
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (int)test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (int)test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += (int)test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += (int)test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += (int)test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    // Test mixed types in loop
    test_mixed_blends(10);
    
#else
    printf("AVX-512BW not detected, using scalar fallback for some modes\n");
#endif
#else
    printf("AVX-512F not detected, using scalar fallbacks\n");
#endif
    
    // Use scalar fallbacks for remaining tests
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += (int)test_v16sf_blend();
    total_checksum += (int)test_v8df_blend();
    total_checksum += test_v16si_blend();
    total_checksum += (int)test_v8di_blend();
    
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
