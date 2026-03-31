#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

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

// Function to print bits of a mask (for debugging)
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

int main() {
    int64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // ==================== V16SF (16 single-precision floats) ====================
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Perform blend: result[i] = mask[i] ? a[i] : b[i]
        __m512 result = _mm512_mask_blend_ps(mask, b, a);
        v16sf_result = result;
        
        // Use result in computation
        __m512 sum_vec = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float sum = _mm512_reduce_add_ps(sum_vec);
        final_sum += (int64_t)sum;
    }
    
    // ==================== V8DF (8 double-precision floats) ====================
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, b, a);
        v8df_result = result;
        
        __m512d sum_vec = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double sum = _mm512_reduce_add_pd(sum_vec);
        final_sum += (int64_t)sum;
    }
    
    // ==================== V16SI (16 32-bit integers) ====================
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8,
                                                                      9, 10, 11, 12, 13, 14, 15, 16));
        
        __m512i result = _mm512_mask_blend_epi32(mask, b, a);
        v16si_result = result;
        
        // Use result
        __m512i sum_vec = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int32_t sum_arr[16];
        _mm512_storeu_si512((void*)sum_arr, sum_vec);
        for (int i = 0; i < 16; i++) final_sum += sum_arr[i];
    }
    
    // ==================== V8DI (8 64-bit integers) ====================
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask: blend odd elements
        __mmask8 mask = _mm512_int2mask(0xAA); // 0b10101010
        
        __m512i result = _mm512_mask_blend_epi64(mask, b, a);
        v8di_result = result;
        
        // Use result
        __m512i sum_vec = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        int64_t sum_arr[8];
        _mm512_storeu_si512((void*)sum_arr, sum_vec);
        for (int i = 0; i < 8; i++) final_sum += sum_arr[i];
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // ==================== V64QI (64 8-bit integers) ====================
    {
        // Create pattern: 0,1,2,...,63
        uint8_t a_arr[64], b_arr[64];
        for (int i = 0; i < 64; i++) {
            a_arr[i] = i;
            b_arr[i] = i + 100;
        }
        __m512i a = _mm512_loadu_si512((void*)a_arr);
        __m512i b = _mm512_loadu_si512((void*)b_arr);
        
        // Create mask by comparing a < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        __m512i result = _mm512_mask_blend_epi8(mask, b, a);
        v64qi_result = result;
        
        // Use result in computation
        __m512i sum_vec = _mm512_add_epi8(result, _mm512_set1_epi8(1));
        uint8_t sum_arr[64];
        _mm512_storeu_si512((void*)sum_arr, sum_vec);
        for (int i = 0; i < 64; i++) final_sum += sum_arr[i];
    }
    
    // ==================== V32HI (32 16-bit integers) ====================
    {
        // Create pattern: 0,1,2,...,31
        int16_t a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = i;
            b_arr[i] = i + 1000;
        }
        __m512i a = _mm512_loadu_si512((void*)a_arr);
        __m512i b = _mm512_loadu_si512((void*)b_arr);
        
        // Create mask: blend even elements
        __mmask32 mask = _mm512_int2mask(0xAAAAAAAA); // Alternating bits
        
        __m512i result = _mm512_mask_blend_epi16(mask, b, a);
        v32hi_result = result;
        
        // Use result
        __m512i sum_vec = _mm512_add_epi16(result, _mm512_set1_epi16(1));
        int16_t sum_arr[32];
        _mm512_storeu_si512((void*)sum_arr, sum_vec);
        for (int i = 0; i < 32; i++) final_sum += sum_arr[i];
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // ==================== V32HF (32 half-precision floats) ====================
    {
        // Initialize with pattern
        _Float16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (_Float16)(i * 0.5f);
            b_arr[i] = (_Float16)(i * 10.0f);
        }
        
        __m512h a = _mm512_loadu_ph((void*)a_arr);
        __m512h b = _mm512_loadu_ph((void*)b_arr);
        
        // Create mask by comparing a < 8.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_LT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, b, a);
        v32hf_result = result;
        
        // Use result
        __m512h sum_vec = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
        _Float16 sum_arr[32];
        _mm512_storeu_ph((void*)sum_arr, sum_vec);
        for (int i = 0; i < 32; i++) final_sum += (int)sum_arr[i];
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // ==================== V32BF (32 brain float values) ====================
    {
        // Initialize with pattern
        __bfloat16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = bfloat16_from_float(i * 0.5f);
            b_arr[i] = bfloat16_from_float(i * 10.0f);
        }
        
        __m512bh a = _mm512_loadu_bf16((void*)a_arr);
        __m512bh b = _mm512_loadu_bf16((void*)b_arr);
        
        // Create mask: blend first half
        __mmask32 mask = _mm512_int2mask(0x0000FFFF); // Lower 16 bits set
        
        __m512bh result = _mm512_mask_blend_ph(mask, b, a);
        v32bf_result = result;
        
        // Use result
        __m512bh sum_vec = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
        __bfloat16 sum_arr[32];
        _mm512_storeu_bf16((void*)sum_arr, sum_vec);
        for (int i = 0; i < 32; i++) final_sum += bfloat16_to_float(sum_arr[i]);
    }
#endif // __AVX512BF16__

    printf("Final aggregated sum: %ld\n", final_sum);
    
    // Additional volatile operation to ensure all results are used
    asm volatile("" : : "m"(v16sf_result), "m"(v8df_result), 
                   "m"(v16si_result), "m"(v8di_result),
                   "m"(v64qi_result), "m"(v32hi_result)
#ifdef __AVX512FP16__
                   , "m"(v32hf_result)
#endif
#ifdef __AVX512BF16__
                   , "m"(v32bf_result)
#endif
                   : "memory");
    
    return 0;
}
