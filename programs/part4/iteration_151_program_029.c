#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    // Chain epi8 -> epi16 blend
    __m512i blend1 = _mm512_mask_blend_epi8(k8, a, b);
    __m512i blend2 = _mm512_mask_blend_epi16(k16, blend1, b);
    return blend2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, 
                                      __mmask16 k32, __mmask8 k64) {
    // Chain ps -> pd blend
    __m512 blend1 = _mm512_mask_blend_ps(k32, a, b);
    __m512d blend2 = _mm512_mask_blend_pd(k64, c, d);
    // Convert pd result back to ps for mixing
    __m512 conv = _mm512_castpd_ps(blend2);
    return _mm512_add_ps(blend1, conv);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // Use same intrinsic for both HF and BF modes
    __m512h blend1 = _mm512_mask_blend_ph(k16, a, b);
    // Create complementary mask
    __mmask32 kcomp = _knot_mask32(k16);
    __m512h blend2 = _mm512_mask_blend_ph(kcomp, b, a);
    return _mm512_add_ph(blend1, blend2);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 k32, __mmask8 k64) {
    // Chain epi32 -> epi64 blend
    __m512i blend1 = _mm512_mask_blend_epi32(k32, a, b);
    __m512i blend2 = _mm512_mask_blend_epi64(k64, blend1, _mm512_add_epi32(a, b));
    return blend2;
}

// Function with data-dependent control flow
static __m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    if (mode & 1) {
        return _mm512_mask_blend_epi8(k8, a, b);
    } else {
        return _mm512_mask_blend_epi16(k16, a, b);
    }
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize source vectors with distinct patterns
    __m512i vi64 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi64_alt = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi32 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vi32_alt = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 vf32 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 vf32_alt = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512d vf64 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d vf64_alt = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    // Generate masks using different methods
    __mmask64 k8_imm = 0xAAAAAAAAAAAAAAAA;  // Immediate constant
    __mmask32 k16_imm = 0x55555555;          // Immediate constant
    
    // Dynamic masks from comparisons
    __mmask64 k8_cmp = _mm512_cmp_epi8_mask(vi64, vi64_alt, _MM_CMPINT_LT);
    __mmask32 k16_cmp = _mm512_cmp_epi16_mask(
        _mm512_set1_epi16(32), 
        _mm512_set1_epi16(16), 
        _MM_CMPINT_GT
    );
    
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(vi32, vi32_alt, _MM_CMPINT_GT);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(
        _mm512_set1_epi64(10), 
        _mm512_set1_epi64(5), 
        _MM_CMPINT_GT
    );
    
    __mmask16 k32f_cmp = _mm512_cmp_ps_mask(vf32, vf32_alt, _CMP_GT_OQ);
    __mmask8 k64f_cmp = _mm512_cmp_pd_mask(vf64, vf64_alt, _CMP_LT_OQ);
    
    // Create masks via bitwise operations
    __mmask64 k8_combined = _kor_mask64(k8_imm, k8_cmp);
    __mmask32 k16_combined = _kxor_mask32(k16_imm, k16_cmp);
    __mmask16 k32_combined = _knot_mask16(k32_cmp);
    __mmask8 k64_combined = _kor_mask8(k64_cmp, 0xAA);
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i result;
        if (i % 3 == 0) {
            // V64QImode blend
            result = _mm512_mask_blend_epi8(k8_combined, vi64, vi64_alt);
        } else if (i % 3 == 1) {
            // V32HImode blend
            result = _mm512_mask_blend_epi16(k16_combined, vi64, vi64_alt);
        } else {
            // Conditional blend
            result = conditional_blend(i, vi64, vi64_alt, k8_imm, k16_imm);
        }
        
        // Reduce and accumulate
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        final_checksum += _mm512_reduce_add_epi64(sum64);
    }
    
    // V16SImode blend
    __m512i blend16si = _mm512_mask_blend_epi32(k32_combined, vi32, vi32_alt);
    __m512i sum16si = _mm512_slli_epi32(blend16si, 1);  // Process result
    final_checksum += _mm512_reduce_add_epi64(sum16si);
    
    // V8DImode blend
    __m512i vi64_di = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i vi64_di_alt = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i blend8di = _mm512_mask_blend_epi64(k64_combined, vi64_di, vi64_di_alt);
    final_checksum += _mm512_reduce_add_epi64(blend8di);
    
    // V16SFmode blend
    __m512 blend16sf = _mm512_mask_blend_ps(k32f_cmp, vf32, vf32_alt);
    float sum16sf = _mm512_reduce_add_ps(blend16sf);
    final_checksum += (uint64_t)sum16sf;
    
    // V8DFmode blend
    __m512d blend8df = _mm512_mask_blend_pd(k64f_cmp, vf64, vf64_alt);
    __m512d sum8df = _mm512_add_pd(blend8df, vf64);
    double sum8df_scalar = _mm512_reduce_add_pd(sum8df);
    final_checksum += (uint64_t)sum8df_scalar;
    
    // Chained blend operations
    __m512i chain1 = blend_v64qi_v32hi(vi64, vi64_alt, k8_imm, k16_imm);
    final_checksum += _mm512_reduce_add_epi64(chain1);
    
    __m512 chain2 = blend_v16sf_v8df(vf32, vf32_alt, vf64, vf64_alt, k32f_cmp, k64f_cmp);
    final_checksum += (uint64_t)_mm512_reduce_add_ps(chain2);
    
    __m512i chain3 = blend_v16si_v8di(vi32, vi32_alt, k32_cmp, k64_cmp);
    final_checksum += _mm512_reduce_add_epi64(chain3);
    
#ifdef __AVX512FP16__
    // V32HFmode and V32BFmode blends (require AVX512FP16)
    __m512h vh32 = _mm512_set_ph(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
    );
    
    __m512h vh32_alt = _mm512_set_ph(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __mmask32 k16_half = _mm512_cmp_ph_mask(vh32, vh32_alt, _CMP_GT_OQ);
    
    // V32HFmode blend
    __m512h blend32hf = _mm512_mask_blend_ph(k16_half, vh32, vh32_alt);
    
    // V32BFmode blend (using same intrinsic with BF16 data)
    __m512h blend32bf = blend_v32hf_v32bf(vh32, vh32_alt, k16_half);
    
    // Reduce and accumulate
    __m512h sum_half = _mm512_add_ph(blend32hf, blend32bf);
    
    // Manual reduction for half precision
    __m256h low = _mm512_castph512_ph256(sum_half);
    __m256h high = _mm512_extractf32x8_ps(sum_half, 1);
    __m256h sum256 = _mm256_add_ph(low, high);
    
    // Further reduction
    __m128h sum128 = _mm_add_ph(_mm256_castph256_ph128(sum256), 
                               _mm256_extractf128_ph(sum256, 1));
    
    // Extract scalar (simplified - actual reduction would need more steps)
    uint16_t* half_data = (uint16_t*)&sum128;
    uint32_t half_sum = 0;
    for (int i = 0; i < 8; i++) {
        half_sum += half_data[i];
    }
    final_checksum += half_sum;
#endif
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
