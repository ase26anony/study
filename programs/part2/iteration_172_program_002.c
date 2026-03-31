/* AVX-512 blend coverage test for i386-expand.cc */
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
#ifdef __GNUC__
#define AVX512F_FUNC __attribute__((target("avx512f")))
#define AVX512BW_FUNC __attribute__((target("avx512f,avx512bw")))
#define AVX512FP16_FUNC __attribute__((target("avx512f,avx512bw,avx512fp16")))
#define AVX512BF16_FUNC __attribute__((target("avx512f,avx512bw,avx512bf16")))
#else
#define AVX512F_FUNC
#define AVX512BW_FUNC
#define AVX512FP16_FUNC
#define AVX512BF16_FUNC
#endif

/* Prevent aggressive optimization */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE __declspec(noinline)
#endif

/* ==================== V64QImode (64x8-bit integers) ==================== */
AVX512BW_FUNC NOINLINE
void test_v64qimode(uint8_t* out, const uint8_t* a, const uint8_t* b, int iter) {
#if HAS_AVX512F && HAS_AVX512BW
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask based on iteration count */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((iter + i) & 1) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V32HImode (32x16-bit integers) ==================== */
AVX512BW_FUNC NOINLINE
void test_v32himode(int16_t* out, const int16_t* a, const int16_t* b, int iter) {
#if HAS_AVX512F && HAS_AVX512BW
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask using comparison */
    __m512i cmp = _mm512_set1_epi16(iter);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(va, cmp);
    
    /* Mix with pattern for more dynamic behavior */
    mask ^= (__mmask32)(iter * 0x55555555);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V32HFmode (32x16-bit half precision) ==================== */
AVX512FP16_FUNC NOINLINE
void test_v32hfmode(_Float16* out, const _Float16* a, const _Float16* b, int iter) {
#if HAS_AVX512F && HAS_AVX512BW && HAS_AVX512FP16
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    /* Create dynamic mask using comparison */
    __m512h threshold = _mm512_set1_ph((_Float16)(iter % 100));
    __mmask32 mask = _mm512_cmp_ph_mask(va, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V32BFmode (32x16-bit bfloat16) ==================== */
AVX512BF16_FUNC NOINLINE
void test_v32bfmode(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int iter) {
#if HAS_AVX512F && HAS_AVX512BW && HAS_AVX512BF16
    /* Load bfloat16 data */
    __m512bh va = _mm512_loadu_bf16((const void*)a);
    __m512bh vb = _mm512_loadu_bf16((const void*)b);
    
    /* Convert to float for comparison to create dynamic mask */
    __m512 va_float = _mm512_cvtneobf16_ps(va);
    __m512 vb_float = _mm512_cvtneobf16_ps(vb);
    
    /* Create dynamic mask - compare with threshold based on iteration */
    __m512 threshold = _mm512_set1_ps((float)(iter % 10));
    __mmask16 mask32 = _mm512_cmp_ps_mask(va_float, threshold, _CMP_GT_OQ);
    
    /* Expand 16-bit mask to 32-bit for bfloat16 blend */
    __mmask32 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (mask32 & (1 << i)) {
            mask |= (3ULL << (i * 2));
        }
    }
    
    /* Apply additional pattern */
    mask ^= (__mmask32)(iter * 0xAAAAAAAA);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_bf16((void*)out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V16SImode (16x32-bit integers) ==================== */
AVX512F_FUNC NOINLINE
void test_v16simode(int32_t* out, const int32_t* a, const int32_t* b, int iter) {
#if HAS_AVX512F
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(iter);
    __mmask16 mask = _mm512_cmpeq_epi32_mask(va, cmp_val);
    
    /* Mix with pattern */
    mask ^= (__mmask16)(iter * 0x5555);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V8DImode (8x64-bit integers) ==================== */
AVX512F_FUNC NOINLINE
void test_v8dimode(int64_t* out, const int64_t* a, const int64_t* b, int iter) {
#if HAS_AVX512F
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask using comparison */
    __m512i cmp_val = _mm512_set1_epi64(iter);
    __mmask8 mask = _mm512_cmpeq_epi64_mask(va, cmp_val);
    
    /* Mix with pattern */
    mask ^= (__mmask8)(iter * 0x55);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V8DFmode (8x64-bit doubles) ==================== */
AVX512F_FUNC NOINLINE
void test_v8dfmode(double* out, const double* a, const double* b, int iter) {
#if HAS_AVX512F
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    /* Dynamic mask using comparison */
    __m512d threshold = _mm512_set1_pd((double)(iter % 10));
    __mmask8 mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    /* Mix with pattern */
    mask ^= (__mmask8)(iter * 0xAA);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== V16SFmode (16x32-bit floats) ==================== */
AVX512F_FUNC NOINLINE
void test_v16sfmode(float* out, const float* a, const float* b, int iter) {
#if HAS_AVX512F
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    /* Dynamic mask using comparison */
    __m512 threshold = _mm512_set1_ps((float)(iter % 10));
    __mmask16 mask = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    /* Mix with pattern */
    mask ^= (__mmask16)(iter * 0xAAAA);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(out, result);
#else
    (void)out; (void)a; (void)b; (void)iter;
#endif
}

/* ==================== Main test driver ==================== */
int main() {
    const int NUM_ITERATIONS = 100;
    uint64_t checksum = 0;
    
    /* Initialize test data */
    uint8_t a_u8[64], b_u8[64], out_u8[64];
    int16_t a_i16[32], b_i16[32], out_i16[32];
    _Float16 a_f16[32], b_f16[32], out_f16[32];
    __bfloat16 a_bf16[32], b_bf16[32], out_bf16[32];
    int32_t a_i32[16], b_i32[16], out_i32[16];
    int64_t a_i64[8], b_i64[8], out_i64[8];
    double a_f64[8], b_f64[8], out_f64[8];
    float a_f32[16], b_f32[16], out_f32[16];
    
    /* Fill arrays with pattern data */
    for (int i = 0; i < 64; i++) {
        a_u8[i] = i;
        b_u8[i] = 64 - i;
        if (i < 32) {
            a_i16[i] = i * 2;
            b_i16[i] = 64 - i * 2;
            a_f16[i] = (_Float16)(i * 0.5f);
            b_f16[i] = (_Float16)(32.0f - i * 0.5f);
            a_bf16[i] = (__bfloat16)(i * 0.7f);
            b_bf16[i] = (__bfloat16)(32.0f - i * 0.7f);
        }
        if (i < 16) {
            a_i32[i] = i * 4;
            b_i32[i] = 64 - i * 4;
            a_f32[i] = i * 0.25f;
            b_f32[i] = 16.0f - i * 0.25f;
        }
        if (i < 8) {
            a_i64[i] = i * 8;
            b_i64[i] = 64 - i * 8;
            a_f64[i] = i * 0.125;
            b_f64[i] = 8.0 - i * 0.125;
        }
    }
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        /* Test each vector mode */
        test_v64qimode(out_u8, a_u8, b_u8, iter);
        test_v32himode(out_i16, a_i16, b_i16, iter);
        test_v32hfmode(out_f16, a_f16, b_f16, iter);
        test_v32bfmode(out_bf16, a_bf16, b_bf16, iter);
        test_v16simode(out_i32, a_i32, b_i32, iter);
        test_v8dimode(out_i64, a_i64, b_i64, iter);
        test_v8dfmode(out_f64, a_f64, b_f64, iter);
        test_v16sfmode(out_f32, a_f32, b_f32, iter);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < 64; i++) {
            checksum += out_u8[i];
            if (i < 32) checksum += out_i16[i];
            if (i < 16) checksum += out_i32[i];
            if (i < 8) checksum += out_i64[i];
        }
    }
    
    printf("AVX-512 blend test completed. Checksum: %lu\n", checksum);
    
    /* Return non-zero if any required feature is missing */
#if !HAS_AVX512F
    printf("Warning: AVX-512F not available\n");
    return 1;
#endif
    
#if !HAS_AVX512BW
    printf("Warning: AVX-512BW not available\n");
    return 2;
#endif
    
    return 0;
}
