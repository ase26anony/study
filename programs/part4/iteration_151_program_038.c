#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper function for V64QImode blends
static inline __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

// Helper function for V32HImode blends  
static inline __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}

// Helper function for V16SImode blends
static inline __m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

// Helper function for V8DImode blends
static inline __m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

// Helper function for V16SFmode blends
static inline __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

// Helper function for V8DFmode blends
static inline __m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
// Helper function for V32HFmode blends
static inline __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}

// Helper function for V32BFmode blends (same intrinsic as HF)
static inline __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

// Chained blend operation: epi8 -> epi16 -> ps
static inline float chain_blend_operations(int selector) {
    // Initialize vectors with distinct patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    // Generate mask using different methods based on selector
    __mmask64 mask64;
    if (selector & 1) {
        // Immediate constant mask
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(32);
        mask64 = _mm512_cmpgt_epi8_mask(v64qi_a, cmp_a);
    }
    
    // First blend: V64QImode
    __m512i result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Convert to V32HImode for next blend
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    // Generate mask using bitwise operations
    __mmask32 mask32 = _kor_mask32(
        _mm512_cmpgt_epi16_mask(v32hi_a, _mm512_set1_epi16(15)),
        selector ? 0x55555555 : 0xAAAAAAAA
    );
    
    // Second blend: V32HImode
    result = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Convert to V16SFmode for final blend
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    // Generate mask using comparison intrinsic
    __mmask16 mask16 = _mm512_cmp_ps_mask(v16sf_a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
    
    // Third blend: V16SFmode
    __m512 float_result = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(float_result);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors for each mode
    __m512i v64qi_a = _mm512_set1_epi8(1);
    __m512i v64qi_b = _mm512_set1_epi8(2);
    
    __m512i v32hi_a = _mm512_set1_epi16(10);
    __m512i v32hi_b = _mm512_set1_epi16(20);
    
    __m512i v16si_a = _mm512_set1_epi32(100);
    __m512i v16si_b = _mm512_set1_epi32(200);
    
    __m512i v8di_a = _mm512_set1_epi64(1000);
    __m512i v8di_b = _mm512_set1_epi64(2000);
    
    __m512 v16sf_a = _mm512_set1_ps(1.5f);
    __m512 v16sf_b = _mm512_set1_ps(2.5f);
    
    __m512d v8df_a = _mm512_set1_pd(10.5);
    __m512d v8df_b = _mm512_set1_pd(20.5);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    __m512bh v32bf_a = _mm512_set1_ph(1.0f);
    __m512bh v32bf_b = _mm512_set1_ph(2.0f);
#endif
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        // Varied mask generation patterns
        __mmask64 mask64;
        if (i % 3 == 0) {
            mask64 = 0x5555555555555555ULL;  // Immediate constant
        } else if (i % 3 == 1) {
            // Dynamic mask from comparison
            mask64 = _mm512_cmpeq_epi8_mask(v64qi_a, _mm512_set1_epi8(1));
        } else {
            // Bitwise operation on existing mask
            __mmask64 temp = _mm512_cmplt_epi8_mask(v64qi_a, v64qi_b);
            mask64 = _knot_mask64(temp);
        }
        
        // V64QImode blend
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // V32HImode blend with different mask pattern
        __mmask32 mask32 = (i % 2) ? 0xAAAAAAAA : 0x55555555;
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        
        // Data-dependent selection between blend types
        if (i > 5) {
            // V16SImode blend with comparison mask
            __mmask16 mask16 = _mm512_cmpgt_epi32_mask(v16si_a, v16si_b);
            __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
            
            // V8DImode blend
            __mmask8 mask8 = 0xAA;
            __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
            
            // Add to checksum
            long long sum64qi = _mm512_reduce_add_epi64(result64qi);
            long long sum32hi = _mm512_reduce_add_epi64(result32hi);
            long long sum16si = _mm512_reduce_add_epi64(result16si);
            long long sum8di = _mm512_reduce_add_epi64(result8di);
            
            checksum += sum64qi + sum32hi + sum16si + sum8di;
        } else {
            // V16SFmode blend
            __mmask16 mask16f = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
            __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16f);
            
            // V8DFmode blend
            __mmask8 mask8d = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
            __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8d);
            
            // Add to checksum
            float sum16sf = _mm512_reduce_add_ps(result16sf);
            double sum8df = _mm512_reduce_add_pd(result8df);
            
            checksum += sum16sf + sum8df;
        }
        
#ifdef __AVX512FP16__
        // V32HFmode and V32BFmode blends (half precision)
        if (i % 4 == 0) {
            __mmask32 mask32hf = 0x55555555;
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32hf);
            __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32hf);
            
            // Simple reduction for half precision
            __m256h low = _mm512_castph512_ph256(result32hf);
            __m256h high = _mm512_extractf32x8_ps(_mm512_castph_ps(result32hf), 1);
            // Add some contribution to checksum
            checksum += 0.1;
        }
#endif
        
        // Call chained blend function
        checksum += chain_blend_operations(i);
    }
    
    // Print final checksum
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
