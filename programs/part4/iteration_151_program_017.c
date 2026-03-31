#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    // Chain epi8 -> epi16 blend
    __m512i blended8 = _mm512_mask_blend_epi8(k8, a, b);
    return _mm512_mask_blend_epi16(k16, a, blended8);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 k32, __mmask8 k64) {
    // Chain ps -> pd blend
    __m512 blended_ps = _mm512_mask_blend_ps(k32, a, b);
    __m512d blended_pd = _mm512_mask_blend_pd(k64, c, d);
    
    // Convert pd result back to ps for compatibility
    __m512 pd_as_ps = _mm512_castpd_ps(blended_pd);
    return _mm512_add_ps(blended_ps, pd_as_ps);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // Use same intrinsic for both HF and BF modes
    return _mm512_mask_blend_ph(k16, a, b);
}
#endif

// Function with data-dependent control flow
static __m512i conditional_blend(int mode, __m512i a, __m512i b, 
                                 __mmask64 k8, __mmask32 k16,
                                 __mmask16 k32, __mmask8 k64) {
    __m512i result = a;
    
    if (mode & 1) {
        // Use epi8 blend
        result = _mm512_mask_blend_epi8(k8, a, b);
    } else {
        // Use epi16 blend
        result = _mm512_mask_blend_epi16(k16, a, b);
    }
    
    if (mode & 2) {
        // Chain with epi32 blend
        result = _mm512_mask_blend_epi32(k32, a, result);
    }
    
    if (mode & 4) {
        // Chain with epi64 blend
        result = _mm512_mask_blend_epi64(k64, b, result);
    }
    
    return result;
}

