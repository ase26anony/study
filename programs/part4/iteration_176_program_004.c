#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow that prevents constant folding
    if (iteration % 3 == 0) {
        // Immediate mask
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Patterned mask
        mask = 0x5555555555555555ULL;
    } else {
        // Alternating pattern
        mask = 0x3333333333333333ULL;
    }
    
    // Logical operation on mask
    if (iteration > 5) {
        mask = _kor_mask64(mask, 0x00000000FFFFFFFFULL);
    }
    
    return mask;
}

// Function with loop-based control flow
__attribute__((noinline))
double process_with_control_flow(int iterations) {
    double checksum = 0.0;
    
    // Initialize vectors with patterned data
    __m512i a_epi8 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i b_epi8 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 a_ps = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 b_ps = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512d a_pd = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d b_pd = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    for (int i = 0; i < iterations; i++) {
        // Switch statement for different blend modes
        switch (i % 4) {
            case 0: {
                // V64QImode blend with comparison mask
                __m512i cmp_a = _mm512_set1_epi8(i);
                __m512i cmp_b = _mm512_set1_epi8(i / 2);
                __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(cmp_a, cmp_b);
                
                // Combine with immediate mask
                __mmask64 final_mask = _kor_mask64(cmp_mask, 
                    (__mmask64)(0xAAAAAAAAAAAAAAAAULL & (i + 1)));
                
                __m512i result = blend_v64qi(a_epi8, b_epi8, final_mask);
                
                // Use result to generate mask for next blend (data dependency chain)
                __mmask32 hi_mask = _mm512_cmpeq_epi16_mask(result, 
                    _mm512_set1_epi16(32));
                
                // V32HImode blend
                __m512i a_epi16 = _mm512_set1_epi16(i);
                __m512i b_epi16 = _mm512_set1_epi16(i * 2);
                __m512i result_hi = blend_v32hi(a_epi16, b_epi16, hi_mask);
                
                // Accumulate checksum
                int64_t* ptr = (int64_t*)&result_hi;
                for (int j = 0; j < 8; j++) {
                    checksum += ptr[j];
                }
                break;
            }
            
            case 1: {
                // V16SFmode blend with comparison mask
                __m512 cmp_val = _mm512_set1_ps(i * 0.5f);
                __mmask16 cmp_mask = _mm512_cmp_ps_mask(a_ps, cmp_val, _CMP_GT_OQ);
                
                // Logical operation on mask
                __mmask16 final_mask = _kxor_mask16(cmp_mask, 0xAAAA);
                
                __m512 result = blend_v16sf(a_ps, b_ps, final_mask);
                
                // Horizontal reduction
                float sum = _mm512_reduce_add_ps(result);
                checksum += sum;
                break;
            }
            
            case 2: {
                // V8DFmode blend
                __m512d cmp_val = _mm512_set1_pd(i * 0.25);
                __mmask8 cmp_mask = _mm512_cmp_pd_mask(a_pd, cmp_val, _CMP_LT_OQ);
                
                // Immediate mask with control flow
                __mmask8 final_mask;
                if (i > iterations / 2) {
                    final_mask = 0xFF;
                } else {
                    final_mask = 0xAA;
                }
                final_mask = _kand_mask8(final_mask, cmp_mask);
                
                __m512d result = blend_v8df(a_pd, b_pd, final_mask);
                
                // Horizontal reduction
                double sum = _mm512_reduce_add_pd(result);
                checksum += sum;
                break;
            }
            
            case 3: {
                // V16SImode and V8DImode blends with dependency chain
                __m512i a_epi32 = _mm512_set1_epi32(i);
                __m512i b_epi32 = _mm512_set1_epi32(i * 3);
                
                // Generate mask from comparison
                __mmask16 mask32 = _mm512_cmpeq_epi32_mask(a_epi32, 
                    _mm512_set1_epi32(i / 2));
                
                __m512i result32 = blend_v16si(a_epi32, b_epi32, mask32);
                
                // Use result to generate mask for V8DImode
                __mmask8 mask64 = _mm512_cmpeq_epi64_mask(result32,
                    _mm512_set1_epi64(i));
                
                __m512i a_epi64 = _mm512_set1_epi64(i);
                __m512i b_epi64 = _mm512_set1_epi64(i * 2);
                __m512i result64 = blend_v8di(a_epi64, b_epi64, mask64);
                
                // Accumulate checksum
                int64_t* ptr = (int64_t*)&result64;
                for (int j = 0; j < 8; j++) {
                    checksum += ptr[j];
                }
                break;
            }
        }
        
        // Modify vectors for next iteration to prevent optimization
        a_epi8 = _mm512_add_epi8(a_epi8, _mm512_set1_epi8(1));
        a_ps = _mm512_add_ps(a_ps, _mm512_set1_ps(0.1f));
        a_pd = _mm512_add_pd(a_pd, _mm512_set1_pd(0.05));
    }
    
    return checksum;
}

