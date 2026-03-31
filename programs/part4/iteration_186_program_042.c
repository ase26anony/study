#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i v64qi_result, v32hi_result, v16si_result, v8di_result;
volatile __m512 v16sf_result;
volatile __m512d v8df_result;
#ifdef __AVX512FP16__
volatile __m512h v32hf_result;
#endif
#ifdef __AVX512BF16__
volatile __m512bh v32bf_result;
#endif

// Function to print bits of a mask (for debugging)
void print_mask8(__mmask8 mask) {
    for (int i = 7; i >= 0; i--) printf("%d", (mask >> i) & 1);
}
void print_mask16(__mmask16 mask) {
    for (int i = 15; i >= 0; i--) printf("%d", (mask >> i) & 1);
}
void print_mask32(__mmask32 mask) {
    for (int i = 31; i >= 0; i--) printf("%d", (mask >> i) & 1);
}
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) printf("%d", (mask >> i) & 1);
}

int main() {
    int64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // ==================== V16SI (16 x 32-bit integers) ====================
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        v16si_result = result; // Store to volatile to prevent optimization
        
        // Use result in computation
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        final_sum += _mm512_reduce_add_epi32(sum);
        
        printf("  V16SI mask: ");
        print_mask16(mask);
        printf("\n");
    }
    
    // ==================== V8DI (8 x 64-bit integers) ====================
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(100,99,98,97,96,95,94,93);
        
        // Create mask by checking if element is even
        __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
        mask = ~mask; // Invert: select from b when a is even
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        v8di_result = result;
        
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        final_sum += _mm512_reduce_add_epi64(sum);
        
        printf("  V8DI mask: ");
        print_mask8(mask);
        printf("\n");
    }
    
    // ==================== V16SF (16 x single-precision floats) ====================
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(100.0f,99.0f,98.0f,97.0f,96.0f,95.0f,94.0f,93.0f,
                                 92.0f,91.0f,90.0f,89.0f,88.0f,87.0f,86.0f,85.0f);
        
        // Create mask: select from b where a < 8.0
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        v16sf_result = result;
        
        // Horizontal sum
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        final_sum += (int64_t)_mm512_reduce_add_ps(sum);
        
        printf("  V16SF mask: ");
        print_mask16(mask);
        printf("\n");
    }
    
    // ==================== V8DF (8 x double-precision floats) ====================
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(100.0,99.0,98.0,97.0,96.0,95.0,94.0,93.0);
        
        // Create mask: select from b where a >= 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GE_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        v8df_result = result;
        
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        final_sum += (int64_t)_mm512_reduce_add_pd(sum);
        
        printf("  V8DF mask: ");
        print_mask8(mask);
        printf("\n");
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend operations...\n");
    
    // ==================== V64QI (64 x 8-bit integers) ====================
    {
        // Create pattern: 0,1,2,3,...,63
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Create pattern: 100,101,102,...,163
        __m512i b = _mm512_set_epi8(
            163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,
            147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,
            131,130,129,128,127,126,125,124,123,122,121,120,119,118,117,116,
            115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100
        );
        
        // Create mask: select from b where a[i] % 2 == 0
        __mmask64 mask = _mm512_test_epi8_mask(a, _mm512_set1_epi8(1));
        mask = ~mask; // Invert: select from b when even
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        v64qi_result = result;
        
        // Sum all elements
        __m512i sum = _mm512_sad_epu8(result, _mm512_setzero_si512());
        final_sum += _mm512_reduce_add_epi64(sum);
        
        printf("  V64QI mask (first 16 bits): ");
        print_mask16((__mmask16)(mask & 0xFFFF));
        printf("...\n");
    }
    
    // ==================== V32HI (32 x 16-bit integers) ====================
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi16(
            131,130,129,128,127,126,125,124,123,122,121,120,119,118,117,116,
            115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100
        );
        
        // Create mask: select from b where a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        v32hi_result = result;
        
        // Horizontal sum
        __m512i sum = _mm512_add_epi16(result, _mm512_set1_epi16(1));
        // Sum pairs to 32-bit, then reduce
        __m512i sum32 = _mm512_madd_epi16(sum, _mm512_set1_epi16(1));
        final_sum += _mm512_reduce_add_epi32(sum32);
        
        printf("  V32HI mask (first 16 bits): ");
        print_mask16((__mmask16)(mask & 0xFFFF));
        printf("...\n");
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blend operations...\n");
    
    // ==================== V32HF (32 x half-precision floats) ====================
    {
        // Initialize with pattern
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)i;
            b_data[i] = (_Float16)(i + 100);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: select from b where a < 16.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(16.0), _CMP_LT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        v32hf_result = result;
        
        // Use result in computation
        __m512h sum = _mm512_add_ph(result, _mm512_set1_ph(1.0));
        // Store and sum manually
        _Float16 sum_data[32];
        _mm512_storeu_ph(sum_data, sum);
        for (int i = 0; i < 32; i++) {
            final_sum += (int64_t)sum_data[i];
        }
        
        printf("  V32HF mask (first 16 bits): ");
        print_mask16((__mmask16)(mask & 0xFFFF));
        printf("...\n");
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blend operations...\n");
    
    // ==================== V32BF (32 x brain float) ====================
    {
        // BF16 uses same intrinsics as FP16 but different type
        __m512bh a, b;
        
        // Initialize with patterns
        unsigned short a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern: i as float, converted to BF16 bits
            float f = (float)i;
            memcpy(&a_data[i], &f, 2); // Take lower 16 bits (BF16)
            
            float g = (float)(i + 100);
            memcpy(&b_data[i], &g, 2);
        }
        
        a = _mm512_loadu_ph((__m512h*)a_data);
        b = _mm512_loadu_ph((__m512h*)b_data);
        
        // Create mask: select from b where (i % 3) == 0
        // We'll create a comparison mask using integer operations
        __m512i a_int = _mm512_loadu_si512(a_data);
        __m512i mod_mask = _mm512_set1_epi16(3);
        __m512i mod_result = _mm512_and_si512(a_int, _mm512_set1_epi16(3));
        __mmask32 mask = _mm512_cmpeq_epi16_mask(mod_result, _mm512_setzero_si512());
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        v32bf_result = result;
        
        // Use result
        __m512bh sum = _mm512_add_ph((__m512h)result, _mm512_set1_ph(1.0));
        unsigned short sum_data[32];
        _mm512_storeu_ph((__m512h*)sum_data, (__m512h)sum);
        for (int i = 0; i < 32; i++) {
            final_sum += sum_data[i];
        }
        
        printf("  V32BF mask (first 16 bits): ");
        print_mask16((__mmask16)(mask & 0xFFFF));
        printf("...\n");
    }
#endif // __AVX512BF16__

    printf("\nFinal aggregated sum: %ld\n", final_sum);
    printf("(This value is arbitrary; the important part is that all blends executed)\n");
    
    return 0;
}
