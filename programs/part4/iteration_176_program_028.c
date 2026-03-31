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
__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Complex mask generation with control flow
__mmask64 generate_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Control flow prevents constant folding
    if (iteration % 3 == 0) {
        // Method 1: Comparison mask
        __m512i zeros = _mm512_setzero_si512();
        mask = _mm512_cmpeq_epi8_mask(data, zeros);
    } else if (iteration % 3 == 1) {
        // Method 2: Immediate mask with pattern
        mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Method 3: Logical combination of masks
        __m512i ones = _mm512_set1_epi8(1);
        __mmask64 m1 = _mm512_cmpeq_epi8_mask(data, ones);
        __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
        mask = _kor_mask64(m1, m2);
    }
    
    return mask;
}

__mmask32 generate_mask32(int iteration, __m512i data) {
    __mmask32 mask;
    
    switch (iteration % 4) {
        case 0:
            mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(0));
            break;
        case 1:
            mask = (__mmask32)0xAAAAAAAA;
            break;
        case 2:
            mask = _mm512_cmplt_epi16_mask(data, _mm512_set1_epi16(100));
            break;
        default:
            mask = _mm512_cmpgt_epi16_mask(data, _mm512_set1_epi16(-100));
            break;
    }
    
    return mask;
}

__mmask16 generate_mask16_float(int iteration, __m512 data) {
    __mmask16 mask;
    
    if (iteration < 10) {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.5f), _CMP_LT_OQ);
    } else {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.5f), _CMP_GT_OQ);
        // Logical operation on mask
        __mmask16 m2 = (__mmask16)0xAAAA;
        mask = _kxor_mask16(mask, m2);
    }
    
    return mask;
}

__mmask8 generate_mask8_double(int iteration, __m512d data) {
    __mmask8 mask;
    
    // Complex control flow
    for (int i = 0; i < iteration % 3; i++) {
        mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(i * 0.5), _CMP_LE_OQ);
        data = _mm512_add_pd(data, _mm512_set1_pd(0.1));
    }
    
    if (iteration % 2 == 0) {
        __mmask8 m2 = (__mmask8)0xAA;
        mask = _kand_mask8(mask, m2);
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i data_i8 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i data_i16 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i data_i32 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i data_i64 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 data_f32 = _mm512_set_ps(
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f
    );
    
    __m512d data_f64 = _mm512_set_pd(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
    
    // Alternate data for blending
    __m512i alt_i8 = _mm512_set1_epi8(0xFF);
    __m512i alt_i16 = _mm512_set1_epi16(0xFFFF);
    __m512i alt_i32 = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i alt_i64 = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF);
    __m512 alt_f32 = _mm512_set1_ps(2.0f);
    __m512d alt_f64 = _mm512_set1_pd(3.0);
    
    // Data dependency chain across different modes
    __m512i result_i8 = data_i8;
    __m512 result_f32 = data_f32;
    __m512d result_f64 = data_f64;
    
    // Loop with control flow to prevent optimization
    for (int iter = 0; iter < 5; iter++) {
        // V64QImode blend
        __mmask64 mask64 = generate_mask64(iter, result_i8);
        result_i8 = blend_v64qi(result_i8, alt_i8, mask64);
        
        // Use integer result to generate mask for float blend (data dependency)
        __m512i as_float_mask = _mm512_and_si512(result_i8, _mm512_set1_epi32(1));
        __m512 float_mask_data = _mm512_cvtepi32_ps(as_float_mask);
        __mmask16 mask16 = generate_mask16_float(iter, float_mask_data);
        
        // V16SFmode blend
        result_f32 = blend_v16sf(result_f32, alt_f32, mask16);
        
        // V32HImode blend
        __mmask32 mask32 = generate_mask32(iter, data_i16);
        __m512i result_i16 = blend_v32hi(data_i16, alt_i16, mask32);
        
        // V16SImode blend
        __mmask16 mask16_int = (__mmask16)(mask32 & 0xFFFF);
        __m512i result_i32 = blend_v16si(data_i32, alt_i32, mask16_int);
        
        // V8DImode blend
        __mmask8 mask8_int = (__mmask8)(mask16_int & 0xFF);
        __m512i result_i64 = blend_v8di(data_i64, alt_i64, mask8_int);
        
        // V8DFmode blend - mask depends on float result
        __mmask8 mask8_double = generate_mask8_double(iter, result_f64);
        result_f64 = blend_v8df(result_f64, alt_f64, mask8_double);
        
        // Accumulate checksums to prevent optimization
        checksum += _mm512_reduce_add_epi64(result_i8);
        checksum += _mm512_reduce_add_epi64(result_i16);
        checksum += _mm512_reduce_add_epi64(result_i32);
        checksum += _mm512_reduce_add_epi64(result_i64);
        
        // Float reductions
        __m512d f32_as_double = _mm512_cvtps_pd(_mm512_castps512_ps256(result_f32));
        checksum += (uint64_t)_mm512_reduce_add_pd(f32_as_double);
        checksum += (uint64_t)_mm512_reduce_add_pd(result_f64);
        
        // Modify data for next iteration
        data_i8 = _mm512_add_epi8(data_i8, _mm512_set1_epi8(1));
        data_f32 = _mm512_add_ps(data_f32, _mm512_set1_ps(0.25f));
        result_f64 = _mm512_add_pd(result_f64, _mm512_set1_pd(0.1));
    }
    
#ifdef __AVX512FP16__
    // Half-precision float blends (V32HFmode/V32BFmode)
    __m512h data_f16 = _mm512_set1_ph(1.0f);
    __m512h alt_f16 = _mm512_set1_ph(2.0f);
    __m512bh data_bf16 = _mm512_set1_ph(1.0f);
    __m512bh alt_bf16 = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 3; i++) {
        __mmask32 mask_half;
        if (i == 0) {
            mask_half = (__mmask32)0xAAAAAAAA;
        } else if (i == 1) {
            mask_half = (__mmask32)0x55555555;
        } else {
            mask_half = (__mmask32)0xFFFFFFFF;
        }
        
        __m512h result_f16 = blend_v32hf(data_f16, alt_f16, mask_half);
        __m512bh result_bf16 = blend_v32bf(data_bf16, alt_bf16, mask_half);
        
        // Convert to float for checksum
        __m512 f16_as_float = _mm512_cvtph_ps(_mm512_castph_si512(result_f16));
        checksum += (uint64_t)_mm512_reduce_add_ps(f16_as_float);
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
