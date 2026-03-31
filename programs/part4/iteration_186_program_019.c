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
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        total_result += _mm512_reduce_add_epi32(sum);
        
        print_result("V16SI", _mm512_reduce_add_epi32(result));
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality check
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        total_result += _mm512_reduce_add_epi64(sum);
        
        print_result("V8DI", _mm512_reduce_add_epi64(result));
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask using comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double dsum = _mm512_reduce_add_pd(sum);
        total_result += (long long)dsum;
        
        print_result("V8DF", (long long)_mm512_reduce_add_pd(result));
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                  7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                  8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float fsum = _mm512_reduce_add_ps(sum);
        total_result += (long long)fsum;
        
        print_result("V16SF", (long long)_mm512_reduce_add_ps(result));
    }
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    
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
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - horizontal sum of bytes
        __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
        total_result += _mm512_reduce_add_epi64(sum64);
        
        // Store to volatile to prevent optimization
        global_512i = result;
        
        print_result("V64QI", _mm512_reduce_add_epi64(sum64));
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                      15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                      16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: blend where a[i] is even
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        mask = ~mask;  // Invert to select even values
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        __m512i sum32 = _mm512_madd_epi16(result, _mm512_set1_epi16(1));
        total_result += _mm512_reduce_add_epi32(sum32);
        
        print_result("V32HI", _mm512_reduce_add_epi32(sum32));
    }
#endif

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    
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
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - convert to float and sum
        __m512 result_f = _mm512_cvtph_ps(_mm512_castph_si512(result));
        float fsum = _mm512_reduce_add_ps(result_f);
        total_result += (long long)fsum;
        
        // Store to volatile
        global_512h = result;
        
        print_result("V32HF", (long long)fsum);
    }
#endif

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blends...\n");
    
    // V32BF: 32 x brain float
    // Note: BF16 uses the same intrinsics as FP16 for blend operations
    {
        // Initialize with pattern
        __m512bh a, b;
        {
            float data_a[32], data_b[32];
            for (int i = 0; i < 32; i++) {
                data_a[i] = (float)i;
                data_b[i] = (float)(31 - i);
            }
            
            // Convert float to bfloat16
            __m512 a_f = _mm512_loadu_ps(data_a);
            __m512 b_f = _mm512_loadu_ps(data_b);
            a = _mm512_cvtneps_pbh(a_f);
            b = _mm512_cvtneps_pbh(b_f);
        }
        
        // Create mask - need to compare, so convert back to float
        __m512 a_f = _mm512_cvtpbh_ps(_mm512_castbph_si512(a));
        __m512 b_f = _mm512_cvtpbh_ps(_mm512_castbph_si512(b));
        __mmask32 mask = _mm512_cmp_ps_mask(a_f, b_f, _CMP_LT_OQ);
        
        // Perform blend (using FP16 blend intrinsic for BF16)
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result - convert to float and sum
        __m512 result_f = _mm512_cvtpbh_ps(_mm512_castbph_si512(result));
        float fsum = _mm512_reduce_add_ps(result_f);
        total_result += (long long)fsum;
        
        print_result("V32BF", (long long)fsum);
    }
#endif
    
    printf("Total aggregated result: %lld\n", total_result);
    
    // Final use of volatile globals to prevent optimization
    asm volatile("" : : "m"(global_512i), "m"(global_512d), "m"(global_512f));
    
    return 0;
}
