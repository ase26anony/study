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
    printf("Testing AVX-512F blend operations...\n");
    
    // ================= V16SI (16 x 32-bit integers) =================
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        // Perform masked blend
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        int sum = 0;
        int* ptr = (int*)&res;
        for (int i = 0; i < 16; i++) {
            sum += ptr[i];
        }
        result_sum += sum;
        global_512i = res;
        
        printf("  V16SI blend result sum: %d\n", sum);
    }
    
    // ================= V8DI (8 x 64-bit integers) =================
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking if elements are even
        __mmask8 mask = _mm512_cmpeq_epi64_mask(
            _mm512_and_epi64(a, _mm512_set1_epi64(1)),
            _mm512_setzero_si512()
        );
        
        // Perform masked blend
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        long long sum = 0;
        long long* ptr = (long long*)&res;
        for (int i = 0; i < 8; i++) {
            sum += ptr[i];
        }
        result_sum += (int)sum;
        global_512i = res;
        
        printf("  V8DI blend result sum: %lld\n", sum);
    }
    
    // ================= V8DF (8 x double precision) =================
    {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5);
        
        // Create mask: a > 3.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.0), _CMP_GT_OQ);
        
        // Perform masked blend
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        double sum = 0.0;
        double* ptr = (double*)&res;
        for (int i = 0; i < 8; i++) {
            sum += ptr[i];
        }
        result_sum += (int)sum;
        global_512d = res;
        
        printf("  V8DF blend result sum: %f\n", sum);
    }
    
    // ================= V16SF (16 x single precision) =================
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f,
                                 8.5f,9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f);
        
        // Create mask: a < 8.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        // Perform masked blend
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        float sum = 0.0f;
        float* ptr = (float*)&res;
        for (int i = 0; i < 16; i++) {
            sum += ptr[i];
        }
        result_sum += (int)sum;
        global_512f = res;
        
        printf("  V16SF blend result sum: %f\n", sum);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // ================= V64QI (64 x 8-bit integers) =================
    {
        // Create pattern data
        uint8_t data_a[64], data_b[64];
        for (int i = 0; i < 64; i++) {
            data_a[i] = i;
            data_b[i] = 63 - i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)data_a);
        __m512i b = _mm512_loadu_si512((const __m512i*)data_b);
        
        // Create mask: select where a[i] < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        // Perform masked blend
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        int sum = 0;
        uint8_t* ptr = (uint8_t*)&res;
        for (int i = 0; i < 64; i++) {
            sum += ptr[i];
        }
        result_sum += sum;
        global_512i = res;
        
        printf("  V64QI blend result sum: %d\n", sum);
    }
    
    // ================= V32HI (32 x 16-bit integers) =================
    {
        // Create pattern data
        int16_t data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = i * 2;
            data_b[i] = i * 3;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)data_a);
        __m512i b = _mm512_loadu_si512((const __m512i*)data_b);
        
        // Create mask: select where a[i] is even
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        mask = ~mask; // Invert: select where LSB is 0 (even)
        
        // Perform masked blend
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        int sum = 0;
        int16_t* ptr = (int16_t*)&res;
        for (int i = 0; i < 32; i++) {
            sum += ptr[i];
        }
        result_sum += sum;
        global_512i = res;
        
        printf("  V32HI blend result sum: %d\n", sum);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // ================= V32HF (32 x half precision) =================
    {
        // Initialize with pattern
        _Float16 data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            data_a[i] = (_Float16)i;
            data_b[i] = (_Float16)(31 - i);
        }
        
        __m512h a = _mm512_loadu_ph(data_a);
        __m512h b = _mm512_loadu_ph(data_b);
        
        // Create mask: a > 15.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.0), _CMP_GT_OQ);
        
        // Perform masked blend
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        _Float16 sum = 0.0;
        _Float16* ptr = (_Float16*)&res;
        for (int i = 0; i < 32; i++) {
            sum += ptr[i];
        }
        result_sum += (int)sum;
        global_512h = res;
        
        printf("  V32HF blend result sum: %f\n", (double)sum);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // ================= V32BF (32 x brain float) =================
    {
        // Note: BF16 uses the same intrinsics as FP16 for blend operations
        // but with different data types
        
        // Initialize with pattern (using __m512bh for BF16)
        __m512bh a, b;
        
        // Create simple test pattern
        uint16_t data_a[32], data_b[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern: i as bfloat16
            data_a[i] = i << 8;  // bfloat16 representation of small integers
            data_b[i] = (31 - i) << 8;
        }
        
        a = _mm512_loadu_si512((const __m512i*)data_a);
        b = _mm512_loadu_si512((const __m512i*)data_b);
        
        // Convert to __m512h for comparison (BF16 can use FP16 comparison)
        __m512h a_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(a));
        __m512h b_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), _mm512_castsi512_ps(b));
        
        // Create mask: compare
        __mmask32 mask = _mm512_cmp_ph_mask(a_h, b_h, _CMP_LT_OQ);
        
        // Perform masked blend (using FP16 intrinsic for BF16)
        __m512bh res = (__m512bh)_mm512_mask_blend_ph(mask, 
            (__m512h)a, (__m512h)b);
        
        // Use result
        int sum = 0;
        uint16_t* ptr = (uint16_t*)&res;
        for (int i = 0; i < 32; i++) {
            sum += ptr[i];
        }
        result_sum += sum;
        
        printf("  V32BF blend result sum: %d\n", sum);
    }
#endif // __AVX512BF16__

    printf("Final aggregated result: %d\n", result_sum);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_512i), "m"(global_512d), "m"(global_512f)
#ifdef __AVX512FP16__
                  , "m"(global_512h)
#endif
                  );
    
    return 0;
}
