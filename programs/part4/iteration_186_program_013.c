#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_512i;
volatile __m512d global_512d;
volatile __m512 global_512f;
#ifdef __AVX512FP16__
volatile __m512h global_512h;
#endif

// Function to print results (prevents dead code elimination)
void use_result(void* ptr, size_t size) {
    volatile char sink;
    char* p = (char*)ptr;
    for (size_t i = 0; i < size; i++) {
        sink = p[i];
    }
}

int main() {
    int result_sum = 0;
    
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
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_512f = res;
        float sum = _mm512_reduce_add_ps(res);
        result_sum += (int)sum;
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_512d = res;
        double sum = _mm512_reduce_add_pd(res);
        result_sum += (int)sum;
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_512i = res;
        int sum = _mm512_reduce_add_epi32(res);
        result_sum += sum;
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask using comparison
        __m512i cmp_val = _mm512_set1_epi64(4);
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, cmp_val);
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i temp = _mm512_add_epi64(res, _mm512_set1_epi64(1));
        global_512i = temp;
        long long sum = _mm512_reduce_add_epi64(res);
        result_sum += (int)sum;
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    
    // V64QI: 64 8-bit integers
    {
        // Create pattern data
        uint8_t a_data[64], b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = i + 100;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select where a < 32
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, cmp_val);
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_512i = res;
        use_result(&res, sizeof(res));
        
        // Compute simple checksum
        uint8_t res_data[64];
        _mm512_storeu_si512((__m512i*)res_data, res);
        for (int i = 0; i < 64; i++) {
            result_sum += res_data[i];
        }
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
        __m512i b = _mm512_setr_epi16(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200,
                                      1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400,
                                      2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200);
        
        // Create mask: select where a is odd
        __m512i ones = _mm512_set1_epi16(1);
        __m512i and_res = _mm512_and_si512(a, ones);
        __mmask32 mask = _mm512_cmpeq_epi16_mask(and_res, ones);
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi16(res, _mm512_set1_epi16(1));
        global_512i = sum;
        
        // Compute horizontal sum
        int32_t temp_sum = _mm512_reduce_add_epi16(res);
        result_sum += temp_sum;
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    
    // V32HF: 32 half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)((i + 1) * 10);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: compare a < 16.0
        __m512h cmp_val = _mm512_set1_ph(16.0);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_LT_OQ);
        
        // Perform blend
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_512h = res;
        use_result(&res, sizeof(res));
        
        // Store and compute sum
        _Float16 res_data[32];
        _mm512_storeu_ph(res_data, res);
        float sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += res_data[i];
        }
        result_sum += (int)sum;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blends...\n");
    
    // V32BF: 32 brain floats (using same intrinsic as half-precision)
    {
        // Initialize with pattern
        __bfloat16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)((i + 1) * 10));
        }
        
        // Load as __m512bh (512-bit vector of BF16)
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // Convert to __m512h for comparison (BF16 uses same comparison as FP16)
        __m512h a_h = _mm512_cvtneps_pbh(_mm512_cvtpbh_ps(a));
        __m512h b_h = _mm512_cvtneps_pbh(_mm512_cvtpbh_ps(b));
        __m512h cmp_val = _mm512_set1_ph(16.0);
        
        // Create mask
        __mmask32 mask = _mm512_cmp_ph_mask(a_h, cmp_val, _CMP_LT_OQ);
        
        // Perform blend using the mask
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        use_result(&res, sizeof(res));
        
        // Store and compute approximate sum
        __bfloat16 res_data[32];
        _mm512_storeu_bf16(res_data, res);
        float sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += bfloat16_to_float(res_data[i]);
        }
        result_sum += (int)sum;
    }
#endif // __AVX512BF16__

    printf("Final result checksum: %d\n", result_sum);
    return 0;
}
