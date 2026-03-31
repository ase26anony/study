#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_vi;
volatile __m512 global_vf;
volatile __m512d global_vd;
volatile __m512h global_vh;

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long total_result = 0;
    
#ifdef __AVX512F__
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
        
        // Use result to prevent optimization
        global_vf = result;
        
        // Extract first element and add to total
        float first = _mm512_cvtss_f32(result);
        total_result += (long long)first;
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_vd = result;
        
        // Extract first element
        double first = _mm512_cvtsd_f64(result);
        total_result += (long long)first;
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                     900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a < 10
        __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(10));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract first element
        int first = _mm512_extract_epi32(result, 0);
        total_result += first;
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by comparing a > 4
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(4));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract first element
        long long first = _mm512_extract_epi64(result, 0);
        total_result += first;
    }
#endif

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(100);
        
        // Create pattern: 01010101...
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract first element
        int8_t first = _mm512_extract_epi8(result, 0);
        total_result += first;
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set1_epi16(1);
        __m512i b = _mm512_set1_epi16(100);
        
        // Create mask by comparing with alternating pattern
        __m512i pattern = _mm512_setr_epi16(
            0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
            0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
        );
        __mmask32 mask = _mm512_cmpeq_epi16_mask(pattern, _mm512_set1_epi16(1));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract first element
        int16_t first = _mm512_extract_epi16(result, 0);
        total_result += first;
    }
#endif

#ifdef __AVX512FP16__
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
        
        // Use result
        global_vh = result;
        
        // Extract first element
        _Float16 first = _mm512_cvtsh_h(result);
        total_result += (long long)first;
    }
#endif

#ifdef __AVX512BF16__
    // V32BF: 32 brain float (use same as V32HF since they share intrinsics)
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)((i + 1) * 100));
        }
        
        // Load as __m512bh for BF16
        __m512bh a = _mm512_loadu_ph((const __m512h*)a_data);
        __m512bh b = _mm512_loadu_ph((const __m512h*)b_data);
        
        // Create mask by comparing first 16 elements (we'll use a simple pattern)
        __mmask32 mask = 0x55555555; // Alternating pattern
        
        // Perform blend (using _ph intrinsic for BF16 as well)
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result by storing to volatile
        __m512bh* volatile ptr = &result;
        (void)ptr;
        
        // Extract first element
        __bfloat16 first = ((__bfloat16*)&result)[0];
        total_result += bfloat16_to_uint32(first);
    }
#endif

    printf("Total result: %lld\n", total_result);
    return 0;
}
