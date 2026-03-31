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
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Blend based on mask
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_512f = res;
        float temp[16];
        _mm512_storeu_ps(temp, res);
        result_sum += (int)temp[0];
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Blend based on mask
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_512d = res;
        double temp[8];
        _mm512_storeu_pd(temp, res);
        result_sum += (int)temp[0];
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a < 10
        __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(10));
        
        // Blend based on mask
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_512i = res;
        int temp[16];
        _mm512_storeu_epi32(temp, res);
        result_sum += temp[0];
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by comparing a > 4
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(4));
        
        // Blend based on mask
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        int64_t temp[8];
        _mm512_storeu_epi64(temp, res);
        result_sum += (int)temp[0];
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
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
        
        // Create mask by comparing a < 50
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(50));
        
        // Blend based on mask
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        uint8_t temp[64];
        _mm512_storeu_epi8(temp, res);
        result_sum += temp[0];
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
        
        // Create mask by comparing a > 20
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(20));
        
        // Blend based on mask
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        int16_t temp[32];
        _mm512_storeu_epi16(temp, res);
        result_sum += temp[0];
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    // V32HF: 32 half-precision floats
    {
        __m512h a = _mm512_setr_ph(
            1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
            17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
        );
        __m512h b = _mm512_setr_ph(
            100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
            900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f,
            1700.0f, 1800.0f, 1900.0f, 2000.0f, 2100.0f, 2200.0f, 2300.0f, 2400.0f,
            2500.0f, 2600.0f, 2700.0f, 2800.0f, 2900.0f, 3000.0f, 3100.0f, 3200.0f
        );
        
        // Create mask by comparing a < 20.0f
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(20.0f), _CMP_LT_OQ);
        
        // Blend based on mask
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_512h = res;
        _Float16 temp[32];
        _mm512_storeu_ph(temp, res);
        result_sum += (int)temp[0];
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float (bfloat16) values
    // Note: BF16 uses the same intrinsics as FP16 for blend operations
    {
        // Create bfloat16 vectors
        unsigned short bf_data_a[32] = {
            0x3C00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700, 0x4800,
            0x4880, 0x4900, 0x4980, 0x4A00, 0x4A80, 0x4B00, 0x4B80, 0x4C00,
            0x4C80, 0x4D00, 0x4D80, 0x4E00, 0x4E80, 0x4F00, 0x4F80, 0x5000,
            0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400
        };
        unsigned short bf_data_b[32] = {
            0x5C00, 0x6000, 0x6200, 0x6400, 0x6500, 0x6600, 0x6700, 0x6800,
            0x6880, 0x6900, 0x6980, 0x6A00, 0x6A80, 0x6B00, 0x6B80, 0x6C00,
            0x6C80, 0x6D00, 0x6D80, 0x6E00, 0x6E80, 0x6F00, 0x6F80, 0x7000,
            0x7080, 0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400
        };
        
        __m512bh a = _mm512_loadu_epi16(bf_data_a);
        __m512bh b = _mm512_loadu_epi16(bf_data_b);
        
        // Create mask by comparing specific patterns
        // We'll use a simple alternating pattern mask
        __mmask32 mask = 0xAAAAAAAA; // Alternating 1/0 pattern
        
        // Blend based on mask (uses same intrinsic as FP16)
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        unsigned short temp[32];
        _mm512_storeu_epi16(temp, res);
        result_sum += temp[0];
    }
#endif // __AVX512BF16__

    printf("Result sum: %d\n", result_sum);
    return 0;
}
