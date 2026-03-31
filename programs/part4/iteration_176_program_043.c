#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each intrinsic gets expanded separately
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask = 0;
    
    // Switch statement for control flow diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 1:
            // Immediate mask (alternating pattern)
            mask = 0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = 0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Mask based on comparison result
            __mmask64 cmp_mask = _mm512_cmpgt_epi8_mask(data, _mm512_set1_epi8(32));
            mask = _kxor_mask64(cmp_mask, 0xFFFFFFFFFFFFFFFFULL);
            break;
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0xFFFF);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFULL);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512h v32hf_b = _mm512_set1_ph(100.0f);
    
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in bfloat16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in bfloat16
    #endif
    
    // Loop with control flow to prevent constant folding
    for (int i = 0; i < 8; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = (i % 2) ? 0xAAAAAAAA : 0x55555555;
        __mmask16 mask16 = _mm512_cmp_ps_mask(v16sf_a, _mm512_set1_ps(7.5f), _CMP_LT_OQ);
        __mmask8 mask8 = _mm512_cmp_pd_mask(v8df_a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Data dependency chain: use result from one blend to affect another
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use the 64qi result to generate a mask for 32hi blend
        __mmask32 mask_from_64qi = _mm512_cmpeq_epi16_mask(
            result64qi, 
            _mm512_set1_epi16(0xFF00)
        );
        
        // Combine masks using logical operations
        __mmask32 combined_mask32 = _kor_mask32(mask32, mask_from_64qi);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, combined_mask32);
        
        // If-else chain for control flow
        if (i < 4) {
            // Use comparison mask for integer blends
            __mmask16 mask16si = _mm512_cmpeq_epi32_mask(
                result32hi, 
                _mm512_set1_epi32(0xFFFF0000)
            );
            __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16si);
            
            // Horizontal sum to prevent optimization
            __m256i sum256 = _mm512_extracti64x4_epi64(result16si, 0);
            sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result16si, 1));
            __m128i sum128 = _mm256_extracti128_si256(sum256, 0);
            sum128 = _mm_add_epi64(sum128, _mm256_extracti128_si256(sum256, 1));
            checksum += _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
        } else {
            // Use different mask generation for float blends
            __mmask8 mask8di = _kand_mask8(mask8, 0xAA);
            __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8di);
            
            // Use integer result to influence float blend
            __mmask16 mask16sf = _mm512_cmp_ps_mask(
                _mm512_castsi512_ps(result8di),
                _mm512_set1_ps(0),
                _CMP_NEQ_OQ
            );
            __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16sf);
            
            // Horizontal sum for floats
            __m256 sum256 = _mm512_extractf32x8_ps(result16sf, 0);
            sum256 = _mm256_add_ps(sum256, _mm512_extractf32x8_ps(result16sf, 1));
            __m128 sum128 = _mm256_extractf128_ps(sum256, 0);
            sum128 = _mm_add_ps(sum128, _mm256_extractf128_ps(sum256, 1));
            checksum += (uint64_t)_mm_cvtss_f32(sum128);
        }
        
        // Always execute double precision blend
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        
        // Horizontal sum for doubles
        __m256d sum256d = _mm512_extractf64x4_pd(result8df, 0);
        sum256d = _mm256_add_pd(sum256d, _mm512_extractf64x4_pd(result8df, 1));
        __m128d sum128d = _mm256_extractf128_pd(sum256d, 0);
        sum128d = _mm_add_pd(sum128d, _mm256_extractf128_pd(sum256d, 1));
        checksum += (uint64_t)_mm_cvtsd_f64(sum128d);
        
        #ifdef __AVX512FP16__
        // Half precision blends with control flow
        if (i % 3 == 0) {
            __mmask32 mask32hf = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_LT_OQ);
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32hf);
            
            // Convert to float for checksum
            __m512 result32hf_f32 = _mm512_cvtph_ps(result32hf);
            __m256 sum256hf = _mm512_extractf32x8_ps(result32hf_f32, 0);
            sum256hf = _mm256_add_ps(sum256hf, _mm512_extractf32x8_ps(result32hf_f32, 1));
            __m128 sum128hf = _mm256_extractf128_ps(sum256hf, 0);
            sum128hf = _mm_add_ps(sum128hf, _mm256_extractf128_ps(sum256hf, 1));
            checksum += (uint64_t)_mm_cvtss_f32(sum128hf);
        }
        
        // Brain float blend
        __mmask32 mask32bf = 0xAAAAAAAA; // Immediate mask
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32bf);
        
        // Convert to float for checksum
        __m512 result32bf_f32 = _mm512_cvtpbh_ps(result32bf);
        __m256 sum256bf = _mm512_extractf32x8_ps(result32bf_f32, 0);
        sum256bf = _mm256_add_ps(sum256bf, _mm512_extractf32x8_ps(result32bf_f32, 1));
        __m128 sum128bf = _mm256_extractf128_ps(sum256bf, 0);
        sum128bf = _mm_add_ps(sum128bf, _mm256_extractf128_ps(sum256bf, 1));
        checksum += (uint64_t)_mm_cvtss_f32(sum128bf);
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
