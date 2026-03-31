#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static uint64_t test_v64qi_blend(void) {
    alignas(64) int8_t a[64], b[64], result[64];
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static uint64_t test_v32hi_blend(void) {
    alignas(64) int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 3);
        b[i] = (int16_t)(i * 5 + 1);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    alignas(64) _Float16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.3f + 0.1f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(*(uint16_t*)&result[i]);
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    alignas(64) __bf16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        // Use integer representation for bfloat16
        uint16_t val_a = (uint16_t)((i % 16) << 10);
        uint16_t val_b = (uint16_t)(((i + 8) % 16) << 10);
        memcpy(&a[i], &val_a, sizeof(__bf16));
        memcpy(&b[i], &val_b, sizeof(__bf16));
    }
    
    __m512bh va = _mm512_load_si512((const __m512i*)a);
    __m512bh vb = _mm512_load_si512((const __m512i*)b);
    
    // For bfloat16, we need to emulate blend using epi16
    __m512i va_int = _mm512_castps_si512(_mm512_castbh_ps(va));
    __m512i vb_int = _mm512_castps_si512(_mm512_castbh_ps(vb));
    
    // Generate mask based on comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va_int, vb_int, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512i vresult_int = _mm512_mask_blend_epi16(mask, va_int, vb_int);
    __m512bh vresult = _mm512_castsi512_bh(vresult_int);
    
    _mm512_store_si512((__m512i*)result, _mm512_castbh_si512(vresult));
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)(*(uint16_t*)&result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static uint64_t test_v16si_blend(void) {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 7;
        b[i] = i * 11 + 3;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static uint64_t test_v8di_blend(void) {
    alignas(64) int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 13LL;
        b[i] = i * 17LL + 5;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double precision floats) ==================== */
static uint64_t test_v8df_blend(void) {
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.0 + 0.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* ==================== V16SFmode (16-single precision floats) ==================== */
static uint64_t test_v16sf_blend(void) {
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 0.9f + 0.1f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_NEQ_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static uint64_t test_mixed_blend_loop(void) {
    const int N = 1024;
    alignas(64) float f32_a[N], f32_b[N], f32_result[N];
    alignas(64) double f64_a[N], f64_b[N], f64_result[N];
    alignas(64) int32_t i32_a[N], i32_b[N], i32_result[N];
    alignas(64) int16_t i16_a[N], i16_b[N], i16_result[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        f32_a[i] = i * 0.1f;
        f32_b[i] = i * 0.2f;
        f64_a[i] = i * 0.3;
        f64_b[i] = i * 0.4;
        i32_a[i] = i * 3;
        i32_b[i] = i * 5;
        i16_a[i] = (int16_t)(i % 100);
        i16_b[i] = (int16_t)((i + 50) % 100);
    }
    
    uint64_t total_sum = 0;
    
    // Process in chunks of vector size
    for (int i = 0; i < N; i += 16) {
        // V16SFmode blend
        __m512 va_f32 = _mm512_load_ps(&f32_a[i]);
        __m512 vb_f32 = _mm512_load_ps(&f32_b[i]);
        __mmask16 mask_f32 = _mm512_cmp_ps_mask(va_f32, vb_f32, _CMP_GT_OQ);
        __m512 vr_f32 = _mm512_mask_blend_ps(mask_f32, va_f32, vb_f32);
        _mm512_store_ps(&f32_result[i], vr_f32);
        
        // V16SImode blend (process 16 int32)
        __m512i va_i32 = _mm512_load_si512((const __m512i*)&i32_a[i]);
        __m512i vb_i32 = _mm512_load_si512((const __m512i*)&i32_b[i]);
        __mmask16 mask_i32 = _mm512_cmp_epi32_mask(va_i32, vb_i32, _MM_CMPINT_LT);
        __m512i vr_i32 = _mm512_mask_blend_epi32(mask_i32, va_i32, vb_i32);
        _mm512_store_si512((__m512i*)&i32_result[i], vr_i32);
    }
    
    for (int i = 0; i < N; i += 32) {
        // V32HImode blend (process 32 int16)
        __m512i va_i16 = _mm512_load_si512((const __m512i*)&i16_a[i]);
        __m512i vb_i16 = _mm512_load_si512((const __m512i*)&i16_b[i]);
        __mmask32 mask_i16 = _mm512_cmp_epi16_mask(va_i16, vb_i16, _MM_CMPINT_EQ);
        __m512i vr_i16 = _mm512_mask_blend_epi16(mask_i16, va_i16, vb_i16);
        _mm512_store_si512((__m512i*)&i16_result[i], vr_i16);
    }
    
    for (int i = 0; i < N; i += 8) {
        // V8DFmode blend
        __m512d va_f64 = _mm512_load_pd(&f64_a[i]);
        __m512d vb_f64 = _mm512_load_pd(&f64_b[i]);
        __mmask8 mask_f64 = _mm512_cmp_pd_mask(va_f64, vb_f64, _CMP_LE_OQ);
        __m512d vr_f64 = _mm512_mask_blend_pd(mask_f64, va_f64, vb_f64);
        _mm512_store_pd(&f64_result[i], vr_f64);
    }
    
    // Compute checksums
    for (int i = 0; i < N; i++) {
        total_sum += (uint64_t)(f32_result[i] * 100);
        total_sum += (uint64_t)(f64_result[i] * 100);
        total_sum += i32_result[i];
        total_sum += i16_result[i];
    }
    
    return total_sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

/* ==================== Scalar Fallbacks ==================== */
static uint64_t scalar_test_v64qi_blend(void) {
    int8_t a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i % 3);
        b[i] = (int8_t)((i + 1) % 5);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 3);
        b[i] = (int16_t)(i * 5 + 1);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* ==================== Main Driver ==================== */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
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
    
    total_checksum += test_mixed_blend_loop();
    printf("Mixed data type loop test completed\n");
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
#endif

#ifndef __AVX512F__
    // Run scalar versions
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    // Add more scalar fallbacks as needed
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
