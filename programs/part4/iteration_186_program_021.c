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
    printf("%s blend result: %lld\n", type, result);
}

int main() {
    long long final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        
        // Create mask by comparing a > b (using signed compare)
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_512i = result;
        final_sum += _mm512_extract_epi32(result, 0);
        
        print_result("V16SI", _mm512_extract_epi32(result, 0));
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(8,7,6,5,4,3,2,1);
        __m512i b = _mm512_set_epi64(1,2,3,4,5,6,7,8);
        
        // Create mask using equality comparison
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
        // Invert mask to get some blending
        mask = ~mask;
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        global_512i = result;
        final_sum += _mm512_extract_epi64(result, 0);
        
        print_result("V8DI", _mm512_extract_epi64(result, 0));
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
        __m512d b = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        
        // Create mask using floating-point comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        global_512d = result;
        final_sum += (long long)_mm512_cvtsd_f64(result);
        
        print_result("V8DF", (long long)_mm512_cvtsd_f64(result));
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(16.0f,15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,
                                  8.0f,7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f);
        __m512 b = _mm512_set_ps(1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,
                                  9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f);
        
        // Create mask using floating-point comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        global_512f = result;
        final_sum += (long long)_mm512_cvtss_f32(result);
        
        print_result("V16SF", (long long)_mm512_cvtss_f32(result));
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
        
        // Create mask by comparing a > b
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        global_512i = result;
        
        // Extract first 8 bytes and sum them
        uint64_t first_qword = _mm512_extract_epi64(result, 0);
        uint8_t* bytes = (uint8_t*)&first_qword;
        long long sum = 0;
        for (int i = 0; i < 8; i++) sum += bytes[i];
        final_sum += sum;
        
        print_result("V64QI", sum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                      15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                      16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask using equality comparison
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        // Invert to get non-zero mask
        mask = ~mask;
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        global_512i = result;
        final_sum += _mm512_extract_epi16(result, 0);
        
        print_result("V32HI", _mm512_extract_epi16(result, 0));
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
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        global_512h = result;
        
        // Extract first half and convert to integer
        _Float16 first = _mm512_extract_ph(result, 0);
        final_sum += (long long)first;
        
        print_result("V32HF", (long long)first);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blends...\n");
    
    // V32BF: 32 x brain float (treated same as half-precision for blending)
    {
        // BF16 uses same intrinsics as FP16
        __m512bh a, b;
        
        // Initialize with pattern data
        uint16_t data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern: i and 31-i as BF16 values
            data_a[i] = i << 8;  // Simple BF16 representation
            data_b[i] = (31 - i) << 8;
        }
        
        a = _mm512_loadu_si512(data_a);
        b = _mm512_loadu_si512(data_b);
        
        // Convert to __m512h for comparison (BF16 uses same comparison)
        __m512h a_h = _mm512_castsi512_ph(a);
        __m512h b_h = _mm512_castsi512_ph(b);
        
        __mmask32 mask = _mm512_cmp_ph_mask(a_h, b_h, _CMP_NEQ_OQ);
        
        __m512bh result = _mm512_castsi512_bh(
            _mm512_mask_blend_epi16(mask, 
                _mm512_castbh_si512(a), 
                _mm512_castbh_si512(b)));
        
        // Store to prevent optimization
        uint16_t store_buffer[32];
        _mm512_storeu_si512(store_buffer, _mm512_castbh_si512(result));
        
        final_sum += store_buffer[0];
        
        print_result("V32BF", store_buffer[0]);
    }
#endif

    printf("\nFinal aggregated sum: %lld\n", final_sum);
    
    // Use all global variables in a way compiler can't optimize away
    asm volatile("" : : "m"(global_512i), "m"(global_512f), "m"(global_512d)
    #ifdef __AVX512FP16__
                 , "m"(global_512h)
    #endif
                 );
    
    return (int)(final_sum & 0x7FFFFFFF);
}
