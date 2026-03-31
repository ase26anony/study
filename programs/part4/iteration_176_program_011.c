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

// Helper function to create complex control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Switch statement for different mask generation methods
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 1:
            // Immediate mask
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Pattern based on iteration
            mask = (__mmask64)(0xFFFFFFFFFFFFFFFFULL >> (iteration % 64));
            break;
    }
    
    return mask;
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration, __m512i data) {
    __mmask32 mask;
    
    if (iteration < 10) {
        // Comparison mask
        mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(0));
    } else if (iteration < 20) {
        // Immediate mask
        mask = (__mmask32)0xAAAAAAAA;
    } else {
        // Logical operation
        __mmask32 m1 = _mm512_cmpgt_epi16_mask(data, _mm512_set1_epi16(100));
        __mmask32 m2 = (__mmask32)0x55555555;
        mask = _kxor_mask32(m1, m2);
    }
    
    return mask;
}

__attribute__((noinline))
__mmask16 generate_complex_mask16_float(int iteration, __m512 data) {
    __mmask16 mask;
    
    // Loop-like behavior without actual loop
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.0f), _CMP_EQ_OQ);
        } else if (i == 1) {
            __mmask16 temp = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.0f), _CMP_LT_OQ);
            mask = _kand_mask16(mask, temp);
        } else {
            mask = _kor_mask16(mask, (__mmask16)0xAAAA);
        }
    }
    
    return mask;
}

__attribute__((noinline))
__mmask8 generate_complex_mask8_double(int iteration, __m512d data) {
    __mmask8 mask;
    
    // Nested if-else chain
    if (iteration % 2 == 0) {
        mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.0), _CMP_GT_OQ);
    } else if (iteration % 3 == 0) {
        mask = (__mmask8)0xAA;
    } else {
        __mmask8 m1 = _mm512_cmp_pd_mask(data, _mm512_set1_pd(1.0), _CMP_LT_OQ);
        __mmask8 m2 = (__mmask8)0x55;
        mask = _kor_mask8(m1, m2);
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(100);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(200);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(300);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(400);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f,
        7.0f,6.5f,6.0f,5.5f,5.0f,4.5f,4.0f,3.5f,
        3.0f,2.5f,2.0f,1.5f,1.0f,0.5f,0.0f,-0.5f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(500.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.5,6.0,5.5,5.0,4.5,4.0,3.5);
    __m512d v8df_b = _mm512_set1_pd(600.0);
    
    // Data dependency chain: use results from one blend to affect another
    for (int i = 0; i < 8; i++) {
        // Generate masks with different methods based on iteration
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = generate_complex_mask32(i, v32hi_a);
        __mmask16 mask16_int = (__mmask16)(0xAAAA ^ (i * 0x1111));
        __mmask8 mask8_int = (__mmask8)(0xAA ^ i);
        __mmask16 mask16_float = generate_complex_mask16_float(i, v16sf_a);
        __mmask8 mask8_double = generate_complex_mask8_double(i, v8df_a);
        
        // Perform blends in all modes
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_int);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8_int);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
        // Data dependency: use integer result to modify float inputs
        if (i > 0) {
            // Convert some integer results to influence next iteration
            __m512i temp = _mm512_add_epi32(result16si, _mm512_set1_epi32(i));
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(temp, _mm512_set1_epi32(0));
            result16sf = blend_v16sf(result16sf, v16sf_b, new_mask);
        }
        
        // Data dependency: use float result to modify double inputs
        if (i % 2 == 0) {
            __mmask8 cmp_mask = _mm512_cmp_pd_mask(
                _mm512_cvtepi32_pd(_mm512_castsi512_si256(result16si)),
                _mm512_set1_pd(0.0),
                _CMP_GT_OQ
            );
            result8df = blend_v8df(result8df, v8df_b, cmp_mask);
        }
        
        // Accumulate checksums to prevent optimization
        uint8_t* ptr64qi = (uint8_t*)&result64qi;
        for (int j = 0; j < 64; j++) checksum += ptr64qi[j];
        
        uint16_t* ptr32hi = (uint16_t*)&result32hi;
        for (int j = 0; j < 32; j++) checksum += ptr32hi[j];
        
        uint32_t* ptr16si = (uint32_t*)&result16si;
        for (int j = 0; j < 16; j++) checksum += ptr16si[j];
        
        uint64_t* ptr8di = (uint64_t*)&result8di;
        for (int j = 0; j < 8; j++) checksum += ptr8di[j];
        
        float* ptr16sf = (float*)&result16sf;
        for (int j = 0; j < 16; j++) checksum += (uint64_t)ptr16sf[j];
        
        double* ptr8df = (double*)&result8df;
        for (int j = 0; j < 8; j++) checksum += (uint64_t)ptr8df[j];
        
        // Modify inputs for next iteration to create data flow
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v32hi_a = _mm512_add_epi16(v32hi_a, _mm512_set1_epi16(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
        v8df_a = _mm512_add_pd(v8df_a, _mm512_set1_pd(0.01));
    }
    
#ifdef __AVX512FP16__
    // Test half-precision blends if supported
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0
    
    __mmask32 mask_half = (__mmask32)0xAAAAAAAA;
    
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
    
    // Add to checksum
    uint16_t* ptr32hf = (uint16_t*)&result32hf;
    uint16_t* ptr32bf = (uint16_t*)&result32bf;
    for (int j = 0; j < 32; j++) {
        checksum += ptr32hf[j];
        checksum += ptr32bf[j];
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
