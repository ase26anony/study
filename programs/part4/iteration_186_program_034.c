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
    long long final_sum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        
        // Create mask by comparing a > b (actually a > 15)
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(15));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result in computation
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int first = _mm512_extract_epi32(sum, 0);
        final_sum += first;
        
        // Store to volatile to prevent optimization
        global_512i = result;
        
        print_result("V16SI (32-bit int)", first);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(8,7,6,5,4,3,2,1);
        __m512i b = _mm512_set_epi64(1,2,3,4,5,6,7,8);
        
        // Create mask using equality comparison
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(4));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        long long first = _mm512_extract_epi64(sum, 0);
        final_sum += first;
        
        global_512i = result;
        
        print_result("V8DI (64-bit int)", first);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
        __m512d b = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        
        // Create mask using floating-point comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.5), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double first = _mm512_cvtsd_f64(sum);
        final_sum += (long long)first;
        
        // Store to volatile
        _mm512_storeu_pd((double*)&global_512d, result);
        
        print_result("V8DF (double)", (long long)first);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(16.0f,15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,
                                  8.0f,7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f);
        __m512 b = _mm512_set_ps(1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,
                                  9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f);
        
        // Create mask using floating-point comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.5f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float first = _mm512_cvtss_f32(sum);
        final_sum += (long long)first;
        
        // Store to volatile
        _mm512_storeu_ps((float*)&global_512f, result);
        
        print_result("V16SF (float)", (long long)first);
    }
#endif

#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Initialize with pattern
        uint8_t data_a[64], data_b[64];
        for (int i = 0; i < 64; i++) {
            data_a[i] = i;
            data_b[i] = 63 - i;
        }
        
        __m512i a = _mm512_loadu_si512(data_a);
        __m512i b = _mm512_loadu_si512(data_b);
        
        // Create mask by comparing a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - perform horizontal sum
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        uint64_t sum = _mm512_extract_epi64(sum64, 0) + 
                      _mm512_extract_epi64(sum64, 1) +
                      _mm512_extract_epi64(sum64, 2) +
                      _mm512_extract_epi64(sum64, 3);
        final_sum += sum;
        
        global_512i = result;
        
        print_result("V64QI (8-bit int)", sum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                      15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                      16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask by comparing a > b
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result - extract and sum first 4 elements
        int sum = _mm512_extract_epi16(result, 0) +
                 _mm512_extract_epi16(result, 1) +
                 _mm512_extract_epi16(result, 2) +
                 _mm512_extract_epi16(result, 3);
        final_sum += sum;
        
        global_512i = result;
        
        print_result("V32HI (16-bit int)", sum);
    }
#endif

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
        
        // Use result - convert to float and sum first few elements
        __m512 result_f32 = _mm512_cvtph_ps(_mm512_castph_si512(result));
        float sum = _mm512_reduce_add_ps(result_f32);
        final_sum += (long long)sum;
        
        // Store to volatile
        _mm512_storeu_ph((_Float16*)&global_512h, result);
        
        print_result("V32HF (half float)", (long long)sum);
    }
#endif

#ifdef __AVX512BF16__
    printf("AVX-512BF16 supported\n");
    
    // V32BF: 32 x brain float
    {
        // Initialize with pattern
        __bfloat16 data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = bfloat16_from_float((float)i);
            data_b[i] = bfloat16_from_float((float)(31 - i));
        }
        
        __m512bh a = _mm512_loadu_bf16(data_a);
        __m512bh b = _mm512_loadu_bf16(data_b);
        
        // Create mask - for BF16 we need to use the same intrinsic as FP16
        // First convert to FP16 for comparison
        __m512h a_h = _mm512_cvtneobf16_ps(a);
        __m512h b_h = _mm512_cvtneobf16_ps(b);
        __mmask32 mask = _mm512_cmp_ph_mask(a_h, b_h, _CMP_GT_OQ);
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - convert to float and compute sum
        __m512 result_f32 = _mm512_cvtneobf16_ps(result);
        float sum = _mm512_reduce_add_ps(result_f32);
        final_sum += (long long)sum;
        
        // Store to volatile
        _mm512_storeu_bf16((__bfloat16*)&global_512bh, result);
        
        print_result("V32BF (bfloat16)", (long long)sum);
    }
#endif

    printf("Final aggregated sum: %lld\n", final_sum);
    
    // Use all volatile globals in a way that can't be optimized out
    asm volatile("" : : "m"(global_512i), "m"(global_512f), "m"(global_512d), 
                  "m"(global_512h), "m"(global_512bh));
    
    return 0;
}
