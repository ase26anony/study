#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each blend gets expanded independently
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Complex mask generation with control flow
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow to generate different mask patterns
    if (iteration % 3 == 0) {
        // Immediate mask pattern
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Pattern based on iteration
        mask = (iteration & 1) ? 0x5555555555555555ULL : 0xFFFFFFFFFFFFFFFFULL;
    } else {
        // Alternating pattern
        mask = 0xCCCCCCCCCCCCCCCCULL;
    }
    
    // Apply logical operations
    if (iteration > 10) {
        mask = _kor_mask64(mask, 0xF0F0F0F0F0F0F0F0ULL);
    }
    
    return mask;
}

// Data dependency chain: result from one blend influences next blend
float process_blend_chain(int mode_selector) {
    float checksum = 0.0f;
    
    // Initialize vectors with patterned data
    __m512i vi1 = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i vi2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512 vf1 = _mm512_set_ps(31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
                               23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f);
    __m512 vf2 = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                               7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
    
    __m512d vd1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d vd2 = _mm512_set_pd(15.0,14.0,13.0,12.0,11.0,10.0,9.0,8.0);
    
    // Switch statement for control flow (Requirement 3)
    switch(mode_selector) {
        case 0: {
            // V64QImode blend
            __mmask64 mask64 = generate_complex_mask64(mode_selector);
            __m512i result64qi = blend_v64qi(vi1, vi2, mask64);
            
            // Use result to generate mask for next blend
            __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result64qi, _mm512_setzero_si512());
            __m512i result32hi = blend_v32hi(vi1, vi2, mask32);
            
            // Horizontal sum for checksum
            checksum += (float)_mm512_reduce_add_epi16(result32hi);
            break;
        }
        
        case 1: {
            // V32HImode blend with comparison mask
            __mmask32 mask32 = _mm512_cmpeq_epi16_mask(vi1, vi2);
            __m512i result32hi = blend_v32hi(vi1, vi2, mask32);
            
            // Use result to generate mask for float blend
            __mmask16 mask16 = _mm512_cmpeq_epi32_mask(result32hi, _mm512_setzero_si512());
            __m512 result16sf = blend_v16sf(vf1, vf2, mask16);
            
            // Horizontal sum
            checksum += _mm512_reduce_add_ps(result16sf);
            break;
        }
        
        case 2: {
            // V16SImode blend with immediate mask
            __mmask16 mask16 = 0xAAAA;  // Alternating pattern
            __m512i result16si = blend_v16si(vi1, vi2, mask16);
            
            // Logical mask operation
            __mmask8 mask8 = _kor_mask8(0xAA, 0x55);  // All ones
            __m512i result8di = blend_v8di(vi1, vi2, mask8);
            
            checksum += (float)_mm512_reduce_add_epi64(result8di);
            break;
        }
        
        case 3: {
            // V16SFmode blend with comparison mask
            __mmask16 mask16 = _mm512_cmp_ps_mask(vf1, vf2, _CMP_LT_OS);
            __m512 result16sf = blend_v16sf(vf1, vf2, mask16);
            
            // Use float result to influence double blend
            __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result16sf), 
                                               _mm512_setzero_pd(), _CMP_NEQ_OQ);
            __m512d result8df = blend_v8df(vd1, vd2, mask8);
            
            checksum += (float)_mm512_reduce_add_pd(result8df);
            break;
        }
        
        case 4: {
            // V8DFmode blend
            __mmask8 mask8 = _mm512_cmp_pd_mask(vd1, vd2, _CMP_GT_OS);
            __m512d result8df = blend_v8df(vd1, vd2, mask8);
            
            // Chain back to integer blend
            __mmask16 mask16 = _mm512_cmp_ps_mask(_mm512_castpd_ps(result8df), 
                                                 _mm512_setzero_ps(), _CMP_NEQ_OQ);
            __m512 result16sf = blend_v16sf(vf1, vf2, mask16);
            
            checksum += _mm512_reduce_add_ps(result16sf);
            break;
        }
        
        default:
            // Default case with multiple blends
            __mmask64 mask64 = 0xFFFFFFFFFFFFFFFFULL;
            __m512i result64qi = blend_v64qi(vi1, vi2, mask64);
            checksum += (float)_mm512_reduce_add_epi8(result64qi);
            break;
    }
    
    return checksum;
}

