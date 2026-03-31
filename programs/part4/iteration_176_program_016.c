#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each intrinsic gets expanded independently
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
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
__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to generate masks with control flow dependency
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    __m512i vec1 = _mm512_set1_epi8(iteration);
    __m512i vec2 = _mm512_set1_epi8(iteration % 256);
    
    // Comparison mask
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(vec1, vec2);
    
    // Immediate mask with pattern
    __mmask64 imm_mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    
    // Logical operation on masks
    if (iteration % 3 == 0) {
        return _kor_mask64(cmp_mask, imm_mask);
    } else if (iteration % 3 == 1) {
        return _kxor_mask64(cmp_mask, imm_mask);
    } else {
        return _kand_mask64(cmp_mask, imm_mask);
    }
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(float threshold, __m512 data) {
    __m512 threshold_vec = _mm512_set1_ps(threshold);
    __mmask32 cmp_mask = _mm512_cmp_ps_mask(data, threshold_vec, _CMP_GT_OQ);
    
    __mmask32 imm_mask = (__mmask32)0xAAAAAAAA;
    
    // Switch statement for different mask combinations
    switch ((int)threshold) {
        case 0: return _kor_mask32(cmp_mask, imm_mask);
        case 1: return _kxor_mask32(cmp_mask, imm_mask);
        default: return _kand_mask32(cmp_mask, imm_mask);
    }
}

__attribute__((noinline))
__mmask16 generate_complex_mask16_double(double threshold, __m512d data) {
    __m512d threshold_vec = _mm512_set1_pd(threshold);
    __mmask16 cmp_mask = _mm512_cmp_pd_mask(data, threshold_vec, _CMP_LT_OQ);
    
    __mmask16 imm_mask = (__mmask16)0xAAAA;
    
    // If-else chain for mask generation
    if (threshold < 0.5) {
        return _kor_mask16(cmp_mask, imm_mask);
    } else if (threshold < 1.0) {
        return _kxor_mask16(cmp_mask, imm_mask);
    } else {
        return _kand_mask16(cmp_mask, imm_mask);
    }
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        1.5f,1.4f,1.3f,1.2f,1.1f,1.0f,0.9f,0.8f,
        0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
    __m512d v8df_b = _mm512_set_pd(0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.0);
    
    // Loop with control flow for mask generation
    for (int i = 0; i < 10; i++) {
        // Generate masks with different methods
        __mmask64 mask64 = generate_complex_mask64(i);
        __mmask32 mask32 = generate_complex_mask32(i * 0.1f, v16sf_a);
        __mmask16 mask16_float = (__mmask16)generate_complex_mask32(i * 0.1f, v16sf_a);
        __mmask8 mask8 = (__mmask8)generate_complex_mask16_double(i * 0.1, v8df_a);
        
        // Perform blends with data dependency chain
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_float);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        
        // Use results to generate new masks for next iteration
        __mmask32 new_mask32 = _mm512_cmpeq_epi16_mask(result32hi, _mm512_set1_epi16(i));
        __mmask16 new_mask16 = _mm512_cmp_ps_mask(result16sf, _mm512_set1_ps(i * 0.1f), _CMP_EQ_OQ);
        
        // Perform additional blends with newly generated masks
        __m512i result32hi_2 = blend_v32hi(result32hi, v32hi_a, new_mask32);
        __m512 result16sf_2 = blend_v16sf(result16sf, v16sf_a, new_mask16);
        
        // Horizontal reduction to prevent optimization removal
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi_2, _mm512_set1_epi16(1));
        __m512 sum16sf = _mm512_add_ps(result16sf, result16sf_2);
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_set1_pd(1.0));
        
        // Accumulate to checksum
        checksum += _mm512_reduce_add_epi64(sum64qi);
        checksum += _mm512_reduce_add_epi32(sum32hi);
        checksum += (uint64_t)_mm512_reduce_add_ps(sum16sf);
        checksum += (uint64_t)_mm512_reduce_add_pd(sum8df);
        
        // Update input data for next iteration
        v64qi_a = result64qi;
        v32hi_a = result32hi_2;
        v16sf_a = result16sf_2;
    }
    
#ifdef __AVX512FP16__
    // Half-precision float blends (requires -mavx512fp16)
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_set1_bh(1.0f);
    __m512bh v32bf_b = _mm512_set1_bh(2.0f);
    
    __mmask32 mask_half = (__mmask32)0xAAAAAAAA;
    
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
    
    // Add half-precision results to checksum
    __m512h sum_half = _mm512_add_ph(result32hf, result32bf);
    float half_sum = _mm512_reduce_add_ph(sum_half);
    checksum += (uint64_t)half_sum;
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
