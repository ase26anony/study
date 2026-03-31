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

// Helper function to create complex control flow
__attribute__((noinline))
int process_with_control_flow(int mode, int iterations) {
    volatile int result = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set1_epi8(0x55);
    __m512i v64qi_b = _mm512_set1_epi8(0xAA);
    
    __m512i v32hi_a = _mm512_set1_epi16(0x3333);
    __m512i v32hi_b = _mm512_set1_epi16(0xCCCC);
    
    __m512i v16si_a = _mm512_set1_epi32(0x0F0F0F0F);
    __m512i v16si_b = _mm512_set1_epi32(0xF0F0F0F0);
    
    __m512i v8di_a = _mm512_set1_epi64(0x00FF00FF00FF00FFULL);
    __m512i v8di_b = _mm512_set1_epi64(0xFF00FF00FF00FF00ULL);
    
    __m512 v16sf_a = _mm512_set1_ps(1.0f);
    __m512 v16sf_b = _mm512_set1_ps(2.0f);
    
    __m512d v8df_a = _mm512_set1_pd(1.0);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    __m512bh v32bf_a = _mm512_set1_ph(1.0f);
    __m512bh v32bf_b = _mm512_set1_ph(2.0f);
    #endif
    
    // Complex control flow with switch and loops
    for (int i = 0; i < iterations; i++) {
        switch (mode) {
            case 0: { // V64QImode path
                // Generate mask using comparison
                __m512i cmp_a = _mm512_set1_epi8(i);
                __m512i cmp_b = _mm512_set1_epi8(iterations / 2);
                __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(cmp_a, cmp_b);
                
                // Combine with immediate mask using logical operation
                __mmask64 imm_mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
                __mmask64 final_mask = _kor_mask64(cmp_mask, imm_mask);
                
                // Blend with control flow dependency
                __m512i blended = blend_v64qi(v64qi_a, v64qi_b, final_mask);
                
                // Use result to generate mask for next operation
                __mmask32 hi_mask = _mm512_cmpeq_epi16_mask(blended, v32hi_a);
                __m512i hi_result = blend_v32hi(v32hi_a, v32hi_b, hi_mask);
                
                // Accumulate result
                result += _mm512_reduce_add_epi16(hi_result);
                break;
            }
            
            case 1: { // V32HImode path
                // Dynamic mask based on loop index
                __mmask32 mask = (i % 3 == 0) ? 
                    (__mmask32)0x55555555U : 
                    (__mmask32)0xAAAAAAAAU;
                
                // Chain blends with if-else
                __m512i blended = blend_v32hi(v32hi_a, v32hi_b, mask);
                
                if (i % 2 == 0) {
                    // Use result to generate float mask
                    __mmask16 float_mask = _mm512_cmpeq_epi32_mask(blended, v16si_a);
                    __m512 float_result = blend_v16sf(v16sf_a, v16sf_b, float_mask);
                    
                    // Horizontal sum
                    float sum = _mm512_reduce_add_ps(float_result);
                    result += (int)sum;
                }
                break;
            }
            
            case 2: { // V16SFmode path
                // Generate mask using float comparison
                __m512 cmp_val = _mm512_set1_ps((float)i);
                __mmask16 cmp_mask = _mm512_cmp_ps_mask(v16sf_a, cmp_val, _CMP_LT_OQ);
                
                // Logical operation on mask
                __mmask16 imm_mask = (__mmask16)0xAAAA;
                __mmask16 final_mask = _kxor_mask16(cmp_mask, imm_mask);
                
                __m512 blended = blend_v16sf(v16sf_a, v16sf_b, final_mask);
                
                // Use result to generate double mask
                __m512d double_cmp = _mm512_cvtps_pd(_mm512_castps512_ps256(blended));
                __mmask8 double_mask = _mm512_cmp_pd_mask(double_cmp, v8df_a, _CMP_GT_OQ);
                __m512d double_result = blend_v8df(v8df_a, v8df_b, double_mask);
                
                double sum = _mm512_reduce_add_pd(double_result);
                result += (int)sum;
                break;
            }
            
            case 3: { // V8DFmode path
                // Complex mask generation
                __mmask8 mask1 = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_NEQ_OQ);
                __mmask8 mask2 = (__mmask8)(0xF0);
                __mmask8 final_mask = _kand_mask8(mask1, mask2);
                
                __m512d blended = blend_v8df(v8df_a, v8df_b, final_mask);
                
                // Chain to integer blend
                __mmask16 int_mask = _mm512_cmp_pd_mask(blended, _mm512_set1_pd(1.5), _CMP_GT_OQ);
                __m512i int_result = blend_v16si(v16si_a, v16si_b, int_mask);
                
                result += _mm512_reduce_add_epi32(int_result);
                break;
            }
            
            case 4: { // V16SImode path
                // Mask depends on loop index with modulo
                __mmask16 mask = (__mmask16)((0x5555 << (i % 4)) | (0xAAAA >> (i % 4)));
                
                __m512i blended = blend_v16si(v16si_a, v16si_b, mask);
                
                // Chain to 64-bit blend
                __mmask8 di_mask = _mm512_cmpeq_epi64_mask(blended, v8di_a);
                __m512i di_result = blend_v8di(v8di_a, v8di_b, di_mask);
                
                result += _mm512_reduce_add_epi64(di_result);
                break;
            }
            
            case 5: { // V8DImode path
                // Immediate mask with pattern
                __mmask8 mask = (__mmask8)(0xAA ^ (i & 0xFF));
                
                __m512i blended = blend_v8di(v8di_a, v8di_b, mask);
                
                // Back to 8-bit blend
                __mmask64 qi_mask = _mm512_cmpeq_epi8_mask(blended, v64qi_a);
                __m512i qi_result = blend_v64qi(v64qi_a, v64qi_b, qi_mask);
                
                result += _mm512_reduce_add_epi8(qi_result);
                break;
            }
            
            #ifdef __AVX512FP16__
            case 6: { // V32HFmode path
                // Half-precision float blend
                __mmask32 hf_mask = (__mmask32)(0x55555555U ^ (i * 0x11111111U));
                
                __m512h blended = blend_v32hf(v32hf_a, v32hf_b, hf_mask);
                
                // Chain to brain float blend
                __m512bh bf_result = blend_v32bf(v32bf_a, v32bf_b, hf_mask);
                
                // Convert and accumulate
                __m512 converted = _mm512_cvtph_ps(_mm512_castph_si512(blended));
                float sum = _mm512_reduce_add_ps(converted);
                result += (int)sum;
                break;
            }
            
            case 7: { // V32BFmode path
                // Brain float blend with comparison mask
                __m512h cmp_val = _mm512_set1_ph((float)i);
                __mmask32 cmp_mask = _mm512_cmp_ph_mask(v32bf_a, cmp_val, _CMP_LT_OQ);
                
                __m512bh blended = blend_v32bf(v32bf_a, v32bf_b, cmp_mask);
                
                // Chain back to half float
                __m512h hf_result = blend_v32hf(v32hf_a, v32hf_b, cmp_mask);
                
                __m512 converted = _mm512_cvtph_ps(_mm512_castph_si512(hf_result));
                float sum = _mm512_reduce_add_ps(converted);
                result += (int)sum;
                break;
            }
            #endif
        }
        
        // Additional control flow to prevent optimization
        if (result > 1000000) {
            result = result % 1000;
        }
    }
    
    return result;
}

