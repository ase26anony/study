#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Function declarations with noinline to ensure separate expansion
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

// Helper function to generate masks with control flow
__mmask64 generate_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow to generate different masks
    if (iteration % 3 == 0) {
        // Comparison mask
        __m512i v1 = _mm512_set1_epi8(iteration);
        __m512i v2 = _mm512_set1_epi8(iteration / 2);
        mask = _mm512_cmpeq_epi8_mask(v1, v2);
    } else if (iteration % 3 == 1) {
        // Immediate mask pattern
        mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL);
    } else {
        // Logical combination of masks
        __m512i v1 = _mm512_set1_epi8(iteration);
        __m512i v2 = _mm512_set1_epi8(100);
        __mmask64 m1 = _mm512_cmpgt_epi8_mask(v1, v2);
        __mmask64 m2 = (__mmask64)(0x5555555555555555ULL);
        mask = _kor_mask64(m1, m2);
    }
    
    return mask;
}

__mmask32 generate_mask32(int iteration) {
    __mmask32 mask;
    
    switch (iteration % 4) {
        case 0:
            mask = (__mmask32)0xAAAAAAAA;
            break;
        case 1: {
            __m512i v1 = _mm512_set1_epi16(iteration);
            __m512i v2 = _mm512_set1_epi16(50);
            mask = _mm512_cmpgt_epi16_mask(v1, v2);
            break;
        }
        case 2: {
            __m512i v1 = _mm512_set1_epi16(iteration);
            __m512i v2 = _mm512_set1_epi16(iteration * 2);
            mask = _mm512_cmpeq_epi16_mask(v1, v2);
            break;
        }
        default: {
            __mmask32 m1 = (__mmask32)0xCCCCCCCC;
            __mmask32 m2 = (__mmask32)0x33333333;
            mask = _kxor_mask32(m1, m2);
            break;
        }
    }
    
    return mask;
}

__mmask16 generate_mask16(int iteration) {
    __mmask16 mask;
    
    // If-else chain for mask generation
    if (iteration < 10) {
        mask = (__mmask16)0xAAAA;
    } else if (iteration < 20) {
        __m512i v1 = _mm512_set1_epi32(iteration);
        __m512i v2 = _mm512_set1_epi32(15);
        mask = _mm512_cmpgt_epi32_mask(v1, v2);
    } else {
        __mmask16 m1 = (__mmask16)0xF0F0;
        __mmask16 m2 = (__mmask16)0x0F0F;
        mask = _kand_mask16(m1, m2);
    }
    
    return mask;
}

__mmask8 generate_mask8(int iteration) {
    __mmask8 mask;
    
    // Loop-dependent mask
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            mask = (__mmask8)(iteration & 0xFF);
        } else {
            __m512i v1 = _mm512_set1_epi64(iteration);
            __m512i v2 = _mm512_set1_epi64(30);
            __mmask8 m = _mm512_cmpgt_epi64_mask(v1, v2);
            mask = _kor_mask8(mask, m);
        }
    }
    
    return mask;
}

// Data dependency chain: use result from one blend to generate mask for another
float process_data_dependency_chain(int iteration) {
    // Start with integer blend
    __m512i int_vec1 = _mm512_set1_epi8(iteration);
    __m512i int_vec2 = _mm512_set1_epi8(iteration + 1);
    __mmask64 mask64 = generate_mask64(iteration);
    __m512i int_result = blend_v64qi(int_vec1, int_vec2, mask64);
    
    // Use integer result to generate float mask
    __m512i cmp_vec = _mm512_set1_epi8(iteration + 50);
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(int_result, cmp_vec);
    
    // Convert mask64 to mask16 for float blend (taking every 4th byte)
    __mmask16 float_mask = (__mmask16)(cmp_mask & 0xFFFF);
    
    // Perform float blend
    __m512 float_vec1 = _mm512_set1_ps(iteration * 1.0f);
    __m512 float_vec2 = _mm512_set1_ps(iteration * 2.0f);
    __m512 float_result = blend_v16sf(float_vec1, float_vec2, float_mask);
    
    // Horizontal sum of float result
    float sum = _mm512_reduce_add_ps(float_result);
    
    return sum;
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_1 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_2 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v32hi_1 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_2 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v16si_1 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_2 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v8di_1 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_2 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 v16sf_1 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_2 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512d v8df_1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_2 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 10; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_mask64(i);
        __mmask32 mask32 = generate_mask32(i);
        __mmask16 mask16 = generate_mask16(i);
        __mmask8 mask8 = generate_mask8(i);
        
        // Call blend functions for all modes
        __m512i result64qi = blend_v64qi(v64qi_1, v64qi_2, mask64);
        __m512i result32hi = blend_v32hi(v32hi_1, v32hi_2, mask32);
        __m512i result16si = blend_v16si(v16si_1, v16si_2, mask16);
        __m512i result8di = blend_v8di(v8di_1, v8di_2, mask8);
        __m512 result16sf = blend_v16sf(v16sf_1, v16sf_2, mask16);
        __m512d result8df = blend_v8df(v8df_1, v8df_2, mask8);
        
        // Process data dependency chain
        float chain_result = process_data_dependency_chain(i);
        
        // Horizontal reductions to prevent optimization removal
        __m128i sum64qi = _mm512_reduce_add_epi64(result64qi);
        __m128i sum32hi = _mm512_reduce_add_epi64(result32hi);
        __m128i sum16si = _mm512_reduce_add_epi64(result16si);
        __m128i sum8di = _mm512_reduce_add_epi64(result8di);
        float sum16sf = _mm512_reduce_add_ps(result16sf);
        double sum8df = _mm512_reduce_add_pd(result8df);
        
        // Accumulate to checksum
        final_checksum += _mm_extract_epi64(sum64qi, 0);
        final_checksum += _mm_extract_epi64(sum32hi, 0);
        final_checksum += _mm_extract_epi64(sum16si, 0);
        final_checksum += _mm_extract_epi64(sum8di, 0);
        final_checksum += (uint64_t)sum16sf;
        final_checksum += (uint64_t)sum8df;
        final_checksum += (uint64_t)chain_result;
        
        // Modify vectors for next iteration
        v64qi_1 = _mm512_add_epi8(v64qi_1, _mm512_set1_epi8(1));
        v16sf_1 = _mm512_add_ps(v16sf_1, _mm512_set1_ps(1.0f));
    }
    
#ifdef __AVX512FP16__
    // Test half-precision blends if supported
    __m512h v32hf_1 = _mm512_set1_ph(1.0f);
    __m512h v32hf_2 = _mm512_set1_ph(2.0f);
    __m512bh v32bf_1 = _mm512_set1_ph(1.0f);
    __m512bh v32bf_2 = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 5; i++) {
        __mmask32 mask32 = generate_mask32(i);
        __m512h result32hf = blend_v32hf(v32hf_1, v32hf_2, mask32);
        __m512bh result32bf = blend_v32bf(v32bf_1, v32bf_2, mask32);
        
        // Convert to float for reduction
        __m512 float_hf = _mm512_cvtph_ps(result32hf);
        __m512 float_bf = _mm512_cvtph_ps(result32bf);
        
        float sum_hf = _mm512_reduce_add_ps(float_hf);
        float sum_bf = _mm512_reduce_add_ps(float_bf);
        
        final_checksum += (uint64_t)sum_hf;
        final_checksum += (uint64_t)sum_bf;
    }
#endif
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
