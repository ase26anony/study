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

// Helper function to generate masks based on control flow
__mmask64 generate_dynamic_mask64(int iteration) {
    // Use different mask generation methods based on iteration
    switch (iteration % 3) {
        case 0: {
            // Comparison mask
            __m512i v1 = _mm512_set1_epi8(iteration);
            __m512i v2 = _mm512_set1_epi8(iteration / 2);
            return _mm512_cmpeq_epi8_mask(v1, v2);
        }
        case 1: {
            // Immediate mask with pattern
            return (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (iteration * 0x1111111111111111ULL));
        }
        default: {
            // Logical combination of masks
            __mmask64 m1 = (__mmask64)0x5555555555555555ULL;
            __mmask64 m2 = (__mmask64)(0xFFFFFFFFFFFFFFFFULL >> (iteration % 64));
            return _kor_mask64(m1, m2);
        }
    }
}

__mmask32 generate_dynamic_mask32(int iteration) {
    switch (iteration % 3) {
        case 0: {
            __m512i v1 = _mm512_set1_epi16(iteration);
            __m512i v2 = _mm512_set1_epi16(iteration * 2);
            return _mm512_cmplt_epi16_mask(v1, v2);
        }
        case 1:
            return (__mmask32)(0xAAAAAAAA ^ (iteration * 0x11111111));
        default:
            return _kor_mask32(0x55555555, 0xAAAAAAAA >> (iteration % 32));
    }
}

__mmask16 generate_dynamic_mask16(int iteration) {
    switch (iteration % 3) {
        case 0: {
            __m512 a = _mm512_set1_ps(iteration * 0.1f);
            __m512 b = _mm512_set1_ps(iteration * 0.2f);
            return _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        }
        case 1:
            return (__mmask16)(0xAAAA ^ (iteration * 0x1111));
        default:
            return _kxor_mask16(0x5555, 0xAAAA >> (iteration % 16));
    }
}

__mmask8 generate_dynamic_mask8(int iteration) {
    switch (iteration % 3) {
        case 0: {
            __m512d a = _mm512_set1_pd(iteration * 0.1);
            __m512d b = _mm512_set1_pd(iteration * 0.3);
            return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        }
        case 1:
            return (__mmask8)(0xAA ^ (iteration * 0x11));
        default:
            return _kand_mask8(0x55, 0xAA >> (iteration % 8));
    }
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
    
    __m512i v64qi_b = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f,
        7.0f,6.5f,6.0f,5.5f,5.0f,4.5f,4.0f,3.5f,
        3.0f,2.5f,2.0f,1.5f,1.0f,0.5f,0.0f,-0.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        -0.5f,0.0f,0.5f,1.0f,1.5f,2.0f,2.5f,3.0f,
        3.5f,4.0f,4.5f,5.0f,5.5f,6.0f,6.5f,7.0f,
        7.5f,8.0f,8.5f,9.0f,9.5f,10.0f,10.5f,11.0f,
        11.5f,12.0f,12.5f,13.0f,13.5f,14.0f,14.5f,15.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        31.0f,30.5f,30.0f,29.5f,29.0f,28.5f,28.0f,27.5f,
        27.0f,26.5f,26.0f,25.5f,25.0f,24.5f,24.0f,23.5f,
        23.0f,22.5f,22.0f,21.5f,21.0f,20.5f,20.0f,19.5f,
        19.0f,18.5f,18.0f,17.5f,17.0f,16.5f,16.0f,15.5f,
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f,
        7.0f,6.5f,6.0f,5.5f,5.0f,4.5f,4.0f,3.5f,
        3.0f,2.5f,2.0f,1.5f,1.0f,0.5f,0.0f,-0.5f
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        -0.5f,0.0f,0.5f,1.0f,1.5f,2.0f,2.5f,3.0f,
        3.5f,4.0f,4.5f,5.0f,5.5f,6.0f,6.5f,7.0f,
        7.5f,8.0f,8.5f,9.0f,9.5f,10.0f,10.5f,11.0f,
        11.5f,12.0f,12.5f,13.0f,13.5f,14.0f,14.5f,15.0f,
        15.5f,16.0f,16.5f,17.0f,17.5f,18.0f,18.5f,19.0f,
        19.5f,20.0f,20.5f,21.0f,21.5f,22.0f,22.5f,23.0f,
        23.5f,24.0f,24.5f,25.0f,25.5f,26.0f,26.5f,27.0f,
        27.5f,28.0f,28.5f,29.0f,29.5f,30.0f,30.5f,31.0f
    );
    
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_set1_epi16(0x3C00)); // 1.0
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_set1_epi16(0x4000)); // 2.0
#endif
    
    // Complex control flow with data dependency chains
    for (int i = 0; i < 10; i++) {
        // Generate dynamic masks based on iteration
        __mmask64 mask64 = generate_dynamic_mask64(i);
        __mmask32 mask32 = generate_dynamic_mask32(i);
        __mmask16 mask16 = generate_dynamic_mask16(i);
        __mmask8 mask8 = generate_dynamic_mask8(i);
        
        // Perform blends with data dependency between modes
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use result from previous blend to generate new mask
        __mmask32 mask_from_prev = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(result64qi, _mm512_set1_epi16(0x00FF)),
            _mm512_set1_epi16(0)
        );
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, 
            _kor_mask32(mask32, mask_from_prev));
        
        // More complex dependency chain
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
        
        // Create mask from integer result for float blend
        __mmask16 float_mask = _mm512_cmpeq_epi32_mask(
            result16si,
            _mm512_set1_epi32(i)
        );
        
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, 
            _kxor_mask16(mask16, float_mask));
        
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        
#ifdef __AVX512FP16__
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
        
        // Accumulate checksums to prevent optimization removal
        // Horizontal reductions
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_setzero_si512());
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_setzero_si512());
        
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_setzero_ps());
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_setzero_pd());
        
        // Extract and accumulate to checksum
        uint64_t temp[8];
        _mm512_storeu_si512(temp, sum64qi);
        checksum += temp[0] + temp[4];
        
        _mm512_storeu_si512(temp, sum32hi);
        checksum += temp[0] + temp[4] + temp[8] + temp[12];
        
        _mm512_storeu_si512(temp, sum16si);
        for (int j = 0; j < 8; j++) checksum += temp[j];
        
        _mm512_storeu_si512(temp, sum8di);
        for (int j = 0; j < 8; j++) checksum += temp[j];
        
        float fsum[16];
        _mm512_storeu_ps(fsum, sum16sf);
        for (int j = 0; j < 16; j++) checksum += (uint64_t)fsum[j];
        
        double dsum[8];
        _mm512_storeu_pd(dsum, sum8df);
        for (int j = 0; j < 8; j++) checksum += (uint64_t)dsum[j];
        
#ifdef __AVX512FP16__
        // For half precision, we need to convert to float for accumulation
        __m512 conv32hf = _mm512_cvtph_ps(result32hf);
        __m512 sum32hf = _mm512_add_ps(conv32hf, _mm512_setzero_ps());
        _mm512_storeu_ps(fsum, sum32hf);
        for (int j = 0; j < 16; j++) checksum += (uint64_t)fsum[j];
#endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
