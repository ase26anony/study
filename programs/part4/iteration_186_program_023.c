#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_512i;
volatile __m512d global_512d;
volatile __m512 global_512f;

#ifdef __AVX512FP16__
volatile __m512h global_512h;
#endif

// Function to print bits of a mask (for debugging)
void print_mask8(__mmask8 m) {
    for (int i = 7; i >= 0; i--) printf("%d", (m >> i) & 1);
}

void print_mask16(__mmask16 m) {
    for (int i = 15; i >= 0; i--) printf("%d", (m >> i) & 1);
}

void print_mask32(__mmask32 m) {
    for (int i = 31; i >= 0; i--) printf("%d", (m >> i) & 1);
}

void print_mask64(__mmask64 m) {
    for (int i = 63; i >= 0; i--) printf("%d", (m >> i) & 1);
}

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    
    // ===== V16SI (16 x 32-bit integers) =====
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_512i = res;
        
        // Extract first element and add to sum
        int first = _mm512_extract_epi32(res, 0);
        result_sum += first;
        
        printf("  V16SI mask: ");
        print_mask16(mask);
        printf(", first element: %d\n", first);
    }
    
    // ===== V8DI (8 x 64-bit integers) =====
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(70,60,50,40,30,20,10,0);
        
        // Create mask using equality test
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(0));
        
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        global_512i = res;
        long long first = _mm512_extract_epi64(res, 0);
        result_sum += (int)first;
        
        printf("  V8DI mask: ");
        print_mask8(mask);
        printf(", first element: %lld\n", first);
    }
    
    // ===== V16SF (16 x single-precision floats) =====
    {
        __m512 a = _mm512_set_ps(15.5f,14.5f,13.5f,12.5f,11.5f,10.5f,9.5f,8.5f,
                                  7.5f,6.5f,5.5f,4.5f,3.5f,2.5f,1.5f,0.5f);
        __m512 b = _mm512_set_ps(150.0f,140.0f,130.0f,120.0f,110.0f,100.0f,90.0f,80.0f,
                                  70.0f,60.0f,50.0f,40.0f,30.0f,20.0f,10.0f,0.0f);
        
        // Create mask using float comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        global_512f = res;
        float first = _mm512_cvtss_f32(res);
        result_sum += (int)first;
        
        printf("  V16SF mask: ");
        print_mask16(mask);
        printf(", first element: %.2f\n", first);
    }
    
    // ===== V8DF (8 x double-precision floats) =====
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(70.0,60.0,50.0,40.0,30.0,20.0,10.0,0.0);
        
        // Create mask using double comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        global_512d = res;
        double first = _mm512_cvtsd_f64(res);
        result_sum += (int)first;
        
        printf("  V8DF mask: ");
        print_mask8(mask);
        printf(", first element: %.2f\n", first);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blends...\n");
    
    // ===== V64QI (64 x 8-bit integers) =====
    {
        // Create pattern: 0,1,2,...,63
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        // Create pattern: 100,101,102,...,163
        __m512i b = _mm512_set_epi8(
            163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,
            147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,
            131,130,129,128,127,126,125,124,123,122,121,120,119,118,117,116,
            115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100);
        
        // Create mask by checking if element < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        global_512i = res;
        
        // Extract and sum first 4 bytes
        uint8_t first4[4];
        _mm512_mask_storeu_epi8(first4, 0xF, res);
        result_sum += first4[0] + first4[1] + first4[2] + first4[3];
        
        printf("  V64QI: mask bits 0-7: ");
        print_mask8((__mmask8)(mask & 0xFF));
        printf(", sum of first 4 bytes: %d\n", first4[0]+first4[1]+first4[2]+first4[3]);
    }
    
    // ===== V32HI (32 x 16-bit integers) =====
    {
        // Create pattern: 0,1,2,...,31
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        // Create pattern: 100,101,...,131
        __m512i b = _mm512_set_epi16(
            131,130,129,128,127,126,125,124,123,122,121,120,119,118,117,116,
            115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100);
        
        // Create mask by checking if element is even
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        global_512i = res;
        
        // Extract first element
        short first = _mm512_extract_epi16(res, 0);
        result_sum += first;
        
        printf("  V32HI mask bits 0-7: ");
        print_mask8((__mmask8)(mask & 0xFF));
        printf(", first element: %d\n", first);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blends...\n");
    
    // ===== V32HF (32 x half-precision floats) =====
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 0.5f);
            b_data[i] = (_Float16)(i * 2.0f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 8.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_LT_OQ);
        
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile global
        _mm512_storeu_ph((_Float16*)&global_512h, res);
        
        // Extract first element
        _Float16 first;
        _mm512_mask_storeu_ph(&first, 1, res);
        result_sum += (int)first;
        
        printf("  V32HF mask bits 0-7: ");
        print_mask8((__mmask8)(mask & 0xFF));
        printf(", first element: %.2f\n", (float)first);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blends...\n");
    
    // ===== V32BF (32 x brain float) =====
    // Note: BF16 uses the same intrinsics as FP16 for blend operations
    {
        // Initialize with pattern (using __m512bh for BF16)
        __m512bh a = _mm512_set1_epi16(0x3F80); // 1.0 in BF16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in BF16
        
        // Create alternating pattern for a
        uint16_t a_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (i % 2 == 0) ? 0x3F80 : 0x3F00; // 1.0 and 0.5 in BF16
        }
        a = _mm512_loadu_epi16(a_data);
        
        // Create mask by checking pattern
        __mmask32 mask = _mm512_test_epi16_mask((__m512i)a, _mm512_set1_epi16(0x0080));
        
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Store result
        uint16_t res_data[32];
        _mm512_storeu_epi16(res_data, (__m512i)res);
        
        // Use first element
        result_sum += res_data[0];
        
        printf("  V32BF mask bits 0-7: ");
        print_mask8((__mmask8)(mask & 0xFF));
        printf(", first element: 0x%04X\n", res_data[0]);
    }
#endif // __AVX512BF16__

    printf("\nFinal result sum: %d\n", result_sum);
    
    // Force use of all results through inline assembly
    asm volatile ("" : : "m"(global_512i), "m"(global_512d), "m"(global_512f)
#ifdef __AVX512FP16__
                  , "m"(global_512h)
#endif
                  );
    
    return 0;
}
