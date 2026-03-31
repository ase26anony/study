#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_512i;
volatile __m512 global_512f;
volatile __m512d global_512d;
volatile __m512h global_512h;
volatile __m512bh global_512bh;

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result: %lld\n", type, result);
}

int main() {
    long long total_result = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int first = _mm512_extract_epi32(sum, 0);
        total_result += first;
        
        print_result("V16SI", first);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking odd/even
        __mmask8 mask = _mm512_cmpeq_epi64_mask(
            _mm512_and_epi64(a, _mm512_set1_epi64(1)),
            _mm512_set1_epi64(0)
        );
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        long long first = _mm512_extract_epi64(sum, 0);
        total_result += first;
        
        print_result("V8DI", first);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.5f,14.5f,13.5f,12.5f,11.5f,10.5f,9.5f,8.5f,
                                  7.5f,6.5f,5.5f,4.5f,3.5f,2.5f,1.5f,0.5f);
        __m512 b = _mm512_set_ps(0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f,
                                  8.5f,9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f);
        
        // Create mask: a > 7.5f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
        
        // Blend operation
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float first = _mm512_cvtss_f32(sum);
        total_result += (long long)first;
        
        print_result("V16SF", (long long)first);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Blend operation
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double first = _mm512_cvtsd_f64(sum);
        total_result += (long long)first;
        
        print_result("V8DF", (long long)first);
    }
#endif

#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Create pattern: 0,1,2,3,...63
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Reverse pattern
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        // Create mask: select where a < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - sum all bytes
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        long long sum = _mm512_extract_epi64(sum64, 0) +
                       _mm512_extract_epi64(sum64, 1) +
                       _mm512_extract_epi64(sum64, 2) +
                       _mm512_extract_epi64(sum64, 3);
        total_result += sum;
        
        print_result("V64QI", sum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                     15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                     16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: select where a is even
        __mmask32 mask = _mm512_cmpeq_epi16_mask(
            _mm512_and_epi16(a, _mm512_set1_epi16(1)),
            _mm512_setzero_si512()
        );
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result - extract first element
        int first = _mm512_extract_epi16(result, 0);
        total_result += first;
        
        print_result("V32HI", first);
    }
#endif

#ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        __m512h a, b;
        _Float16 a_data[32], b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 0.5f);
            b_data[i] = (_Float16)((31 - i) * 0.5f);
        }
        
        a = _mm512_load_ph(a_data);
        b = _mm512_load_ph(b_data);
        
        // Create mask: compare with threshold
        __m512h threshold = _mm512_set1_ph((_Float16)8.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
        
        // Blend operation
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - store to volatile global
        _mm512_store_ph((_Float16*)&global_512h, result);
        
        // Extract first element
        _Float16 first = ((_Float16*)&global_512h)[0];
        total_result += (long long)(first * 100);
        
        print_result("V32HF", (long long)(first * 100));
    }
#endif

#ifdef __AVX512BF16__
    printf("AVX-512BF16 supported\n");
    
    // V32BF: 32 x brain float
    {
        // Note: BF16 uses same intrinsics as FP16 for blend
        __m512bh a, b;
        __bfloat16 a_data[32], b_data[32];
        
        // Initialize with simple pattern
        for (int i = 0; i < 32; i++) {
            // Simple integer pattern that works with BF16
            a_data[i] = bfloat16_from_float((float)(i % 16));
            b_data[i] = bfloat16_from_float((float)(15 - (i % 16)));
        }
        
        a = _mm512_load_ph((const void*)a_data);
        b = _mm512_load_ph((const void*)b_data);
        
        // Create mask using integer comparison on the bit pattern
        __m512i a_int = _mm512_castph_si512(a);
        __m512i b_int = _mm512_castph_si512(b);
        
        // Compare with threshold (8 in integer form)
        __mmask32 mask = _mm512_cmplt_epi16_mask(a_int, _mm512_set1_epi16(8 << 8));
        
        // Blend operation - using FP16 intrinsic since BF16 uses same
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        _mm512_store_ph((void*)&global_512bh, result);
        
        // Extract first element
        __bfloat16 first = ((__bfloat16*)&global_512bh)[0];
        total_result += (long long)bfloat16_to_float(first);
        
        print_result("V32BF", (long long)bfloat16_to_float(first));
    }
#endif

    printf("\nTotal aggregated result: %lld\n", total_result);
    return 0;
}
