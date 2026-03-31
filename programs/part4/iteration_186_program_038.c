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
void use_result(void* result, size_t size) {
    volatile char sink[64];
    memcpy((void*)sink, result, size > 64 ? 64 : size);
}

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Blend based on mask
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
        
        // Create mask using equality test
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and use first element
        int64_t first = _mm512_extract_epi64(res, 0);
        result_sum += (int)first;
        global_512i = res;
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Horizontal sum
        double sum = _mm512_reduce_add_pd(res);
        result_sum += (int)sum;
        global_512d = res;
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512 b = _mm512_set_ps(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask: a < b
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Horizontal sum
        float sum = _mm512_reduce_add_ps(res);
        result_sum += (int)sum;
        global_512f = res;
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        // Create alternating mask pattern
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; // 1010... pattern
        
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Sum first 8 bytes
        int8_t* ptr = (int8_t*)&res;
        for(int i = 0; i < 8; i++) {
            result_sum += ptr[i];
        }
        global_512i = res;
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        // Create mask: a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Sum first 4 elements
        int16_t* ptr = (int16_t*)&res;
        for(int i = 0; i < 4; i++) {
            result_sum += ptr[i];
        }
        global_512i = res;
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for(int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(31 - i);
            b_data[i] = (_Float16)i;
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5), _CMP_GT_OQ);
        
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and sum first 4 elements
        _Float16 res_data[32];
        _mm512_storeu_ph(res_data, res);
        for(int i = 0; i < 4; i++) {
            result_sum += (int)res_data[i];
        }
        global_512h = res;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (uses same intrinsics as half-precision)
    {
        // Note: __bfloat16 is available in GCC 11+
        #ifdef __bf16
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        
        for(int i = 0; i < 32; i++) {
            // Simple pattern
            a_data[i] = bfloat16_from_float(31.0f - i);
            b_data[i] = bfloat16_from_float(i);
        }
        
        __m512bh a = _mm512_loadu_ph((void*)a_data);
        __m512bh b = _mm512_loadu_ph((void*)b_data);
        
        // Create mask using comparison
        __mmask32 mask = 0x55555555; // Alternating pattern
        
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Store result
        __bfloat16 res_data[32];
        _mm512_storeu_ph((void*)res_data, res);
        
        // Use first element
        result_sum += (int)bfloat16_to_float(res_data[0]);
        #endif
    }
#endif // __AVX512BF16__

    printf("Final result sum: %d\n", result_sum);
    printf("All AVX-512 blend operations tested.\n");
    
    return 0;
}
