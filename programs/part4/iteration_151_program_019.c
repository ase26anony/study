#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    // V64QImode blend
    __mmask64 mask64 = selector ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with mask from comparison
    __m512i cmp_val = _mm512_set1_epi16(selector * 100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result1, cmp_val, _MM_CMPINT_GT);
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    // V16SFmode blend with dynamic mask
    __m512 cmp_val = _mm512_set1_ps(selector * 50.0f);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with inverted mask
    __m512d cmp_val_d = _mm512_set1_pd(selector * 25.0);
    __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), cmp_val_d, _CMP_LT_OQ);
    mask8 = _knot_mask8(mask8); // Test mask operations
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, _mm512_castps_pd(result1), d));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int selector) {
    // V32HFmode blend with pattern mask
    __mmask32 mask32 = selector ? 0xAAAAAAAA : 0x55555555;
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // For V32BFmode (treated same as HF in intrinsics)
    // Create mask using comparison
    __m512h cmp_val = _mm512_set1_ph(selector ? 1.0f : -1.0f);
    __mmask32 mask32_2 = _mm512_cmp_ph_mask(result1, cmp_val, _CMP_EQ_OQ);
    return _mm512_mask_blend_ph(mask32_2, result1, a);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    // V16SImode blend with complex mask generation
    __m512i cmp_val = _mm512_set1_epi32(selector * 1000);
    __mmask16 mask16 = _mm512_cmp_epi32_mask(a, cmp_val, _MM_CMPINT_LT);
    
    // Combine with another mask
    __mmask16 mask16_2 = _mm512_cmp_epi32_mask(b, cmp_val, _MM_CMPINT_GT);
    __mmask16 combined_mask = _kor_mask16(mask16, mask16_2);
    
    __m512i result1 = _mm512_mask_blend_epi32(combined_mask, a, b);
    
    // V8DImode blend
    __mmask8 mask8 = selector ? 0xAA : 0x55;
    return _mm512_mask_blend_epi64(mask8, result1, d);
}

// Function that chains multiple blend operations
static inline float chain_blends(int iterations) {
    // Initialize vectors with distinct patterns
    __m512i vi1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 vf1 = _mm512_set_ps(
        63.0f,62.0f,61.0f,60.0f,59.0f,58.0f,57.0f,56.0f,
        55.0f,54.0f,53.0f,52.0f,51.0f,50.0f,49.0f,48.0f,
        47.0f,46.0f,45.0f,44.0f,43.0f,42.0f,41.0f,40.0f,
        39.0f,38.0f,37.0f,36.0f,35.0f,34.0f,33.0f,32.0f,
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 vf2 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f,
        32.0f,33.0f,34.0f,35.0f,36.0f,37.0f,38.0f,39.0f,
        40.0f,41.0f,42.0f,43.0f,44.0f,45.0f,46.0f,47.0f,
        48.0f,49.0f,50.0f,51.0f,52.0f,53.0f,54.0f,55.0f,
        56.0f,57.0f,58.0f,59.0f,60.0f,61.0f,62.0f,63.0f
    );
    
    __m512d vd1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d vd2 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    __m512i vi3 = _mm512_set1_epi32(100);
    __m512i vi4 = _mm512_set1_epi64(200);
    
    float checksum = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        // Data-dependent control flow
        if (i % 3 == 0) {
            // Chain integer blends
            __m512i int_result = blend_v64qi_v32hi(vi1, vi2, i);
            int_result = blend_v16si_v8di(int_result, vi3, vi2, vi4, i);
            
            // Reduce and accumulate
            __m256i low = _mm512_castsi512_si256(int_result);
            __m256i high = _mm512_extracti64x4_epi64(int_result, 1);
            __m256i sum256 = _mm256_add_epi64(low, high);
            __m128i sum128 = _mm_add_epi64(_mm256_castsi256_si128(sum256),
                                          _mm256_extracti128_si256(sum256, 1));
            checksum += (float)(_mm_extract_epi64(sum128, 0) + 
                               _mm_extract_epi64(sum128, 1));
        } 
        else if (i % 3 == 1) {
            // Chain float blends
            __m512 float_result = blend_v16sf_v8df(vf1, vf2, vd1, vd2, i);
            
            // Reduce and accumulate
            checksum += _mm512_reduce_add_ps(float_result);
        }
        else {
#ifdef __AVX512FP16__
            // FP16 blends (if supported)
            __m512h vh1 = _mm512_castps_ph(vf1);
            __m512h vh2 = _mm512_castps_ph(vf2);
            __m512h hf_result = blend_v32hf_v32bf(vh1, vh2, i);
            
            // Convert back to float for reduction
            __m512 float_result = _mm512_castph_ps(hf_result);
            checksum += _mm512_reduce_add_ps(float_result);
#else
            // Alternative: more integer blends
            __mmask64 mask64 = (i % 2) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
            __m512i alt_result = _mm512_mask_blend_epi8(mask64, vi1, vi2);
            
            // Manual reduction
            int64_t sum = 0;
            int8_t* ptr = (int8_t*)&alt_result;
            for (int j = 0; j < 64; j++) {
                sum += ptr[j];
            }
            checksum += (float)sum;
#endif
        }
        
        // Modify vectors slightly each iteration
        vi1 = _mm512_add_epi8(vi1, _mm512_set1_epi8(1));
        vf1 = _mm512_add_ps(vf1, _mm512_set1_ps(0.5f));
        vd1 = _mm512_add_pd(vd1, _mm512_set1_pd(0.25));
    }
    
    return checksum;
}

