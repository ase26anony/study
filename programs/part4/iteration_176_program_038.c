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

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi_alt(__m512i a, __m512i b, __mmask32 k) {
    // Alternative implementation with mask combination
    __mmask32 k2 = _kor_mask32(k, 0x55555555);
    return _mm512_mask_blend_epi16(k2, a, b);
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

// Control flow to prevent constant folding
__attribute__((noinline))
__mmask64 generate_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    switch (iteration % 4) {
        case 0:
            // Immediate mask
            mask = 0xAAAAAAAAAAAAAAAAULL;
            break;
        case 1:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = 0xCCCCCCCCCCCCCCCCULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
        default:
            // Pattern based on iteration
            mask = (iteration & 1) ? 0xFFFFFFFFFFFFFFFFULL : 0x0;
            break;
    }
    
    return mask;
}

__attribute__((noinline))
__mmask32 generate_mask32_hi(int iteration, __m512i data) {
    if (iteration < 2) {
        // Immediate mask
        return 0xAAAAAAAA;
    } else {
        // Comparison mask
        return _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(0));
    }
}

__attribute__((noinline))
__mmask16 generate_mask16_sf(int iteration, __m512 data) {
    __mmask16 mask;
    
    if (iteration % 3 == 0) {
        // Immediate mask
        mask = 0xAAAA;
    } else if (iteration % 3 == 1) {
        // Comparison mask
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.0f), _CMP_EQ_OQ);
    } else {
        // Logical operation on masks
        __mmask16 m1 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.0f), _CMP_LT_OQ);
        __mmask16 m2 = 0x5555;
        mask = _kxor_mask16(m1, m2);
    }
    
    return mask;
}

__attribute__((noinline))
__mmask8 generate_mask8_df(int iteration, __m512d data) {
    __mmask8 mask;
    
    for (int i = 0; i < iteration + 1; i++) {
        // Loop to create non-trivial control flow
        if (i % 2 == 0) {
            mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.0), _CMP_GT_OQ);
        } else {
            mask = 0xAA;
        }
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i data_i8 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i data_i16 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i data_i32 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i data_i64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 data_f32 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d data_f64 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    // Alternate data for blending
    __m512i alt_i8 = _mm512_set1_epi8(0xFF);
    __m512i alt_i16 = _mm512_set1_epi16(0xFFFF);
    __m512i alt_i32 = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i alt_i64 = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF);
    __m512 alt_f32 = _mm512_set1_ps(1.0f);
    __m512d alt_f64 = _mm512_set1_pd(2.0);
    
    // Data dependency chain: results from one blend influence next blends
    __m512i chain_result_i8 = data_i8;
    __m512 chain_result_f32 = data_f32;
    
    for (int i = 0; i < 8; i++) {
        // V64QImode
        __mmask64 mask64 = generate_mask64(i, chain_result_i8);
        __m512i result_i8 = blend_v64qi(data_i8, alt_i8, mask64);
        
        // Horizontal sum to prevent optimization
        __m512i sum_i8 = _mm512_sad_epu8(result_i8, _mm512_setzero_si512());
        checksum += _mm512_extract_epi64(sum_i8, 0);
        checksum += _mm512_extract_epi64(sum_i8, 1);
        
        // Update chain for next iteration
        chain_result_i8 = result_i8;
        
        // V32HImode - two different functions
        __mmask32 mask32_hi = generate_mask32_hi(i, chain_result_i8);
        __m512i result_i16 = blend_v32hi(data_i16, alt_i16, mask32_hi);
        __m512i result_i16_alt = blend_v32hi_alt(data_i16, alt_i16, mask32_hi);
        
        // Horizontal sum for 16-bit
        __m512i sum_i16 = _mm512_madd_epi16(result_i16, _mm512_set1_epi16(1));
        checksum += _mm512_extract_epi32(sum_i16, 0);
        
        // V16SImode - use result from previous blend in comparison
        __mmask16 mask16_si = _mm512_cmpeq_epi32_mask(result_i16, data_i16);
        __m512i result_i32 = blend_v16si(data_i32, alt_i32, mask16_si);
        
        // V8DImode
        __mmask8 mask8_di = (i & 1) ? 0xAA : 0x55;
        __m512i result_i64 = blend_v8di(data_i64, alt_i64, mask8_di);
        
        // V16SFmode - complex mask generation
        __mmask16 mask16_sf = generate_mask16_sf(i, chain_result_f32);
        __m512 result_f32 = blend_v16sf(data_f32, alt_f32, mask16_sf);
        
        // Horizontal sum for floats
        __m512 sum_f32 = _mm512_add_ps(result_f32, _mm512_permute_ps(result_f32, 0xB1));
        checksum += (uint64_t)_mm512_cvtss_f32(sum_f32);
        
        // Update chain
        chain_result_f32 = result_f32;
        
        // V8DFmode - mask depends on previous float result
        __mmask8 mask8_df = generate_mask8_df(i, _mm512_cvtps_pd(_mm512_castps512_ps256(result_f32)));
        __m512d result_f64 = blend_v8df(data_f64, alt_f64, mask8_df);
        
        // Horizontal sum for doubles
        __m512d sum_f64 = _mm512_add_pd(result_f64, _mm512_permute_pd(result_f64, 0x05));
        checksum += (uint64_t)_mm512_cvtsd_f64(sum_f64);
        
        #ifdef __AVX512FP16__
        // V32HFmode and V32BFmode
        if (i < 4) {  // Conditional execution
            __m512h data_hf = _mm512_set1_ph(1.0f);
            __m512h alt_hf = _mm512_set1_ph(2.0f);
            __m512bh data_bf = _mm512_set1_ph(1.0f);
            __m512bh alt_bf = _mm512_set1_ph(2.0f);
            
            __mmask32 mask32_hf = (i & 1) ? 0xAAAAAAAA : 0x55555555;
            __m512h result_hf = blend_v32hf(data_hf, alt_hf, mask32_hf);
            __m512bh result_bf = blend_v32bf(data_bf, alt_bf, mask32_hf);
            
            // Use results to affect checksum
            __m512h sum_hf = _mm512_add_ph(result_hf, _mm512_permutexvar_ph(_mm512_set_epi16(
                31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
            ), result_hf));
            checksum += (uint64_t)_mm512_cvtsh_h(sum_hf);
        }
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