int main() {
    // Initialize data with distinct patterns
    int8_t i8_data_a[64], i8_data_b[64];
    int16_t i16_data_a[32], i16_data_b[32];
    int32_t i32_data_a[16], i32_data_b[16];
    int64_t i64_data_a[8], i64_data_b[8];
    float f32_data_a[16], f32_data_b[16];
    double f64_data_a[8], f64_data_b[8];
    
    for (int i = 0; i < 64; i++) {
        i8_data_a[i] = i;
        i8_data_b[i] = 64 - i;
        if (i < 32) {
            i16_data_a[i] = i * 2;
            i16_data_b[i] = 64 - i * 2;
        }
        if (i < 16) {
            i32_data_a[i] = i * 4;
            i32_data_b[i] = 256 - i * 4;
            f32_data_a[i] = i * 0.5f;
            f32_data_b[i] = 8.0f - i * 0.5f;
        }
        if (i < 8) {
            i64_data_a[i] = i * 8;
            i64_data_b[i] = 512 - i * 8;
            f64_data_a[i] = i * 0.25;
            f64_data_b[i] = 2.0 - i * 0.25;
        }
    }
    
    // Load vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)i8_data_a);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)i8_data_b);
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)i16_data_a);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)i16_data_b);
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)i32_data_a);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)i32_data_b);
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)i64_data_a);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)i64_data_b);
    
    __m512 v16sf_a = _mm512_loadu_ps(f32_data_a);
    __m512 v16sf_b = _mm512_loadu_ps(f32_data_b);
    __m512d v8df_a = _mm512_loadu_pd(f64_data_a);
    __m512d v8df_b = _mm512_loadu_pd(f64_data_b);
    
    // Generate masks using different methods
    // 1. Immediate constants
    __mmask64 k8_imm = 0xAAAAAAAAAAAAAAAA;
    __mmask32 k16_imm = 0xAAAAAAAA;
    __mmask16 k32_imm = 0xAAAA;
    __mmask8 k64_imm = 0xAA;
    
    // 2. Comparison intrinsics
    __mmask64 k8_cmp = _mm512_cmp_epi8_mask(v64qi_a, v64qi_b, _MM_CMPINT_GT);
    __mmask32 k16_cmp = _mm512_cmp_epi16_mask(v32hi_a, v32hi_b, _MM_CMPINT_LT);
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_EQ);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_NE);
    __mmask16 k32f_cmp = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
    __mmask8 k64f_cmp = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
    
    // 3. Bitwise operations on masks
    __mmask64 k8_bw = _kor_mask64(k8_imm, k8_cmp);
    __mmask32 k16_bw = _kxor_mask32(k16_imm, k16_cmp);
    __mmask16 k32_bw = _knot_mask16(k32_cmp);
    __mmask8 k64_bw = _kand_mask8(k64_imm, k64_cmp);
    
    // Variable for control flow
    int mode_selector = 0;
    long long checksum = 0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        mode_selector = (mode_selector + i) % 8;
        
        // Perform blends based on mode_selector
        __m512i blended_int = conditional_blend(mode_selector, v64qi_a, v64qi_b,
                                                k8_bw, k16_bw, k32_bw, k64_bw);
        
        // Direct intrinsic calls for all modes
        __m512i result_64qi = _mm512_mask_blend_epi8(k8_imm, v64qi_a, v64qi_b);
        __m512i result_32hi = _mm512_mask_blend_epi16(k16_cmp, v32hi_a, v32hi_b);
        __m512i result_16si = _mm512_mask_blend_epi32(k32_bw, v16si_a, v16si_b);
        __m512i result_8di = _mm512_mask_blend_epi64(k64_imm, v8di_a, v8di_b);
        __m512 result_16sf = _mm512_mask_blend_ps(k32f_cmp, v16sf_a, v16sf_b);
        __m512d result_8df = _mm512_mask_blend_pd(k64f_cmp, v8df_a, v8df_b);
        
        // Use helper functions
        __m512i chained_int = blend_v64qi_v32hi(v64qi_a, v64qi_b, k8_cmp, k16_bw);
        __m512 chained_float = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b,
                                                k32_imm, k64_bw);
        
        // FP16/BF16 blends if available
        #ifdef __AVX512FP16__
        _Float16 f16_data_a[32], f16_data_b[32];
        for (int j = 0; j < 32; j++) {
            f16_data_a[j] = j * 0.1f;
            f16_data_b[j] = 3.0f - j * 0.1f;
        }
        __m512h v32hf_a = _mm512_loadu_ph(f16_data_a);
        __m512h v32hf_b = _mm512_loadu_ph(f16_data_b);
        __mmask32 k16_hf = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_GT_OQ);
        __m512h result_32hf = _mm512_mask_blend_ph(k16_hf, v32hf_a, v32hf_b);
        
        // For BF16, we use the same intrinsic but different data
        __m512h result_32bf = blend_v32hf_v32bf(v32hf_b, v32hf_a, k16_hf);
        #endif
        
        // Prevent dead code elimination by computing reductions
        // Reduce integer vectors
        __m256i reduced_256 = _mm512_castsi512_si256(result_64qi);
        __m128i reduced_128 = _mm256_castsi256_si128(reduced_256);
        
        // Extract and accumulate to checksum
        int64_t temp[8];
        _mm512_storeu_si512((__m512i*)temp, result_8di);
        for (int j = 0; j < 8; j++) {
            checksum += temp[j];
        }
        
        // Reduce float vectors
        float f_temp[16];
        _mm512_storeu_ps(f_temp, result_16sf);
        for (int j = 0; j < 16; j++) {
            checksum += (long long)f_temp[j];
        }
        
        double d_temp[8];
        _mm512_storeu_pd(d_temp, result_8df);
        for (int j = 0; j < 8; j++) {
            checksum += (long long)d_temp[j];
        }
        
        #ifdef __AVX512FP16__
        _Float16 h_temp[32];
        _mm512_storeu_ph(h_temp, result_32hf);
        for (int j = 0; j < 32; j++) {
            checksum += (long long)h_temp[j];
        }
        #endif
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
