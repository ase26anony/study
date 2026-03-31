#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_512i_result;
volatile __m512 global_512f_result;
volatile __m512d global_512d_result;
#ifdef __AVX512FP16__
volatile __m512h global_512h_result;
#endif

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85);
        
        // Create mask by comparing a with threshold
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        global_512i_result = result;
        
        // Extract first element as contribution to final sum
        final_sum += _mm512_extract_epi32(result, 0);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(70,60,50,40,30,20,10,0);
        
        // Create mask using equality comparison
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(0));
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        global_512i_result = result;
        
        final_sum += _mm512_extract_epi64(result, 0);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(150.0f,140.0f,130.0f,120.0f,110.0f,100.0f,90.0f,80.0f,
                                 70.0f,60.0f,50.0f,40.0f,30.0f,20.0f,10.0f,0.0f);
        
        // Create mask using floating-point comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(5.0f), _CMP_GT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        global_512f_result = result;
        
        final_sum += (long long)_mm512_cvtss_f32(result);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(70.0,60.0,50.0,40.0,30.0,20.0,10.0,0.0);
        
        // Create mask using floating-point comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(2.5), _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        global_512d_result = result;
        
        final_sum += (long long)_mm512_cvtsd_f64(result);
    }
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Create pattern for 64 bytes
        uint8_t data_a[64], data_b[64];
        for (int i = 0; i < 64; i++) {
            data_a[i] = i;
            data_b[i] = 255 - i;
        }
        
        __m512i a = _mm512_loadu_si512(data_a);
        __m512i b = _mm512_loadu_si512(data_b);
        
        // Create mask by comparing with threshold
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(32));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        global_512i_result = result;
        
        // Extract first byte as contribution
        final_sum += _mm512_extract_epi8(result, 0);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                     15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(310,300,290,280,270,260,250,240,230,220,210,200,
                                     190,180,170,160,150,140,130,120,110,100,90,80,
                                     70,60,50,40,30,20,10,0);
        
        // Create mask using equality check on lower bits
        __mmask32 mask = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(a, _mm512_set1_epi16(1)),
            _mm512_set1_epi16(0)
        );
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        global_512i_result = result;
        
        final_sum += _mm512_extract_epi16(result, 0);
    }
#endif

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        _Float16 data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = (_Float16)i;
            data_b[i] = (_Float16)(100.0f - i);
        }
        
        __m512h a = _mm512_loadu_ph(data_a);
        __m512h b = _mm512_loadu_ph(data_b);
        
        // Create mask using comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.0f), _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        global_512h_result = result;
        
        // Extract first element
        _Float16 first = _mm512_extract_f16x2(result, 0)[0];
        final_sum += (long long)first;
    }
#endif

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // For BF16, we need to use __m512bh type and appropriate intrinsics
        // Since _mm512_mask_blend_ph works with __m512h, we'll use that
        // and rely on the compiler to handle the BF16 mode
        
        // Create pattern data
        __m512bh a, b;
        uint16_t data_a[32], data_b[32];
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern for BF16 values
            data_a[i] = i << 8;  // Just a pattern, not valid BF16
            data_b[i] = (31 - i) << 8;
        }
        
        // Load as __m512h for blending
        __m512h a_h = _mm512_loadu_ph((_Float16*)data_a);
        __m512h b_h = _mm512_loadu_ph((_Float16*)data_b);
        
        // Create mask (alternating pattern)
        __mmask32 mask = 0xAAAAAAAA;  // 10101010... pattern
        
        __m512h result = _mm512_mask_blend_ph(mask, a_h, b_h);
        global_512h_result = result;
        
        // Extract contribution
        uint16_t first_val = ((uint16_t*)&result)[0];
        final_sum += first_val;
    }
#endif

    printf("Final aggregated result: %lld\n", final_sum);
    
    // Force use of all global results to prevent optimization
    asm volatile("" : : "m"(global_512i_result), "m"(global_512f_result), 
                  "m"(global_512d_result)
#ifdef __AVX512FP16__
                  , "m"(global_512h_result)
#endif
                  );
    
    return 0;
}
