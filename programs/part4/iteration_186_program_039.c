#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512 global_v16sf;
volatile __m512d global_v8df;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
#endif

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result: %lld\n", type, result);
}

int main() {
    long long total_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Store to volatile to prevent optimization
        global_v16sf = result;
        
        // Extract first element and add to sum
        total_sum += (long long)_mm512_cvtss_f32(result);
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Store to volatile
        global_v8df = result;
        
        // Extract first element
        total_sum += (long long)_mm512_cvtsd_f64(result);
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, _mm512_set1_epi32(5));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Store to volatile
        global_v16si = result;
        
        // Extract first element
        total_sum += (long long)_mm512_extract_epi32(result, 0);
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by checking odd/even
        __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_epi64(a, _mm512_set1_epi64(1)), 
                                               _mm512_set1_epi64(0));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Store to volatile
        global_v8di = result;
        
        // Extract first element
        total_sum += (long long)_mm512_extract_epi64(result, 0);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(100);
        
        // Create pattern: 01010101...
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile
        global_v64qi = result;
        
        // Extract first element
        total_sum += (long long)_mm512_extract_epi8(result, 0);
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set1_epi16(1);
        __m512i b = _mm512_set1_epi16(100);
        
        // Create mask by comparing with alternating pattern
        __m512i pattern = _mm512_setr_epi16(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
                                           0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);
        __mmask32 mask = _mm512_cmpeq_epi16_mask(pattern, _mm512_set1_epi16(0));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store to volatile
        global_v32hi = result;
        
        // Extract first element
        total_sum += (long long)_mm512_extract_epi16(result, 0);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    
    // V32HF: 32 half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)((i + 1) * 100);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(16.0), _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        global_v32hf = result;
        
        // Extract first element
        _Float16 first = _mm512_cvtsh_h(result);
        total_sum += (long long)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blends...\n");
    
    // V32BF: 32 brain float values
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)((i + 1) * 100));
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // Create mask using comparison (need to convert to float for comparison)
        __m512 a_f32 = _mm512_cvtpbh_ps(a);
        __m512 b_f32 = _mm512_cvtpbh_ps(b);
        __mmask16 mask32 = _mm512_cmp_ps_mask(a_f32, _mm512_set1_ps(16.0), _CMP_LT_OQ);
        
        // Convert to 32-bit mask for blend
        __mmask32 mask = _mm512_kunpackd(mask32, mask32);
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        global_v32bf = result;
        
        // Extract first element
        __bfloat16 first = b_data[0];
        total_sum += (long long)bfloat16_to_float(first);
    }
#endif // __AVX512BF16__

    printf("Total sum from all blends: %lld\n", total_sum);
    
    // Use all volatile globals to prevent optimization
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), 
                     "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v16sf), "m"(global_v8df)
#ifdef __AVX512FP16__
                     , "m"(global_v32hf)
#endif
#ifdef __AVX512BF16__
                     , "m"(global_v32bf)
#endif
                     );
    
    return 0;
}
