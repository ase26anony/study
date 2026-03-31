#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mask_type) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result1, cmp_val, _MM_CMPINT_LT);
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int condition) {
    // V16SFmode blend with comparison mask
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_LT_OQ);
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with inverted mask
    __m512d cmp_val_d = _mm512_set1_pd(1.0);
    __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), cmp_val_d, _CMP_GT_OQ);
    if (condition > 0) {
        mask8 = _knot_mask8(mask8); // Invert mask based on condition
    }
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, _mm512_castps_pd(result1), d));
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, int mask_pattern) {
    // V32HFmode blend with pattern-based mask
    __mmask32 mask32;
    if (mask_pattern == 0) {
        mask32 = 0x55555555; // Alternating pattern
    } else {
        mask32 = 0xAAAAAAAA; // Opposite alternating pattern
    }
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // Second blend with combined masks
    __mmask32 mask32_2 = _mm512_cmp_ph_mask(result1, c, _CMP_EQ_OQ);
    mask32_2 = _kor_mask32(mask32, mask32_2); // Combine masks
    return _mm512_mask_blend_ph(mask32_2, result1, c);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    // V16SImode blend with loop-dependent mask
    __mmask16 mask16 = (iter % 2) ? 0xAAAA : 0x5555;
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with comparison mask
    __m512i cmp_val = _mm512_set1_epi64(1000);
    __mmask8 mask8 = _mm512_cmp_epi64_mask(result1, cmp_val, _MM_CMPINT_GT);
    return _mm512_mask_blend_epi64(mask8, result1, d);
}

// Main test function
int main() {
    // Initialize test data
    __m512i int_vec1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_vec2 = _mm512_set_epi16(
        100,101,102,103,104,105,106,107,108,109,110,111,
        112,113,114,115,116,117,118,119,120,121,122,123,
        124,125,126,127,128,129,130,131
    );
    
    __m512i int_vec3 = _mm512_set_epi32(
        200,201,202,203,204,205,206,207,
        208,209,210,211,212,213,214,215
    );
    
    __m512i int_vec4 = _mm512_set_epi64(
        300,301,302,303,304,305,306,307
    );
    
    __m512 float_vec1 = _mm512_set_ps(
        0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,
        0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f,1.6f
    );
    
    __m512 float_vec2 = _mm512_set_ps(
        2.1f,2.2f,2.3f,2.4f,2.5f,2.6f,2.7f,2.8f,
        2.9f,3.0f,3.1f,3.2f,3.3f,3.4f,3.5f,3.6f
    );
    
    __m512d double_vec1 = _mm512_set_pd(0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8);
    __m512d double_vec2 = _mm512_set_pd(1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8);
    
    // Initialize FP16 vectors (if supported)
    __m512h fp16_vec1, fp16_vec2, fp16_vec3;
    #ifdef __AVX512FP16__
    fp16_vec1 = _mm512_set_ph(
        0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,
        0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f,1.6f,
        1.7f,1.8f,1.9f,2.0f,2.1f,2.2f,2.3f,2.4f,
        2.5f,2.6f,2.7f,2.8f,2.9f,3.0f,3.1f,3.2f
    );
    fp16_vec2 = _mm512_set_ph(
        3.3f,3.4f,3.5f,3.6f,3.7f,3.8f,3.9f,4.0f,
        4.1f,4.2f,4.3f,4.4f,4.5f,4.6f,4.7f,4.8f,
        4.9f,5.0f,5.1f,5.2f,5.3f,5.4f,5.5f,5.6f,
        5.7f,5.8f,5.9f,6.0f,6.1f,6.2f,6.3f,6.4f
    );
    fp16_vec3 = _mm512_set_ph(
        6.5f,6.6f,6.7f,6.8f,6.9f,7.0f,7.1f,7.2f,
        7.3f,7.4f,7.5f,7.6f,7.7f,7.8f,7.9f,8.0f,
        8.1f,8.2f,8.3f,8.4f,8.5f,8.6f,8.7f,8.8f,
        8.9f,9.0f,9.1f,9.2f,9.3f,9.4f,9.5f,9.6f
    );
    #endif
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i blend_result1, blend_result2;
        __m512 blend_result3;
        __m512h blend_result4;
        
        // Control flow to select different blend types
        if (i % 3 == 0) {
            // Chain blends: V64QI -> V32HI -> V16SI -> V8DI
            blend_result1 = blend_v64qi_v32hi(int_vec1, int_vec2, i);
            blend_result2 = blend_v16si_v8di(int_vec3, int_vec4, blend_result1, int_vec4, i);
            
            // Reduce and accumulate
            int64_t sum = 0;
            int64_t* ptr = (int64_t*)&blend_result2;
            for (int j = 0; j < 8; j++) {
                sum += ptr[j];
            }
            checksum += sum;
        }
        else if (i % 3 == 1) {
            // Chain blends: V16SF -> V8DF
            blend_result3 = blend_v16sf_v8df(float_vec1, float_vec2, double_vec1, double_vec2, i);
            
            // Reduce and accumulate
            float sum = 0;
            float* ptr = (float*)&blend_result3;
            for (int j = 0; j < 16; j++) {
                sum += ptr[j];
            }
            checksum += sum;
        }
        else {
            #ifdef __AVX512FP16__
            // V32HF/V32BF blends
            blend_result4 = blend_v32hf_v32bf(fp16_vec1, fp16_vec2, fp16_vec3, i);
            
            // Reduce and accumulate
            _Float16 sum = 0;
            _Float16* ptr = (_Float16*)&blend_result4;
            for (int j = 0; j < 32; j++) {
                sum += ptr[j];
            }
            checksum += sum;
            #endif
        }
        
        // Direct blend calls for uncovered modes
        __mmask64 direct_mask64 = (i % 2) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
        __m512i direct_result1 = _mm512_mask_blend_epi8(direct_mask64, int_vec1, int_vec2);
        
        __mmask32 direct_mask32 = _mm512_cmp_epi16_mask(direct_result1, _mm512_set1_epi16(50), _MM_CMPINT_GT);
        __m512i direct_result2 = _mm512_mask_blend_epi16(direct_mask32, direct_result1, int_vec2);
        
        __mmask16 direct_mask16 = 0xAA55; // Patterned mask
        __m512i direct_result3 = _mm512_mask_blend_epi32(direct_mask16, int_vec3, int_vec4);
        
        __mmask8 direct_mask8 = _mm512_cmp_epi64_mask(direct_result3, _mm512_set1_epi64(250), _MM_CMPINT_LT);
        __m512i direct_result4 = _mm512_mask_blend_epi64(direct_mask8, direct_result3, int_vec4);
        
        // Float blends
        __mmask16 float_mask16 = _mm512_cmp_ps_mask(float_vec1, _mm512_set1_ps(1.0f), _CMP_GT_OQ);
        __m512 direct_result5 = _mm512_mask_blend_ps(float_mask16, float_vec1, float_vec2);
        
        __mmask8 double_mask8 = _knot_mask8(_mm512_cmp_pd_mask(double_vec1, _mm512_set1_pd(0.5), _CMP_LT_OQ));
        __m512d direct_result6 = _mm512_mask_blend_pd(double_mask8, double_vec1, double_vec2);
        
        // Accumulate more results
        float* fptr = (float*)&direct_result5;
        for (int j = 0; j < 16; j++) {
            checksum += fptr[j];
        }
        
        double* dptr = (double*)&direct_result6;
        for (int j = 0; j < 8; j++) {
            checksum += dptr[j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
