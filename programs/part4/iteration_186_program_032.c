#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_512i;
volatile __m512 global_512f;
volatile __m512d global_512d;
#ifdef __AVX512FP16__
volatile __m512h global_512h;
#endif

// Function to print 64-bit integer for verification
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long total_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_512i = result;
        
        // Extract first element and add to sum
        total_sum += _mm512_extract_epi32(result, 0);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality check
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        global_512i = result;
        total_sum += _mm512_extract_epi64(result, 0);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask by comparing a > b
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        global_512d = result;
        
        // Convert first element to integer for sum
        total_sum += (long long)_mm512_cvtsd_f64(result);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512 b = _mm512_set_ps(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask using less-than comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        global_512f = result;
        
        // Extract and convert first element
        total_sum += (long long)_mm512_cvtss_f32(result);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Create pattern: a = 0,1,2,3,... ; b = 255,254,253,...
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
        
        // Create mask: blend where a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        global_512i = result;
        
        // Extract first byte
        total_sum += _mm512_extract_epi8(result, 0);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask using equality with 15
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        global_512i = result;
        
        total_sum += _mm512_extract_epi16(result, 0);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Create alternating pattern
        __m512h a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);
        
        __m512h b = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f);
        
        // Create mask using greater-than comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        global_512h = result;
        
        // Extract first element (convert to float for sum)
        total_sum += (long long)_mm512_cvtph_f32(_mm512_extract_ph(result, 0));
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (uses same intrinsic as half-precision)
    {
        // Note: bfloat16 uses the same __m512bh type and operations as FP16
        // but with different underlying format
        __m512bh a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);
        
        __m512bh b = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f);
        
        // Create mask using less-than comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile to prevent optimization
        _mm512_store_ph((void*)&global_512h, result);
        
        // Extract and convert
        __m128bh low = _mm512_extractf32x8_ph(result, 0);
        float first_val = _mm_cvtsh_f32(_mm_extract_ph(low, 0));
        total_sum += (long long)first_val;
    }
#endif // __AVX512BF16__

    // Final output to ensure all computations are used
    print_result("Total sum from all blend operations", total_sum);
    
    // Additional volatile operation to ensure blends aren't optimized out
    asm volatile("" : : "r"(&global_512i), "r"(&global_512f), "r"(&global_512d));
    
    return 0;
}
