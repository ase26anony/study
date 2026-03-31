#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512h global_v32hf;
volatile __m512bh global_v32bf;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512d global_v8df;
volatile __m512 global_v16sf;

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s result: %lld\n", name, result);
}

int main() {
    long long final_sum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        global_v16si = result;
        
        // Extract first element and add to sum
        final_sum += _mm512_extract_epi32(result, 0);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality check
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        global_v8di = result;
        
        final_sum += _mm512_extract_epi64(result, 0);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask using comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        global_v8df = result;
        
        final_sum += (long long)_mm512_cvtsd_f64(result);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                  7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                  8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        global_v16sf = result;
        
        final_sum += (long long)_mm512_cvtss_f32(result);
    }
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Create pattern data
        uint8_t data_a[64], data_b[64];
        for (int i = 0; i < 64; i++) {
            data_a[i] = i;
            data_b[i] = 63 - i;
        }
        
        __m512i a = _mm512_loadu_si512(data_a);
        __m512i b = _mm512_loadu_si512(data_b);
        
        // Create mask by checking if elements are even
        __mmask64 mask = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_setzero_si512()
        );
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        global_v64qi = result;
        
        // Extract and add first element
        final_sum += _mm512_extract_epi8(result, 0);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                     15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                     16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask using greater-than comparison
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        global_v32hi = result;
        
        final_sum += _mm512_extract_epi16(result, 0);
    }
    
#ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        _Float16 data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = (_Float16)i;
            data_b[i] = (_Float16)(31 - i);
        }
        
        __m512h a = _mm512_loadu_ph(data_a);
        __m512h b = _mm512_loadu_ph(data_b);
        
        // Create mask using comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5f), _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        global_v32hf = result;
        
        // Extract first element
        _Float16 first = _mm512_cvtsh_h(result);
        final_sum += (long long)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("AVX512-BF16 supported\n");
    
    // V32BF: 32 x brain float (bfloat16)
    {
        // Initialize with pattern
        __bfloat16 data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = (__bfloat16)i;
            data_b[i] = (__bfloat16)(31 - i);
        }
        
        __m512bh a = _mm512_loadu_ph((const void*)data_a);
        __m512bh b = _mm512_loadu_ph((const void*)data_b);
        
        // Create mask using comparison (note: bfloat16 uses same intrinsics as FP16)
        __mmask32 mask = _mm512_cmp_ph_mask(
            (__m512h)a, 
            _mm512_set1_ph(15.5f), 
            _CMP_GT_OQ
        );
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        global_v32bf = result;
        
        // Extract first element
        __bfloat16 first;
        _mm_store_sd((double*)&first, _mm_castsi128_pd(
            _mm512_extracti32x4_epi32((__m512i)result, 0)
        ));
        final_sum += (long long)first;
    }
#endif // __AVX512BF16__

#endif // __AVX512BW__
#endif // __AVX512F__

    printf("Final aggregated sum: %lld\n", final_sum);
    
    // Use volatile globals to prevent optimization
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), 
                     "m"(global_v32hf), "m"(global_v32bf),
                     "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v8df), "m"(global_v16sf));
    
    return 0;
}
