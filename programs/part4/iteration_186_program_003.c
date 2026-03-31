#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
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

// Helper function to print results
void print_hex(const char* label, const void* data, size_t size) {
    printf("%s: ", label);
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size && i < 16; i++) {
        printf("%02x ", bytes[i]);
    }
    if (size > 16) printf("...");
    printf("\n");
}

int main() {
    uint64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Blend based on mask
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = result;
        
        // Extract first element and add to sum
        float first = _mm512_cvtss_f32(result);
        final_sum += (uint64_t)first;
        
        print_hex("V16SF result", &result, 64);
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a < 5.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(5.0), _CMP_LT_OQ);
        
        // Blend based on mask
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result to prevent optimization
        global_v8df = result;
        
        // Extract first element and add to sum
        double first = _mm512_cvtsd_f64(result);
        final_sum += (uint64_t)first;
        
        print_hex("V8DF result", &result, 64);
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a < 10
        __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(10));
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_v16si = result;
        
        // Extract first element and add to sum
        int32_t first = _mm512_cvtsi512_si32(result);
        final_sum += first;
        
        print_hex("V16SI result", &result, 64);
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by comparing a < 5
        __mmask8 mask = _mm512_cmplt_epi64_mask(a, _mm512_set1_epi64(5));
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result to prevent optimization
        global_v8di = result;
        
        // Extract first element and add to sum
        int64_t first = _mm512_cvtsi512_si64(result);
        final_sum += first;
        
        print_hex("V8DI result", &result, 64);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend operations...\n");
    
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
            17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
            33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
            49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
        );
        __m512i b = _mm512_set_epi8(
            100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
            116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,
            132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
            148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163
        );
        
        // Create mask by comparing a < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = result;
        
        // Extract first element and add to sum
        int8_t first = (int8_t)_mm512_cvtsi512_si32(result);
        final_sum += first;
        
        print_hex("V64QI result", &result, 64);
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
            17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
        );
        __m512i b = _mm512_set_epi16(
            100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,
            1700,1800,1900,2000,2100,2200,2300,2400,2500,2600,2700,2800,2900,3000,3100,3200
        );
        
        // Create mask by comparing a < 20
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, _mm512_set1_epi16(20));
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hi = result;
        
        // Extract first element and add to sum
        int16_t first = (int16_t)_mm512_cvtsi512_si32(result);
        final_sum += first;
        
        print_hex("V32HI result", &result, 64);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 half-precision floats
    {
        // Create pattern for a and b
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)((i + 1) * 100);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __m512h threshold = _mm512_set1_ph(16.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_LT_OQ);
        
        // Blend based on mask
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hf = result;
        
        // Extract first element and add to sum
        _Float16 first = _mm512_cvtsh_h(result);
        final_sum += (uint64_t)first;
        
        print_hex("V32HF result", &result, 64);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 brain float values
    {
        // Create pattern for a and b
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern: 1.0, 2.0, 3.0, ...
            uint16_t val = (i + 1) << 8; // Simple representation
            a_data[i] = (__bfloat16)val;
            b_data[i] = (__bfloat16)(val * 100);
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // For BF16, we need to convert to float for comparison
        __m512 a_f32 = _mm512_cvtpbh_ps(a);
        __m512 b_f32 = _mm512_cvtpbh_ps(b);
        
        // Create mask by comparing converted values
        __mmask16 mask = _mm512_cmp_ps_mask(a_f32, _mm512_set1_ps(16.0f), _CMP_LT_OQ);
        
        // Blend based on mask (using the same intrinsic as FP16)
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32bf = result;
        
        // Extract first element and add to sum
        __bfloat16 first = a_data[0];
        final_sum += (uint64_t)first;
        
        print_hex("V32BF result", &result, 64);
    }
#endif // __AVX512BF16__

    printf("\nFinal checksum: %lu\n", final_sum);
    
    // Additional volatile operations to ensure blends aren't optimized out
    asm volatile ("" : : "m"(global_v64qi), "m"(global_v32hi), 
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
