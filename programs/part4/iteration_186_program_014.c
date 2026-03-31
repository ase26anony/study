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
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        int sum = _mm512_reduce_add_epi32(res);
        result_sum += sum;
        global_512i = res;
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking odd/even
        __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
        
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        long long sum = _mm512_reduce_add_epi64(res);
        result_sum += (int)sum;
        global_512i = res;
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        double sum = _mm512_reduce_add_pd(res);
        result_sum += (int)sum;
        global_512d = res;
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                 8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask: a < 8.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        float sum = _mm512_reduce_add_ps(res);
        result_sum += (int)sum;
        global_512f = res;
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63);
        
        // Create mask: a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result - extract first 8 bytes and sum them
        uint64_t first_qword = _mm512_extract_epi64(res, 0);
        result_sum += (int)first_qword;
        global_512i = res;
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        int sum = _mm512_reduce_add_epi16(res);
        result_sum += sum;
        global_512i = res;
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(31 - i);
            b_data[i] = (_Float16)i;
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5), _CMP_GT_OQ);
        
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        _Float16 sum = 0;
        _Float16 res_data[32];
        _mm512_storeu_ph(res_data, res);
        for (int i = 0; i < 32; i++) {
            sum += res_data[i];
        }
        result_sum += (int)sum;
        global_512h = res;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blends...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // Initialize with pattern
        __m512bh a = _mm512_set1_epi16(0x3C00); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create alternating pattern in a
        uint16_t a_data[32];
        uint16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (i % 2) ? 0x3C00 : 0x4000; // Alternating 1.0 and 2.0
            b_data[i] = 0x3C00; // All 1.0
        }
        a = _mm512_loadu_epi16(a_data);
        b = _mm512_loadu_epi16(b_data);
        
        // Create mask: compare a with 1.5 (0x3E00 in bfloat16)
        __m512bh one_point_five = _mm512_set1_epi16(0x3E00);
        __mmask32 mask = _mm512_cmp_ph_mask(
            (__m512h)a, (__m512h)one_point_five, _CMP_GT_OQ);
        
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        uint16_t res_data[32];
        _mm512_storeu_epi16(res_data, res);
        int sum = 0;
        for (int i = 0; i < 32; i++) {
            sum += res_data[i];
        }
        result_sum += sum;
    }
#endif // __AVX512BF16__

    printf("Final result sum: %d\n", result_sum);
    
    // Additional volatile operations to ensure all blends are used
    asm volatile("" : : "r"(global_512i), "r"(global_512d), "r"(global_512f)
#ifdef __AVX512FP16__
                 , "r"(global_512h)
#endif
                 : "memory");
    
    return 0;
}
