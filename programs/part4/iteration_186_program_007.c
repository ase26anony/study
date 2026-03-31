#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_vi;
volatile __m512d global_vd;
volatile __m512 global_vf;
#ifdef __AVX512FP16__
volatile __m512h global_vh;
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
    uint64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask by comparing a > b (element-wise)
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Perform masked blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_vi = result;
        int32_t* res_ptr = (int32_t*)&result;
        final_sum += res_ptr[0] + res_ptr[15];
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking equality with pattern
        __m512i pattern = _mm512_set1_epi64(3);
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, pattern);
        
        // Perform masked blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        global_vi = result;
        int64_t* res_ptr = (int64_t*)&result;
        final_sum += res_ptr[0] + res_ptr[7];
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __m512d threshold = _mm512_set1_pd(3.5);
        __mmask8 mask = _mm512_cmp_pd_mask(a, threshold, _CMP_GT_OQ);
        
        // Perform masked blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        global_vd = result;
        double* res_ptr = (double*)&result;
        final_sum += (uint64_t)(res_ptr[0] + res_ptr[7]);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512 b = _mm512_set_ps(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask: a < 7.5
        __m512 threshold = _mm512_set1_ps(7.5);
        __mmask16 mask = _mm512_cmp_ps_mask(a, threshold, _CMP_LT_OQ);
        
        // Perform masked blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        global_vf = result;
        float* res_ptr = (float*)&result;
        final_sum += (uint64_t)(res_ptr[0] + res_ptr[15]);
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
        
        // Create mask: a > 31
        __m512i threshold = _mm512_set1_epi8(31);
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, threshold);
        
        // Perform masked blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        global_vi = result;
        int8_t* res_ptr = (int8_t*)&result;
        final_sum += res_ptr[0] + res_ptr[63];
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
        
        // Create mask: a is even
        __m512i even_check = _mm512_set1_epi16(1);
        __mmask32 mask = _mm512_test_epi16_mask(a, even_check);
        mask = ~mask; // Invert to select even values
        
        // Perform masked blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        global_vi = result;
        int16_t* res_ptr = (int16_t*)&result;
        final_sum += res_ptr[0] + res_ptr[31];
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        __m512h a = _mm512_set_ph(
            31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
            23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512h b = _mm512_set_ph(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
            16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
            24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
        );
        
        // Create mask: a > 15.5
        __m512h threshold = _mm512_set1_ph(15.5f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
        
        // Perform masked blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        global_vh = result;
        _Float16* res_ptr = (_Float16*)&result;
        final_sum += (uint64_t)(res_ptr[0] + res_ptr[31]);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // Note: BF16 uses the same intrinsic as FP16 for blending
        __m512bh a = _mm512_set_ph(
            31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
            23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512bh b = _mm512_set_ph(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
            16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
            24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
        );
        
        // Create mask: a > 15.5
        __m512bh threshold = _mm512_set1_ph(15.5f);
        __mmask32 mask = _mm512_cmp_ph_mask((__m512h)a, (__m512h)threshold, _CMP_GT_OQ);
        
        // Perform masked blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        _Float16* res_ptr = (_Float16*)&result;
        final_sum += (uint64_t)(res_ptr[0] + res_ptr[31]);
    }
#endif // __AVX512BF16__

    printf("Final checksum: %lu\n", final_sum);
    
    // Additional volatile operations to ensure all results are used
    asm volatile("" : : "r"(global_vi), "r"(global_vd), "r"(global_vf));
    
    return 0;
}