#ifdef __AVX512FP16__
float process_half_precision_blends() {
    float checksum = 0.0f;
    
    // Initialize half-precision vectors
    __m512h vh1 = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512h vh2 = _mm512_set_ph(
        63.0f,62.0f,61.0f,60.0f,59.0f,58.0f,57.0f,56.0f,
        55.0f,54.0f,53.0f,52.0f,51.0f,50.0f,49.0f,48.0f,
        47.0f,46.0f,45.0f,44.0f,43.0f,42.0f,41.0f,40.0f,
        39.0f,38.0f,37.0f,36.0f,35.0f,34.0f,33.0f,32.0f
    );
    
    // V32HFmode blend
    __mmask32 mask32 = 0xAAAAAAAA;  // Alternating pattern
    __m512h result32hf = blend_v32hf(vh1, vh2, mask32);
    
    // V32BFmode blend (brain float)
    __m512bh vbh1 = _mm512_castph_bh(vh1);
    __m512bh vbh2 = _mm512_castph_bh(vh2);
    __m512bh result32bf = blend_v32bf(vbh1, vbh2, mask32);
    
    // Convert back to float for checksum
    __m512h temp = _mm512_castbh_ph(result32bf);
    
    // Simple reduction
    for (int i = 0; i < 32; i++) {
        checksum += ((_Float16*)&result32hf)[i];
        checksum += ((_Float16*)&temp)[i];
    }
    
    return checksum;
}
#endif

int main() {
    float total_checksum = 0.0f;
    
    // Loop with control flow (Requirement 3)
    for (int i = 0; i < 10; i++) {
        // If-else chain where result influences next operation
        if (i < 5) {
            total_checksum += process_blend_chain(i % 5);
        } else {
            // Different blend mode sequence
            __m512i vi1 = _mm512_set1_epi32(i);
            __m512i vi2 = _mm512_set1_epi32(i * 2);
            
            // Generate mask based on loop index
            __mmask16 mask16 = (i % 2 == 0) ? 0xFFFF : 0x0000;
            __m512i result16si = blend_v16si(vi1, vi2, mask16);
            
            // Data dependency: use integer result for float blend
            __m512 vf1 = _mm512_cvtepi32_ps(result16si);
            __m512 vf2 = _mm512_set1_ps(i * 3.0f);
            
            __mmask16 mask16f = _mm512_cmp_ps_mask(vf1, vf2, _CMP_LT_OS);
            __m512 result16sf = blend_v16sf(vf1, vf2, mask16f);
            
            total_checksum += _mm512_reduce_add_ps(result16sf);
        }
    }
    
    // Process all blend modes in sequence with data dependencies
    __m512i base_int = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    __m512 base_float = _mm512_cvtepi32_ps(base_int);
    
    // Chain: V64QImode -> V32HImode -> V16SImode -> V8DImode
    __mmask64 mask64 = generate_complex_mask64(0);
    __m512i result64qi = blend_v64qi(base_int, _mm512_setzero_si512(), mask64);
    
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result64qi, _mm512_setzero_si512());
    __m512i result32hi = blend_v32hi(base_int, _mm512_set1_epi16(100), mask32);
    
    __mmask16 mask16 = _mm512_cmpeq_epi32_mask(result32hi, _mm512_set1_epi32(100));
    __m512i result16si = blend_v16si(base_int, _mm512_set1_epi32(200), mask16);
    
    __mmask8 mask8 = _mm512_cmpeq_epi64_mask(result16si, _mm512_set1_epi64(200));
    __m512i result8di = blend_v8di(_mm512_set1_epi64(300), _mm512_set1_epi64(400), mask8);
    
    // Chain: V16SFmode -> V8DFmode
    __mmask16 mask16f = _mm512_cmp_ps_mask(base_float, _mm512_set1_ps(8.5f), _CMP_GT_OS);
    __m512 result16sf = blend_v16sf(base_float, _mm512_set1_ps(50.0f), mask16f);
    
    __m512d base_double = _mm512_cvtps_pd(_mm512_castps512_ps256(result16sf));
    __mmask8 mask8d = _mm512_cmp_pd_mask(base_double, _mm512_set1_pd(25.0), _CMP_LT_OS);
    __m512d result8df = blend_v8df(base_double, _mm512_set1_pd(100.0), mask8d);
    
    // Accumulate results to prevent optimization
    total_checksum += (float)_mm512_reduce_add_epi8(result64qi);
    total_checksum += (float)_mm512_reduce_add_epi16(result32hi);
    total_checksum += (float)_mm512_reduce_add_epi32(result16si);
    total_checksum += (float)_mm512_reduce_add_epi64(result8di);
    total_checksum += _mm512_reduce_add_ps(result16sf);
    total_checksum += (float)_mm512_reduce_add_pd(result8df);
    
#ifdef __AVX512FP16__
    // Process half-precision blends if supported
    total_checksum += process_half_precision_blends();
#endif
    
    printf("Final checksum: %f\n", total_checksum);
    return 0;
}