int main() {
    int total_checksum = 0;
    
    // Test all modes with different iteration counts to create varied control flow
    for (int mode = 0; mode < 8; mode++) {
        #ifndef __AVX512FP16__
        if (mode >= 6) continue; // Skip half-precision modes if not supported
        #endif
        
        int iterations = 10 + mode * 5; // Vary iterations per mode
        
        int result = process_with_control_flow(mode, iterations);
        total_checksum += result;
        
        printf("Mode %d result: %d\n", mode, result);
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    // Additional direct calls to ensure all intrinsics are used
    __m512i test_a = _mm512_setr_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    __m512i test_b = _mm512_setr_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Force generation of all blend patterns
    __mmask64 k64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i r64 = _mm512_mask_blend_epi8(k64, test_a, test_b);
    
    __mmask32 k32 = 0x55555555U;
    __m512i r32 = _mm512_mask_blend_epi16(k32, test_a, test_b);
    
    __mmask16 k16 = 0xAAAA;
    __m512i r16 = _mm512_mask_blend_epi32(k16, test_a, test_b);
    
    __mmask8 k8 = 0xAA;
    __m512i r8 = _mm512_mask_blend_epi64(k8, test_a, test_b);
    
    __m512 f_a = _mm512_set1_ps(1.0f);
    __m512 f_b = _mm512_set1_ps(2.0f);
    __m512 rf = _mm512_mask_blend_ps(k16, f_a, f_b);
    
    __m512d d_a = _mm512_set1_pd(1.0);
    __m512d d_b = _mm512_set1_pd(2.0);
    __m512d rd = _mm512_mask_blend_pd(k8, d_a, d_b);
    
    #ifdef __AVX512FP16__
    __m512h h_a = _mm512_set1_ph(1.0f);
    __m512h h_b = _mm512_set1_ph(2.0f);
    __m512h rh = _mm512_mask_blend_ph(k32, h_a, h_b);
    
    __m512bh bh_a = _mm512_set1_ph(1.0f);
    __m512bh bh_b = _mm512_set1_ph(2.0f);
    __m512bh rbh = _mm512_mask_blend_ph(k32, bh_a, bh_b);
    #endif
    
    // Accumulate results to prevent optimization
    total_checksum += _mm512_reduce_add_epi8(r64);
    total_checksum += _mm512_reduce_add_epi16(r32);
    total_checksum += _mm512_reduce_add_epi32(r16);
    total_checksum += _mm512_reduce_add_epi64(r8);
    total_checksum += (int)_mm512_reduce_add_ps(rf);
    total_checksum += (int)_mm512_reduce_add_pd(rd);
    
    printf("Final total checksum: %d\n", total_checksum);
    
    return 0;
}
