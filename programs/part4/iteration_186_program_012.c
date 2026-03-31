#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_512i;
volatile __m512d global_512d;
volatile __m512 global_512f;
#ifdef __AVX512FP16__
volatile __m512h global_512h;
#endif

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long final_result = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int first = _mm512_extract_epi32(sum, 0);
        final_result += first;
        
        global_512i = result;
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality check
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        long long first = _mm512_extract_epi64(sum, 0);
        final_result += first;
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double first = _mm512_cvtsd_f64(sum);
        final_result += (long long)first;
        
        global_512d = result;
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512 b = _mm512_set_ps(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask: a < b
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float first = _mm512_cvtss_f32(sum);
        final_result += (long long)first;
        
        global_512f = result;
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    
    // V64QI: 64 x 8-bit integers
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
        
        // Create mask: a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - perform horizontal sum
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        uint64_t sum = _mm512_extract_epi64(sum64, 0) +
                      _mm512_extract_epi64(sum64, 1) +
                      _mm512_extract_epi64(sum64, 2) +
                      _mm512_extract_epi64(sum64, 3);
        final_result += (long long)sum;
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: a == b (will be false for all except maybe some)
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        __m512i sum32 = _mm512_madd_epi16(result, _mm512_set1_epi16(1));
        int sum = _mm512_extract_epi32(sum32, 0) +
                 _mm512_extract_epi32(sum32, 1) +
                 _mm512_extract_epi32(sum32, 2) +
                 _mm512_extract_epi32(sum32, 3);
        final_result += sum;
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        __m512h a, b;
        _Float16 a_data[32], b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i);
            b_data[i] = (_Float16)(31 - i);
        }
        
        a = _mm512_loadu_ph(a_data);
        b = _mm512_loadu_ph(b_data);
        
        // Create mask: a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5), _CMP_GT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - store to volatile global
        global_512h = result;
        
        // Extract first element
        _Float16 first = _mm512_cvtsh_h(result);
        final_result += (long long)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blends...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // Initialize with pattern
        __m512bh a, b;
        __bfloat16 a_data[32], b_data[32];
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern
            a_data[i] = bfloat16_from_float((float)i);
            b_data[i] = bfloat16_from_float((float)(31 - i));
        }
        
        a = _mm512_loadu_ph((const void*)a_data);
        b = _mm512_loadu_ph((const void*)b_data);
        
        // Create mask using comparison (need to cast to __m512h for comparison)
        __mmask32 mask = _mm512_cmp_ph_mask((__m512h)a, (__m512h)_mm512_set1_ph(15.5), _CMP_GT_OQ);
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - store to array and check first element
        __bfloat16 result_data[32];
        _mm512_storeu_ph((void*)result_data, (__m512h)result);
        
        final_result += (long long)bfloat16_to_float(result_data[0]);
    }
#endif // __AVX512BF16__

    printf("Final aggregated result: %lld\n", final_result);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_512i), "m"(global_512d), "m"(global_512f));
    
    return (int)(final_result & 0x7FFFFFFF);
}
