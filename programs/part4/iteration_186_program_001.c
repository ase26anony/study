#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_512i;
volatile __m512 global_512f;
volatile __m512d global_512d;
volatile __m512h global_512h;
volatile __m512bh global_512bh;

// Function to print results
void print_result(const char* type, long long result) {
    printf("%s blend result checksum: %lld\n", type, result);
}

int main() {
    long long total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        
        // Create mask by comparing a > b (using signed compare)
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_512i = result;
        
        // Compute checksum
        int checksum = 0;
        int* res_ptr = (int*)&result;
        for (int i = 0; i < 16; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += checksum;
        print_result("V16SI", checksum);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by comparing a != b
        __mmask8 mask = _mm512_cmpneq_epi64_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_512i = result;
        
        // Compute checksum
        long long checksum = 0;
        long long* res_ptr = (long long*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += checksum;
        print_result("V8DI", checksum);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.5f,14.5f,13.5f,12.5f,11.5f,10.5f,9.5f,8.5f,
                                  7.5f,6.5f,5.5f,4.5f,3.5f,2.5f,1.5f,0.5f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                  8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        global_512f = result;
        
        // Compute checksum
        float checksum = 0.0f;
        float* res_ptr = (float*)&result;
        for (int i = 0; i < 16; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += (long long)checksum;
        print_result("V16SF", (long long)checksum);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.5,6.5,5.5,4.5,3.5,2.5,1.5,0.5);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask by comparing a < b
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_512d = result;
        
        // Compute checksum
        double checksum = 0.0;
        double* res_ptr = (double*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += (long long)checksum;
        print_result("V8DF", (long long)checksum);
    }
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Initialize with pattern
        uint8_t a_data[64];
        uint8_t b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 63 - i;
        }
        
        __m512i a = _mm512_loadu_si512((__m512i*)a_data);
        __m512i b = _mm512_loadu_si512((__m512i*)b_data);
        
        // Create mask by comparing a > b (signed comparison)
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_512i = result;
        
        // Compute checksum
        int checksum = 0;
        uint8_t* res_ptr = (uint8_t*)&result;
        for (int i = 0; i < 64; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += checksum;
        print_result("V64QI", checksum);
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                     15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi16(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                     16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask by comparing a == b
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_512i = result;
        
        // Compute checksum
        int checksum = 0;
        short* res_ptr = (short*)&result;
        for (int i = 0; i < 32; i++) {
            checksum += res_ptr[i];
        }
        total_checksum += checksum;
        print_result("V32HI", checksum);
    }
    
#ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)i;
            b_data[i] = (_Float16)(31 - i);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a > b
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_512h = result;
        
        // Compute checksum
        float checksum = 0.0f;
        _Float16* res_ptr = (_Float16*)&result;
        for (int i = 0; i < 32; i++) {
            checksum += (float)res_ptr[i];
        }
        total_checksum += (long long)checksum;
        print_result("V32HF", (long long)checksum);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("AVX-512BF16 supported\n");
    
    // V32BF: 32 x brain float
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            // Convert int to float, then to bfloat16
            float temp_a = (float)i;
            float temp_b = (float)(31 - i);
            memcpy(&a_data[i], &temp_a, sizeof(__bfloat16));
            memcpy(&b_data[i], &temp_b, sizeof(__bfloat16));
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // Create mask by comparing a > b (need to convert to float for comparison)
        __m512h a_half = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_cvtneps_pbh(_mm512_loadu_ps((float*)a_data)));
        __m512h b_half = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_cvtneps_pbh(_mm512_loadu_ps((float*)b_data)));
        __mmask32 mask = _mm512_cmp_ph_mask(a_half, b_half, _CMP_GT_OQ);
        
        // Perform blend using the same intrinsic as V32HF
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_512bh = result;
        
        // Compute checksum
        float checksum = 0.0f;
        __bfloat16* res_ptr = (__bfloat16*)&result;
        for (int i = 0; i < 32; i++) {
            float temp;
            memcpy(&temp, &res_ptr[i], sizeof(__bfloat16));
            checksum += temp;
        }
        total_checksum += (long long)checksum;
        print_result("V32BF", (long long)checksum);
    }
#endif // __AVX512BF16__

#endif // __AVX512BW__

#endif // __AVX512F__
    
    printf("Total checksum: %lld\n", total_checksum);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_512i), "m"(global_512f), "m"(global_512d)
#ifdef __AVX512FP16__
                 , "m"(global_512h)
#endif
#ifdef __AVX512BF16__
                 , "m"(global_512bh)
#endif
                 );
    
    return 0;
}
