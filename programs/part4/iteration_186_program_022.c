#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512h global_v32hf;
volatile __m512bh global_v32bf;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512d global_v8df;
volatile __m512 global_v16sf;

// Function to print bits of a mask (for debugging)
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    // ==================== V16SF (16 single-precision floats) ====================
#ifdef __AVX512F__
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Perform masked blend
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = blended;
        
        // Extract first element and add to sum
        float first = _mm512_cvtss_f32(blended);
        result_sum += (int)first;
        
        printf("V16SF blend done, mask: 0x%04x, first element: %.1f\n", mask, first);
    }
#endif
    
    // ==================== V8DF (8 double-precision floats) ====================
#ifdef __AVX512F__
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform masked blend
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result to prevent optimization
        global_v8df = blended;
        
        // Extract first element and add to sum
        double first = _mm512_cvtsd_f64(blended);
        result_sum += (int)first;
        
        printf("V8DF blend done, mask: 0x%02x, first element: %.1f\n", mask, first);
    }
#endif
    
    // ==================== V16SI (16 32-bit integers) ====================
#ifdef __AVX512F__
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        // Perform masked blend
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_v16si = blended;
        
        // Extract first element and add to sum
        int first = _mm512_cvtsi512_si32(blended);
        result_sum += first;
        
        printf("V16SI blend done, mask: 0x%04x, first element: %d\n", mask, first);
    }
#endif
    
    // ==================== V8DI (8 64-bit integers) ====================
#ifdef __AVX512F__
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by checking if elements are odd
        __m512i ones = _mm512_set1_epi64(1);
        __mmask8 mask = _mm512_test_epi64_mask(a, ones);
        
        // Perform masked blend
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result to prevent optimization
        global_v8di = blended;
        
        // Extract first element and add to sum
        long long first = _mm512_cvtsi512_si64(blended);
        result_sum += (int)first;
        
        printf("V8DI blend done, mask: 0x%02x, first element: %lld\n", mask, first);
    }
#endif
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    // ==================== V64QI (64 8-bit integers) ====================
#ifdef __AVX512BW__
    {
        // Create pattern for 64 bytes
        int8_t a_data[64];
        int8_t b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 100 + i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select elements where (i % 3) == 0
        __m512i mod3 = _mm512_set1_epi8(3);
        __m512i zero = _mm512_setzero_si512();
        __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(3)), zero);
        
        // Perform masked blend
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = blended;
        
        // Extract first element and add to sum
        int8_t first = _mm512_cvtsi512_si32(blended) & 0xFF;
        result_sum += first;
        
        printf("V64QI blend done, mask bits: 0x%016lx..., first element: %d\n", 
               (unsigned long)(mask >> 32), first);
    }
#endif
    
    // ==================== V32HI (32 16-bit integers) ====================
#ifdef __AVX512BW__
    {
        int16_t a_data[32];
        int16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = i * 10;
            b_data[i] = 1000 + i * 20;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select elements > 150
        __m512i threshold = _mm512_set1_epi16(150);
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, threshold);
        
        // Perform masked blend
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hi = blended;
        
        // Extract first element and add to sum
        int16_t first = _mm512_cvtsi512_si32(blended) & 0xFFFF;
        result_sum += first;
        
        printf("V32HI blend done, mask: 0x%08x, first element: %d\n", mask, first);
    }
#endif
#endif // __AVX512BW__
    
    // ==================== V32HF (32 half-precision floats) ====================
#ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 0.5f);
            b_data[i] = (_Float16)(100.0f + i * 2.0f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: select elements < 10.0
        __m512h threshold = _mm512_set1_ph((_Float16)10.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_LT_OQ);
        
        // Perform masked blend
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hf = blended;
        
        // Extract first element and add to sum
        _Float16 first = _mm512_cvtsh_h(blended);
        result_sum += (int)first;
        
        printf("V32HF blend done, mask: 0x%08x, first element: %.2f\n", mask, (float)first);
    }
#endif
    
    // ==================== V32BF (32 brain float) ====================
#ifdef __AVX512BF16__
    printf("AVX-512BF16 supported\n");
    {
        // Note: BF16 uses same intrinsics as FP16 but different type
        __m512bh a, b;
        
        // Initialize with some pattern
        uint16_t a_data[32];
        uint16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern: alternating 0x3F80 (1.0) and 0x4000 (2.0)
            a_data[i] = (i % 2) ? 0x4000 : 0x3F80;
            b_data[i] = 0x4080 + i; // Different pattern
        }
        
        // Load as epi16 then convert
        __m512i a_epi16 = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b_epi16 = _mm512_loadu_si512((const __m512i*)b_data);
        
        a = (__m512bh)a_epi16;
        b = (__m512bh)b_epi16;
        
        // Create mask: select every third element
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if (i % 3 == 0) {
                mask |= (1ULL << i);
            }
        }
        
        // Perform masked blend
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32bf = blended;
        
        // Extract first element (as uint16_t) and add to sum
        uint16_t first = ((uint16_t*)&blended)[0];
        result_sum += first;
        
        printf("V32BF blend done, mask: 0x%08x, first element: 0x%04x\n", mask, first);
    }
#endif
    
    printf("Final result sum: %d\n", result_sum);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), "m"(global_v32hf),
                     "m"(global_v32bf), "m"(global_v16si), "m"(global_v8di),
                     "m"(global_v8df), "m"(global_v16sf));
    
    return 0;
}