#ifdef __AVX512FP16__
__attribute__((noinline))
float process_half_precision(int iterations) {
    float checksum = 0.0f;
    
    // Initialize half-precision vectors
    _Float16 h_data_a[32], h_data_b[32];
    for (int i = 0; i < 32; i++) {
        h_data_a[i] = (_Float16)(i * 0.5f);
        h_data_b[i] = (_Float16)(31 - i * 0.5f);
    }
    
    __m512h a_hf = _mm512_set_ph(
        h_data_a[0], h_data_a[1], h_data_a[2], h_data_a[3],
        h_data_a[4], h_data_a[5], h_data_a[6], h_data_a[7],
        h_data_a[8], h_data_a[9], h_data_a[10], h_data_a[11],
        h_data_a[12], h_data_a[13], h_data_a[14], h_data_a[15],
        h_data_a[16], h_data_a[17], h_data_a[18], h_data_a[19],
        h_data_a[20], h_data_a[21], h_data_a[22], h_data_a[23],
        h_data_a[24], h_data_a[25], h_data_a[26], h_data_a[27],
        h_data_a[28], h_data_a[29], h_data_a[30], h_data_a[31]
    );
    
    __m512h b_hf = _mm512_set_ph(
        h_data_b[0], h_data_b[1], h_data_b[2], h_data_b[3],
        h_data_b[4], h_data_b[5], h_data_b[6], h_data_b[7],
        h_data_b[8], h_data_b[9], h_data_b[10], h_data_b[11],
        h_data_b[12], h_data_b[13], h_data_b[14], h_data_b[15],
        h_data_b[16], h_data_b[17], h_data_b[18], h_data_b[19],
        h_data_b[20], h_data_b[21], h_data_b[22], h_data_b[23],
        h_data_b[24], h_data_b[25], h_data_b[26], h_data_b[27],
        h_data_b[28], h_data_b[29], h_data_b[30], h_data_b[31]
    );
    
    __m512bh a_bh = _mm512_castph_bh(a_hf);
    __m512bh b_bh = _mm512_castph_bh(b_hf);
    
    for (int i = 0; i < iterations; i++) {
        // V32HFmode blend
        __mmask32 mask_hf;
        if (i % 2 == 0) {
            mask_hf = 0xAAAAAAAA;  // Immediate mask
        } else {
            // Comparison mask
            __m512h cmp_val = _mm512_set1_ph((_Float16)(i * 0.25f));
            mask_hf = _mm512_cmp_ph_mask(a_hf, cmp_val, _CMP_GT_OQ);
        }
        
        __m512h result_hf = blend_v32hf(a_hf, b_hf, mask_hf);
        
        // V32BFmode blend with dependency on previous result
        __mmask32 mask_bh = _kor_mask32(mask_hf, 0x55555555);
        __m512bh result_bh = blend_v32bf(a_bh, b_bh, mask_bh);
        
        // Convert back to check results
        __m512h result_check = _mm512_castbh_ph(result_bh);
        
        // Accumulate checksum
        _Float16* ptr = (_Float16*)&result_check;
        for (int j = 0; j < 32; j++) {
            checksum += ptr[j];
        }
        
        // Modify for next iteration
        a_hf = _mm512_add_ph(a_hf, _mm512_set1_ph((_Float16)0.1f));
    }
    
    return checksum;
}
#endif

int main() {
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Process with complex control flow
    double checksum1 = process_with_control_flow(16);
    printf("Checksum from main processing: %f\n", checksum1);
    
    // Additional direct calls to ensure all modes are covered
    __m512i test_a = _mm512_set1_epi8(1);
    __m512i test_b = _mm512_set1_epi8(2);
    
    // Direct V64QImode call
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i res64qi = blend_v64qi(test_a, test_b, mask64);
    
    // Direct V32HImode call
    __m512i test_a_hi = _mm512_set1_epi16(10);
    __m512i test_b_hi = _mm512_set1_epi16(20);
    __mmask32 mask32 = 0xAAAAAAAA;
    __m512i res32hi = blend_v32hi(test_a_hi, test_b_hi, mask32);
    
    // Direct V16SFmode call
    __m512 test_a_ps = _mm512_set1_ps(1.0f);
    __m512 test_b_ps = _mm512_set1_ps(2.0f);
    __mmask16 mask16 = 0xAAAA;
    __m512 res16sf = blend_v16sf(test_a_ps, test_b_ps, mask16);
    
    // Direct V8DFmode call
    __m512d test_a_pd = _mm512_set1_pd(1.0);
    __m512d test_b_pd = _mm512_set1_pd(2.0);
    __mmask8 mask8 = 0xAA;
    __m512d res8df = blend_v8df(test_a_pd, test_b_pd, mask8);
    
    // Direct V16SImode call
    __m512i test_a_si = _mm512_set1_epi32(100);
    __m512i test_b_si = _mm512_set1_epi32(200);
    __m512i res16si = blend_v16si(test_a_si, test_b_si, mask16);
    
    // Direct V8DImode call
    __m512i test_a_di = _mm512_set1_epi64(1000);
    __m512i test_b_di = _mm512_set1_epi64(2000);
    __m512i res8di = blend_v8di(test_a_di, test_b_di, mask8);
    
#ifdef __AVX512FP16__
    printf("Processing half-precision blends...\n");
    float checksum2 = process_half_precision(8);
    printf("Checksum from half-precision processing: %f\n", checksum2);
#else
    printf("AVX-512FP16 not enabled, skipping half-precision tests\n");
#endif
    
    // Final accumulation to prevent optimization
    int64_t* ptr64 = (int64_t*)&res64qi;
    int64_t* ptr32 = (int64_t*)&res32hi;
    int64_t* ptr16 = (int64_t*)&res16si;
    int64_t* ptr8 = (int64_t*)&res8di;
    
    double final_checksum = checksum1;
    for (int i = 0; i < 8; i++) {
        final_checksum += ptr64[i] + ptr32[i] + ptr16[i] + ptr8[i];
    }
    
    float* fptr = (float*)&res16sf;
    double* dptr = (double*)&res8df;
    for (int i = 0; i < 16; i++) final_checksum += fptr[i];
    for (int i = 0; i < 8; i++) final_checksum += dptr[i];
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
