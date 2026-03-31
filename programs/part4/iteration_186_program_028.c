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
void print_result(const char* type, long long result) {
    printf("%s blend result: %lld\n", type, result);
}

int main() {
    long long total_result = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
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
        total_result += first;
        
        global_512i = result; // Prevent optimization
        print_result("V16SI (epi32)", first);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality comparison
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        long long first = _mm512_extract_epi64(sum, 0);
        total_result += first;
        
        global_512i = result;
        print_result("V8DI (epi64)", first);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask using floating-point comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double first = _mm512_cvtsd_f64(sum);
        total_result += (long long)first;
        
        global_512d = result;
        print_result("V8DF (pd)", (long long)first);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                 8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask using floating-point comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float first = _mm512_cvtss_f32(sum);
        total_result += (long long)first;
        
        global_512f = result;
        print_result("V16SF (ps)", (long long)first);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blends...\n");
    
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
        
        // Create mask: blend where a[i] < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - horizontal sum of bytes
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        uint64_t sum = _mm512_extract_epi64(sum64, 0) +
                      _mm512_extract_epi64(sum64, 1) +
                      _mm512_extract_epi64(sum64, 2) +
                      _mm512_extract_epi64(sum64, 3);
        total_result += sum;
        
        global_512i = result;
        print_result("V64QI (epi8)", sum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                     15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                     16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: blend where a[i] is even
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        mask = ~mask; // Invert to select even numbers
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result - extract first element
        short first = _mm512_extract_epi16(result, 0);
        total_result += first;
        
        global_512i = result;
        print_result("V32HI (epi16)", first);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blends...\n");
    
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
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5f), _CMP_LT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - reduce by adding all elements
        __m512h sum_vec = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
        
        // Store and compute sum
        _Float16 result_array[32];
        _mm512_storeu_ph(result_array, sum_vec);
        
        float sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += result_array[i];
        }
        total_result += (long long)sum;
        
        global_512h = result;
        print_result("V32HF (ph)", (long long)sum);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blends...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // Note: BF16 uses same intrinsics as FP16 in GCC
        __m512bh a, b;
        
        // Initialize with pattern data
        uint16_t data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern: i and 31-i as BF16 values
            data_a[i] = i << 8;  // Simple BF16 representation
            data_b[i] = (31 - i) << 8;
        }
        
        // Load as __m512bh
        a = _mm512_loadu_si512(data_a);
        b = _mm512_loadu_si512(data_b);
        
        // For BF16, we need to convert to float for comparison
        // Create a simple mask pattern
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - store and check first element
        uint16_t result_array[32];
        _mm512_storeu_si512(result_array, result);
        
        total_result += result_array[0];
        print_result("V32BF (bfloat16)", result_array[0]);
    }
#endif // __AVX512BF16__

    printf("\nTotal aggregated result: %lld\n", total_result);
    
    // Use results in volatile asm to ensure they're not optimized away
    asm volatile("" : : "m"(global_512i), "m"(global_512d), "m"(global_512f));
    
    return 0;
}
