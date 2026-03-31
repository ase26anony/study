#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512h global_v32hf;
volatile __m512bh global_v32bf;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512d global_v8df;
volatile __m512 global_v16sf;

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result: %lld\n", type, result);
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
        
        // Create mask by comparing a < b (all true)
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = result;
        
        // Horizontal sum
        __m512 sum1 = _mm512_add_ps(result, _mm512_permute_ps(result, 0xB1));
        __m512 sum2 = _mm512_add_ps(sum1, _mm512_permute_ps(sum1, 0x4E));
        __m512 sum3 = _mm512_add_ps(sum2, _mm512_permute_ps(sum2, 0x1B));
        float final_sum = _mm512_reduce_add_ps(sum3);
        
        total_result += (long long)final_sum;
        print_result("V16SF", (long long)final_sum);
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a < b (all true)
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result to prevent optimization
        global_v8df = result;
        
        // Horizontal sum
        __m512d sum1 = _mm512_add_pd(result, _mm512_permute_pd(result, 0x5));
        __m512d sum2 = _mm512_add_pd(sum1, _mm512_permute_pd(sum1, 0x4E));
        __m512d sum3 = _mm512_add_pd(sum2, _mm512_permute_pd(sum2, 0x1B));
        double final_sum = _mm512_reduce_add_pd(sum3);
        
        total_result += (long long)final_sum;
        print_result("V8DF", (long long)final_sum);
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == (b/100) (all true)
        __m512i b_div_100 = _mm512_srli_epi32(b, 2); // Approximate division by 4
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b_div_100);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_v16si = result;
        
        // Horizontal sum
        __m512i sum = _mm512_add_epi32(result, _mm512_bsrli_epi128(result, 4));
        sum = _mm512_add_epi32(sum, _mm512_bsrli_epi128(sum, 8));
        int final_sum = _mm512_reduce_add_epi32(sum);
        
        total_result += final_sum;
        print_result("V16SI", final_sum);
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by comparing a < b (all true)
        __mmask8 mask = _mm512_cmplt_epi64_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result to prevent optimization
        global_v8di = result;
        
        // Horizontal sum
        __m512i sum = _mm512_add_epi64(result, _mm512_bsrli_epi128(result, 8));
        sum = _mm512_add_epi64(sum, _mm512_bsrli_epi128(sum, 16));
        sum = _mm512_add_epi64(sum, _mm512_bsrli_epi128(sum, 32));
        long long final_sum = _mm512_reduce_add_epi64(sum);
        
        total_result += final_sum;
        print_result("V8DI", final_sum);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(100);
        
        // Create pattern: 0xAA alternating pattern
        for (int i = 0; i < 64; i++) {
            ((char*)&a)[i] = i % 2;
            ((char*)&b)[i] = 100 + (i % 3);
        }
        
        // Create mask by comparing a < b (alternating pattern)
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = result;
        
        // Sum all elements
        __m512i sum = _mm512_sad_epu8(result, _mm512_setzero_si512());
        int final_sum = _mm512_reduce_add_epi32(sum);
        
        total_result += final_sum;
        print_result("V64QI", final_sum);
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set1_epi16(1);
        __m512i b = _mm512_set1_epi16(100);
        
        // Create pattern
        for (int i = 0; i < 32; i++) {
            ((short*)&a)[i] = i;
            ((short*)&b)[i] = 100 + i * 2;
        }
        
        // Create mask by comparing a < b (most true)
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hi = result;
        
        // Horizontal sum
        __m512i sum = _mm512_add_epi16(result, _mm512_bsrli_epi128(result, 2));
        sum = _mm512_add_epi16(sum, _mm512_bsrli_epi128(sum, 4));
        sum = _mm512_add_epi16(sum, _mm512_bsrli_epi128(sum, 8));
        int final_sum = _mm512_reduce_add_epi32(_mm512_madd_epi16(sum, _mm512_set1_epi16(1)));
        
        total_result += final_sum;
        print_result("V32HI", final_sum);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    // V32HF: 32 half-precision floats
    {
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)((i + 1) * 100.0f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < b (all true)
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hf = result;
        
        // Store and compute sum
        _Float16 result_data[32];
        _mm512_storeu_ph(result_data, result);
        
        float sum = 0.0f;
        for (int i = 0; i < 32; i++) {
            sum += (float)result_data[i];
        }
        
        total_result += (long long)sum;
        print_result("V32HF", (long long)sum);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float (bfloat16)
    // Note: AVX512-BF16 uses the same intrinsics as FP16 for blend operations
    {
        __m512bh a = _mm512_set1_epi16(0x3C00); // bfloat16 1.0
        __m512bh b = _mm512_set1_epi16(0x4000); // bfloat16 2.0
        
        // Create pattern
        for (int i = 0; i < 32; i++) {
            ((short*)&a)[i] = 0x3C00 + (i % 4) * 0x0400; // 1.0, 1.25, 1.5, 1.75...
            ((short*)&b)[i] = 0x4000 + (i % 4) * 0x0400; // 2.0, 2.25, 2.5, 2.75...
        }
        
        // Create mask using integer comparison since bfloat16 lacks comparison intrinsics
        __mmask32 mask = _mm512_cmplt_epi16_mask((__m512i)a, (__m512i)b);
        
        // Perform blend (uses same intrinsic as FP16)
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32bf = result;
        
        // Convert to float and sum
        float sum = 0.0f;
        for (int i = 0; i < 32; i++) {
            // Simple extraction and conversion
            short bf_val = ((short*)&result)[i];
            float f_val;
            // Simple bfloat16 to float conversion (not precise but sufficient for coverage)
            int temp = bf_val << 16;
            memcpy(&f_val, &temp, sizeof(float));
            sum += f_val;
        }
        
        total_result += (long long)sum;
        print_result("V32BF", (long long)sum);
    }
#endif // __AVX512BF16__

    printf("Total result: %lld\n", total_result);
    
    // Use all global variables in a way that can't be optimized out
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), "m"(global_v32hf),
                     "m"(global_v32bf), "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v8df), "m"(global_v16sf));
    
    return (int)(total_result & 0x7FFFFFFF);
}