int main() {
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Test all blend modes directly
    __m512i a_i64 = _mm512_set1_epi64(1);
    __m512i b_i64 = _mm512_set1_epi64(2);
    __m512i a_i32 = _mm512_set1_epi32(3);
    __m512i b_i32 = _mm512_set1_epi32(4);
    __m512i a_i16 = _mm512_set1_epi16(5);
    __m512i b_i16 = _mm512_set1_epi16(6);
    __m512i a_i8 = _mm512_set1_epi8(7);
    __m512i b_i8 = _mm512_set1_epi8(8);
    
    __m512 a_ps = _mm512_set1_ps(9.0f);
    __m512 b_ps = _mm512_set1_ps(10.0f);
    __m512d a_pd = _mm512_set1_pd(11.0);
    __m512d b_pd = _mm512_set1_pd(12.0);
    
    // Direct calls to all blend intrinsics
    __m512i r8 = _mm512_mask_blend_epi64(0xAA, a_i64, b_i64);
    __m512i r16 = _mm512_mask_blend_epi32(0xAAAA, a_i32, b_i32);
    __m512i r32 = _mm512_mask_blend_epi16(0xAAAAAAAA, a_i16, b_i16);
    __m512i r64 = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAAULL, a_i8, b_i8);
    
    __m512 rps = _mm512_mask_blend_ps(0xAAAA, a_ps, b_ps);
    __m512d rpd = _mm512_mask_blend_pd(0xAA, a_pd, b_pd);
    
#ifdef __AVX512FP16__
    __m512h a_ph = _mm512_set1_ph(13.0f);
    __m512h b_ph = _mm512_set1_ph(14.0f);
    __m512h rph = _mm512_mask_blend_ph(0xAAAAAAAA, a_ph, b_ph);
#endif
    
    // Complex chained operation
    float checksum = chain_blends(100);
    
    // Reduce all results to prevent dead code elimination
    __m256i r8_256 = _mm512_castsi512_si256(r8);
    __m256i r16_256 = _mm512_castsi512_si256(r16);
    __m256i r32_256 = _mm512_castsi512_si256(r32);
    __m256i r64_256 = _mm512_castsi512_si256(r64);
    
    // Combine and hash results
    __m256i combined = _mm256_add_epi64(r8_256, r16_256);
    combined = _mm256_add_epi64(combined, r32_256);
    combined = _mm256_add_epi64(combined, r64_256);
    
    __m128i low128 = _mm256_castsi256_si128(combined);
    __m128i high128 = _mm256_extracti128_si256(combined, 1);
    __m128i sum128 = _mm_add_epi64(low128, high128);
    
    uint64_t int_sum = _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
    
    float float_sum = _mm512_reduce_add_ps(rps) + _mm512_reduce_add_pd(rpd);
    
#ifdef __AVX512FP16__
    __m512 rph_f32 = _mm512_castph_ps(rph);
    float_sum += _mm512_reduce_add_ps(rph_f32);
#endif
    
    checksum += (float)int_sum + float_sum;
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
