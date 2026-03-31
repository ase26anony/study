#include <stdio.h>
#include <immintrin.h>
#include <stdint.h>

// Global volatile arrays to prevent optimization
volatile __m512i v64qi_result, v32hi_result, v16si_result, v8di_result;
volatile __m512 v16sf_result;
volatile __m512d v8df_result;
#ifdef __AVX512FP16__
volatile __m512h v32hf_result;
#endif
#ifdef __AVX512BF16__
volatile __m512bh v32bf_result;
#endif

// Function to print 64-bit integer
void print_u64(const char* label, unsigned long long val) {
    printf("%s: %llu\n", label, val);
}

int main() {
    unsigned long long final_result = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // ================= V16SI (16 x 32-bit integers) =================
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        v16si_result = result;
        
        // Extract first element and add to final result
        final_result += (unsigned long long)_mm512_extract_epi32(result, 0);
    }
    
    // ================= V8DI (8 x 64-bit integers) =================
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking if elements are odd
        __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        v8di_result = result;
        
        final_result += (unsigned long long)_mm512_extract_epi64(result, 0);
    }
    
    // ================= V16SF (16 x float) =================
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                  7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                  8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask by comparing a > 7.5f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        v16sf_result = result;
        
        final_result += (unsigned long long)_mm512_cvtss_u32(result);
    }
    
    // ================= V8DF (8 x double) =================
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask by comparing a < 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        v8df_result = result;
        
        final_result += (unsigned long long)_mm512_cvtsd_u64(result);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // ================= V64QI (64 x 8-bit integers) =================
    {
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63);
        
        // Create mask by checking if elements are even
        __mmask64 mask = _mm512_test_epi8_mask(a, _mm512_set1_epi8(1));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        v64qi_result = result;
        
        final_result += (unsigned long long)_mm512_extract_epi8(result, 0);
    }
    
    // ================= V32HI (32 x 16-bit integers) =================
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask by comparing a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        v32hi_result = result;
        
        final_result += (unsigned long long)_mm512_extract_epi16(result, 0);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // ================= V32HF (32 x half precision) =================
    {
        // Initialize with pattern
        __m512h a, b;
        short* a_ptr = (short*)&a;
        short* b_ptr = (short*)&b;
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern: a = i, b = 31-i
            a_ptr[i] = i;  // Half-precision representation
            b_ptr[i] = 31 - i;
        }
        
        // Create mask by comparing with constant
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.0), _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        v32hf_result = result;
        
        // Extract first element
        final_result += (unsigned long long)((short*)&result)[0];
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // ================= V32BF (32 x brain float) =================
    {
        // Brain float uses same intrinsics as half precision
        __m512bh a, b;
        unsigned short* a_ptr = (unsigned short*)&a;
        unsigned short* b_ptr = (unsigned short*)&b;
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern
            a_ptr[i] = i * 100;
            b_ptr[i] = (31 - i) * 100;
        }
        
        // Create mask - use same intrinsic as half precision
        __mmask32 mask = _mm512_cmp_ph_mask(*(__m512h*)&a, 
                                           _mm512_set1_ph(1500.0), 
                                           _CMP_GT_OQ);
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        v32bf_result = result;
        
        // Extract first element
        final_result += (unsigned long long)((unsigned short*)&result)[0];
    }
#endif // __AVX512BF16__

    // Print final result to ensure all computations are used
    print_u64("Final aggregated result", final_result);
    
    // Additional volatile operations to ensure blends aren't optimized out
    asm volatile("" : : "m"(v64qi_result), "m"(v32hi_result), 
                       "m"(v16si_result), "m"(v8di_result),
                       "m"(v16sf_result), "m"(v8df_result)
#ifdef __AVX512FP16__
                       , "m"(v32hf_result)
#endif
#ifdef __AVX512BF16__
                       , "m"(v32bf_result)
#endif
                       : "memory");
    
    return 0;
}
