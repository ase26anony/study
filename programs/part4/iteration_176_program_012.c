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

// Control flow that prevents constant folding
__attribute__((noinline))
__mmask64 generate_dynamic_mask64(int iteration) {
    __m512i pattern1 = _mm512_set1_epi8(iteration % 256);
    __m512i pattern2 = _mm512_set1_epi8((iteration * 7) % 256);
    
    // Comparison mask - dynamic based on iteration
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(pattern1, pattern2);
    
    // Immediate mask - constant pattern
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
__mmask32 generate_dynamic_mask32(int iteration) {
    __m512i pattern1 = _mm512_set1_epi16(iteration % 65536);
    __m512i pattern2 = _mm512_set1_epi16((iteration * 13) % 65536);
    
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(pattern1, pattern2);
    __mmask32 imm_mask = (__mmask32)0xAAAAAAAA;
    
    switch (iteration % 4) {
        case 0: return _kor_mask32(cmp_mask, imm_mask);
        case 1: return _kxor_mask32(cmp_mask, imm_mask);
        case 2: return _kand_mask32(cmp_mask, imm_mask);
        default: return _knot_mask32(imm_mask);
    }
}

__attribute__((noinline))
__mmask16 generate_dynamic_mask16_float(int iteration) {
    __m512 pattern1 = _mm512_set1_ps((float)iteration);
    __m512 pattern2 = _mm512_set1_ps((float)(iteration * 2));
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(pattern1, pattern2, _CMP_LT_OQ);
    __mmask16 imm_mask = (__mmask16)0xAAAA;
    
    if (iteration % 2 == 0) {
        return _kor_mask16(cmp_mask, imm_mask);
    } else {
        return _kxor_mask16(cmp_mask, imm_mask);
    }
}

__attribute__((noinline))
__mmask8 generate_dynamic_mask8_double(int iteration) {
    __m512d pattern1 = _mm512_set1_pd((double)iteration);
    __m512d pattern2 = _mm512_set1_pd((double)(iteration * 3));
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(pattern1, pattern2, _CMP_GT_OQ);
    __mmask8 imm_mask = (__mmask8)0xAA;
    
    return _kor_mask8(cmp_mask, imm_mask);
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
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
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d v8df_b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        (__fp16)0.0,(__fp16)1.0,(__fp16)2.0,(__fp16)3.0,
        (__fp16)4.0,(__fp16)5.0,(__fp16)6.0,(__fp16)7.0,
        (__fp16)8.0,(__fp16)9.0,(__fp16)10.0,(__fp16)11.0,
        (__fp16)12.0,(__fp16)13.0,(__fp16)14.0,(__fp16)15.0,
        (__fp16)16.0,(__fp16)17.0,(__fp16)18.0,(__fp16)19.0,
        (__fp16)20.0,(__fp16)21.0,(__fp16)22.0,(__fp16)23.0,
        (__fp16)24.0,(__fp16)25.0,(__fp16)26.0,(__fp16)27.0,
        (__fp16)28.0,(__fp16)29.0,(__fp16)30.0,(__fp16)31.0
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        (__fp16)31.0,(__fp16)30.0,(__fp16)29.0,(__fp16)28.0,
        (__fp16)27.0,(__fp16)26.0,(__fp16)25.0,(__fp16)24.0,
        (__fp16)23.0,(__fp16)22.0,(__fp16)21.0,(__fp16)20.0,
        (__fp16)19.0,(__fp16)18.0,(__fp16)17.0,(__fp16)16.0,
        (__fp16)15.0,(__fp16)14.0,(__fp16)13.0,(__fp16)12.0,
        (__fp16)11.0,(__fp16)10.0,(__fp16)9.0,(__fp16)8.0,
        (__fp16)7.0,(__fp16)6.0,(__fp16)5.0,(__fp16)4.0,
        (__fp16)3.0,(__fp16)2.0,(__fp16)1.0,(__fp16)0.0
    );
    
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_set1_epi16(0x3C00)); // 1.0 in bfloat16
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_set1_epi16(0x4000)); // 2.0 in bfloat16
    #endif
    
    // Loop with non-trivial control flow to prevent optimization
    for (int i = 0; i < 10; i++) {
        // Generate dynamic masks based on iteration
        __mmask64 mask64 = generate_dynamic_mask64(i);
        __mmask32 mask32 = generate_dynamic_mask32(i);
        __mmask16 mask16_int = (__mmask16)generate_dynamic_mask32(i);
        __mmask8 mask8_int = (__mmask8)generate_dynamic_mask32(i);
        __mmask16 mask16_float = generate_dynamic_mask16_float(i);
        __mmask8 mask8_double = generate_dynamic_mask8_double(i);
        
        // Perform blends for all modes
        __m512i result_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result_v32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result_v16si = blend_v16si(v16si_a, v16si_b, mask16_int);
        __m512i result_v8di = blend_v8di(v8di_a, v8di_b, mask8_int);
        __m512 result_v16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        __m512d result_v8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
        #ifdef __AVX512FP16__
        __m512h result_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
        __m512bh result_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
        #endif
        
        // Data dependency chain: use result from one blend to generate mask for another
        if (i > 0) {
            // Use integer blend result to generate float mask
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(
                result_v16si, 
                _mm512_set1_epi32(i)
            );
            result_v16sf = blend_v16sf(result_v16sf, v16sf_b, new_mask);
        }
        
        // Compute horizontal sums to prevent optimization
        __m512i sum64 = _mm512_sad_epu8(result_v64qi, _mm512_setzero_si512());
        __m512i sum32 = _mm512_add_epi16(result_v32hi, _mm512_srli_epi16(result_v32hi, 8));
        __m512i sum16 = _mm512_add_epi32(result_v16si, _mm512_srli_epi32(result_v16si, 16));
        __m512i sum8 = _mm512_add_epi64(result_v8di, _mm512_srli_epi64(result_v8di, 32));
        
        __m512 sum16sf = _mm512_add_ps(result_v16sf, _mm512_permute_ps(result_v16sf, 0xB1));
        __m512d sum8df = _mm512_add_pd(result_v8df, _mm512_permute_pd(result_v8df, 0x55));
        
        // Accumulate to checksum
        uint64_t temp[8];
        _mm512_storeu_si512(temp, sum64);
        checksum += temp[0] + temp[4];
        
        _mm512_storeu_si512(temp, sum32);
        checksum += temp[0] + temp[4];
        
        _mm512_storeu_si512(temp, sum16);
        checksum += temp[0] + temp[4];
        
        _mm512_storeu_si512(temp, sum8);
        checksum += temp[0] + temp[4];
        
        float fsum[16];
        _mm512_storeu_ps(fsum, sum16sf);
        checksum += (uint64_t)fsum[0];
        
        double dsum[8];
        _mm512_storeu_pd(dsum, sum8df);
        checksum += (uint64_t)dsum[0];
        
        #ifdef __AVX512FP16__
        // For half-precision, convert to float for checksum
        __m512 v32hf_as_float = _mm512_cvtph_ps(result_v32hf);
        __m512 v32bf_as_float = _mm512_cvtpbh_ps(result_v32bf);
        
        float hsum[16];
        _mm512_storeu_ps(hsum, _mm512_add_ps(v32hf_as_float, v32bf_as_float));
        checksum += (uint64_t)hsum[0];
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
