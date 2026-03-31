#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512 global_v16sf;
volatile __m512d global_v8df;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
#endif

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result: %lld\n", type, result);
}

int main() {
    long long total_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
    // ===== V16SF (16 single-precision floats) =====
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 9.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(9.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Store to volatile to prevent optimization
        global_v16sf = result;
        
        // Extract first element and add to sum
        float first = _mm512_cvtss_f32(result);
        total_sum += (long long)first;
        
        print_result("V16SF", (long long)first);
    }
    
    // ===== V8DF (8 double-precision floats) =====
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Store to volatile
        global_v8df = result;
        
        // Extract first element
        double first = _mm512_cvtsd_f64(result);
        total_sum += (long long)first;
        
        print_result("V8DF", (long long)first);
    }
    
    // ===== V16SI (16 32-bit integers) =====
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                     900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Store to volatile
        global_v16si = result;
        
        // Extract first element
        int first = _mm512_extract_epi32(result, 0);
        total_sum += first;
        
        print_result("V16SI", first);
    }
    
    // ===== V8DI (8 64-bit integers) =====
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by checking odd/even
        __m512i mask_val = _mm512_set1_epi64(1);
        __m512i and_result = _mm512_and_epi64(a, mask_val);
        __mmask8 mask = _mm512_cmpeq_epi64_mask(and_result, mask_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Store to volatile
        global_v8di = result;
        
        // Extract first element
        long long first = _mm512_extract_epi64(result, 0);
        total_sum += first;
        
        print_result("V8DI", first);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blends...\n");
    
    // ===== V64QI (64 8-bit integers) =====
    {
        // Create pattern data
        uint8_t a_data[64];
        uint8_t b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 100 + i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select where a < 32
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, cmp_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile
        global_v64qi = result;
        
        // Extract first few bytes and sum them
        uint8_t result_data[64];
        _mm512_storeu_si512((__m512i*)result_data, result);
        
        int sum = 0;
        for (int i = 0; i < 8; i++) {
            sum += result_data[i];
        }
        total_sum += sum;
        
        print_result("V64QI", sum);
    }
    
    // ===== V32HI (32 16-bit integers) =====
    {
        // Create pattern data
        int16_t a_data[32];
        int16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = i * 10;
            b_data[i] = 1000 + i * 20;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select where a is even
        __m512i mask_val = _mm512_set1_epi16(1);
        __m512i and_result = _mm512_and_epi16(a, mask_val);
        __mmask32 mask = _mm512_cmpeq_epi16_mask(and_result, _mm512_setzero_si512());
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store to volatile
        global_v32hi = result;
        
        // Extract first few elements
        int16_t result_data[32];
        _mm512_storeu_si512((__m512i*)result_data, result);
        
        int sum = 0;
        for (int i = 0; i < 4; i++) {
            sum += result_data[i];
        }
        total_sum += sum;
        
        print_result("V32HI", sum);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blends...\n");
    
    // ===== V32HF (32 half-precision floats) =====
    {
        // Create pattern data
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)(100 + i);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __m512h cmp_val = _mm512_set1_ph(16.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        global_v32hf = result;
        
        // Extract first element
        _Float16 result_data[32];
        _mm512_storeu_ph(result_data, result);
        
        total_sum += (long long)result_data[0];
        
        print_result("V32HF", (long long)result_data[0]);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blends...\n");
    
    // ===== V32BF (32 brain float values) =====
    {
        // Create pattern data
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)(100 + i));
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // Create mask by comparing (as float) a < 16.0
        // Note: We need to convert to float for comparison
        float a_float[32];
        float b_float[32];
        for (int i = 0; i < 32; i++) {
            a_float[i] = bfloat16_to_float(a_data[i]);
            b_float[i] = bfloat16_to_float(b_data[i]);
        }
        
        __m512 a_f = _mm512_loadu_ps(a_float);
        __m512 cmp_val = _mm512_set1_ps(16.0f);
        __mmask16 mask_float = _mm512_cmp_ps_mask(a_f, cmp_val, _CMP_LT_OQ);
        
        // Convert to 32-bit mask for BF16 blend
        __mmask32 mask = 0;
        for (int i = 0; i < 16; i++) {
            if (mask_float & (1 << i)) {
                mask |= (1 << (2*i)) | (1 << (2*i + 1));
            }
        }
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        global_v32bf = result;
        
        // Extract first element
        __bfloat16 result_data[32];
        _mm512_storeu_bf16(result_data, result);
        
        total_sum += (long long)bfloat16_to_float(result_data[0]);
        
        print_result("V32BF", (long long)bfloat16_to_float(result_data[0]));
    }
#endif // __AVX512BF16__

    printf("\nTotal sum from all blends: %lld\n", total_sum);
    
    // Use all volatile globals in a way that can't be optimized out
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), 
                     "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v16sf), "m"(global_v8df)
#ifdef __AVX512FP16__
                     , "m"(global_v32hf)
#endif
#ifdef __AVX512BF16__
                     , "m"(global_v32bf)
#endif
                     );
    
    return 0;
}
