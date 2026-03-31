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
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask32(__mmask32 mask) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask16(__mmask16 mask) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask8(__mmask8 mask) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

int main() {
    int64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // ========== V16SF (16 single-precision floats) ==========
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Perform blend: result[i] = mask[i] ? a[i] : b[i]
        __m512 result = _mm512_mask_blend_ps(mask, b, a);
        
        // Use result in computation to prevent optimization
        v16sf_result = result;
        
        // Extract first element and add to final sum
        float first_elem = _mm512_cvtss_f32(result);
        final_sum += (int64_t)first_elem;
        
        printf("  V16SF blend mask: ");
        print_mask16(mask);
        printf("  First element of result: %f\n", first_elem);
    }
    
    // ========== V8DF (8 double-precision floats) ==========
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a < 5.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(5.0), _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, b, a);
        
        // Use result
        v8df_result = result;
        
        // Extract first element
        double first_elem = _mm512_cvtsd_f64(result);
        final_sum += (int64_t)first_elem;
        
        printf("  V8DF blend mask: ");
        print_mask8(mask);
        printf("  First element of result: %lf\n", first_elem);
    }
    
    // ========== V16SI (16 32-bit integers) ==========
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                     900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a < 10
        __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(10));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, b, a);
        
        // Use result
        v16si_result = result;
        
        // Extract first element
        int32_t first_elem = _mm512_extract_epi32(result, 0);
        final_sum += first_elem;
        
        printf("  V16SI blend mask: ");
        print_mask16(mask);
        printf("  First element of result: %d\n", first_elem);
    }
    
    // ========== V8DI (8 64-bit integers) ==========
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by comparing a < 5
        __mmask8 mask = _mm512_cmplt_epi64_mask(a, _mm512_set1_epi64(5));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, b, a);
        
        // Use result
        v8di_result = result;
        
        // Extract first element
        int64_t first_elem = _mm512_extract_epi64(result, 0);
        final_sum += first_elem;
        
        printf("  V8DI blend mask: ");
        print_mask8(mask);
        printf("  First element of result: %ld\n", first_elem);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend operations...\n");
    
    // ========== V64QI (64 8-bit integers) ==========
    {
        // Create pattern: 0, 1, 2, ..., 63
        __m512i a = _mm512_setr_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        // Create pattern: 100, 101, 102, ..., 163
        __m512i b = _mm512_setr_epi8(
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
            116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
            148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163
        );
        
        // Create mask: select a where a[i] < 32, otherwise b
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, b, a);
        
        // Use result
        v64qi_result = result;
        
        // Extract first element
        int8_t first_elem = (int8_t)_mm512_extract_epi8(result, 0);
        final_sum += first_elem;
        
        printf("  V64QI blend mask (first 16 bits): ");
        // Print only first 16 bits for readability
        for (int i = 15; i >= 0; i--) {
            printf("%d", (mask >> i) & 1);
        }
        printf("...\n");
        printf("  First element of result: %d\n", first_elem);
    }
    
    // ========== V32HI (32 16-bit integers) ==========
    {
        // Create pattern: 0, 1, 2, ..., 31
        __m512i a = _mm512_setr_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        // Create pattern: 100, 101, 102, ..., 131
        __m512i b = _mm512_setr_epi16(
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
            116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131
        );
        
        // Create mask: select a where a[i] < 16, otherwise b
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, _mm512_set1_epi16(16));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, b, a);
        
        // Use result
        v32hi_result = result;
        
        // Extract first element
        int16_t first_elem = (int16_t)_mm512_extract_epi16(result, 0);
        final_sum += first_elem;
        
        printf("  V32HI blend mask (first 8 bits): ");
        for (int i = 7; i >= 0; i--) {
            printf("%d", (mask >> i) & 1);
        }
        printf("...\n");
        printf("  First element of result: %d\n", first_elem);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blend operations...\n");
    
    // ========== V32HF (32 half-precision floats) ==========
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);  // 1.0, 2.0, ..., 32.0
            b_data[i] = (_Float16)(i + 100); // 100.0, 101.0, ..., 131.0
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __m512h threshold = _mm512_set1_ph((_Float16)16.0);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, b, a);
        
        // Use result
        v32hf_result = result;
        
        // Extract first element
        _Float16 first_elem = _mm512_cvtph_f16(result);
        final_sum += (int64_t)first_elem;
        
        printf("  V32HF blend mask (first 8 bits): ");
        for (int i = 7; i >= 0; i--) {
            printf("%d", (mask >> i) & 1);
        }
        printf("...\n");
        printf("  First element of result: %f\n", (float)first_elem);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blend operations...\n");
    
    // ========== V32BF (32 brain float) ==========
    {
        // Brain float uses same intrinsics as half-precision
        __m512bh a, b;
        
        // Initialize with some pattern
        uint16_t a_data[32];
        uint16_t b_data[32];
        
        for (int i = 0; i < 32; i++) {
            // Simple pattern: alternating 0x3C00 (1.0) and 0x4000 (2.0) for a
            a_data[i] = (i % 2 == 0) ? 0x3C00 : 0x4000;  // 1.0 or 2.0 in bfloat16
            b_data[i] = 0x4040 + i;  // Different pattern for b
        }
        
        // Load data
        a = _mm512_loadu_epi16(a_data);
        b = _mm512_loadu_epi16(b_data);
        
        // Create a simple mask: select every other element from a
        __mmask32 mask = 0xAAAAAAAA;  // Binary: 10101010... pattern
        
        // Perform blend using the same intrinsic as half-precision
        __m512bh result = _mm512_mask_blend_ph(mask, b, a);
        
        // Use result
        v32bf_result = result;
        
        // Extract first element
        uint16_t first_elem = _mm512_extract_epi16(result, 0);
        final_sum += first_elem;
        
        printf("  V32BF blend mask (first 8 bits): ");
        for (int i = 7; i >= 0; i--) {
            printf("%d", (mask >> i) & 1);
        }
        printf("...\n");
        printf("  First element of result (hex): 0x%04x\n", first_elem);
    }
#endif // __AVX512BF16__

    printf("\nFinal aggregated sum: %ld\n", final_sum);
    
    // Force compiler to keep all results by using them in a volatile asm
    __asm__ volatile ("" : : "m"(v64qi_result), "m"(v32hi_result), "m"(v16si_result), 
                       "m"(v8di_result), "m"(v16sf_result), "m"(v8df_result)
#ifdef __AVX512FP16__
                       , "m"(v32hf_result)
#endif
#ifdef __AVX512BF16__
                       , "m"(v32bf_result)
#endif
                       );
    
    return 0;
}
