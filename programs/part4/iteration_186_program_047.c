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

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result checksum: %lld\n", type, result);
}

int main() {
    long long total_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result in computation
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int first = _mm512_extract_epi32(sum, 0);
        total_sum += first;
        
        // Store to volatile to prevent optimization
        global_512i = result;
        
        print_result("V16SI (16x int32)", first);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality test
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        long long first = _mm512_extract_epi64(sum, 0);
        total_sum += first;
        
        global_512i = result;
        
        print_result("V8DI (8x int64)", first);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double first = _mm512_cvtsd_f64(sum);
        total_sum += (long long)first;
        
        // Store to volatile
        _mm512_storeu_pd((double*)&global_512d, result);
        
        print_result("V8DF (8x double)", (long long)first);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                  7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                  8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask: a < 8.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float first = _mm512_cvtss_f32(sum);
        total_sum += (long long)first;
        
        _mm512_storeu_ps((float*)&global_512f, result);
        
        print_result("V16SF (16x float)", (long long)first);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend operations...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Initialize with pattern
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        // Create mask: a > 30
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(30));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - horizontal sum of bytes
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        long long sum = _mm512_extract_epi64(sum64, 0) +
                       _mm512_extract_epi64(sum64, 1) +
                       _mm512_extract_epi64(sum64, 2) +
                       _mm512_extract_epi64(sum64, 3);
        total_sum += sum;
        
        global_512i = result;
        
        print_result("V64QI (64x int8)", sum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        // Create mask: a == 15
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result - extract and sum first 4 elements
        short first = _mm512_extract_epi16(result, 0);
        total_sum += first;
        
        global_512i = result;
        
        print_result("V32HI (32x int16)", first);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        __m512h a = _mm512_set_ph(
            31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
            23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512h b = _mm512_set_ph(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
            16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
            24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
        );
        
        // Create mask: a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5f), _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - convert first element to float and use
        float first = _mm512_cvt_roundsh_ss(result, _MM_FROUND_CUR_DIRECTION);
        total_sum += (long long)(first * 100);
        
        // Store to volatile
        global_512h = result;
        
        print_result("V32HF (32x half)", (long long)(first * 100));
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX512-BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (uses same intrinsics as half)
    {
        // Note: __bfloat16 type and specific BF16 intrinsics may vary by compiler
        // Using the same half-precision blend since V32BF uses same pattern
        __m512h a = _mm512_set_ph(
            31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
            23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512h b = _mm512_set_ph(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
            16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
            24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
        );
        
        // Create mask using equality with 10.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(10.0f), _CMP_EQ_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        float first = _mm512_cvt_roundsh_ss(result, _MM_FROUND_CUR_DIRECTION);
        total_sum += (long long)(first * 50);
        
        global_512h = result;
        
        print_result("V32BF (32x bfloat16)", (long long)(first * 50));
    }
#endif // __AVX512BF16__

    printf("\nTotal checksum: %lld\n", total_sum);
    
    // Use volatile variables to prevent dead code elimination
    asm volatile("" : : "m"(global_512i), "m"(global_512f), "m"(global_512d));
    
    return 0;
}
