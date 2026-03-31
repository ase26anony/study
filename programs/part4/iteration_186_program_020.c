#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512  global_v16sf;
volatile __m512d global_v8df;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
#endif

// Function to print 64-bit mask
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

// Function to print 32-bit mask
void print_mask32(__mmask32 mask) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

// Function to print 16-bit mask
void print_mask16(__mmask16 mask) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

// Function to print 8-bit mask
void print_mask8(__mmask8 mask) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512 blend operations...\n");
    
    // ============================================
    // V64QI - 64-byte vectors of 8-bit integers
    // ============================================
#ifdef __AVX512BW__
    {
        printf("\nTesting V64QI (_mm512_mask_blend_epi8):\n");
        
        // Initialize source vectors with alternating patterns
        int8_t a_data[64], b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 64 - i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask by comparing elements
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, cmp_val);
        
        printf("Mask (64-bit): ");
        print_mask64(mask);
        
        // Perform the blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile global to prevent optimization
        global_v64qi = result;
        
        // Use result in computation
        __m512i sum = _mm512_add_epi8(result, _mm512_set1_epi8(1));
        int8_t sum_data[64];
        _mm512_storeu_si512((__m512i*)sum_data, sum);
        result_sum += sum_data[0];
        
        printf("First element of blended sum: %d\n", sum_data[0]);
    }
    
    // ============================================
    // V32HI - 32-word vectors of 16-bit integers
    // ============================================
    {
        printf("\nTesting V32HI (_mm512_mask_blend_epi16):\n");
        
        // Initialize source vectors
        int16_t a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = i * 2;
            b_data[i] = i * 3;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask
        __m512i cmp_val = _mm512_set1_epi16(30);
        __mmask32 mask = _mm512_cmpgt_epi16_mask(cmp_val, a);
        
        printf("Mask (32-bit): ");
        print_mask32(mask);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        global_v32hi = result;
        
        // Use result
        __m512i sum = _mm512_add_epi16(result, _mm512_set1_epi16(1));
        int16_t sum_data[32];
        _mm512_storeu_si512((__m512i*)sum_data, sum);
        result_sum += sum_data[0];
        
        printf("First element of blended sum: %d\n", sum_data[0]);
    }
#endif // __AVX512BW__
    
    // ============================================
    // V16SI - 16-dword vectors of 32-bit integers
    // ============================================
    {
        printf("\nTesting V16SI (_mm512_mask_blend_epi32):\n");
        
        // Initialize source vectors
        int32_t a_data[16], b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 10;
            b_data[i] = i * 20;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask
        __m512i cmp_val = _mm512_set1_epi32(80);
        __mmask16 mask = _mm512_cmplt_epi32_mask(a, cmp_val);
        
        printf("Mask (16-bit): ");
        print_mask16(mask);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        global_v16si = result;
        
        // Use result
        __m512i sum = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        int32_t sum_data[16];
        _mm512_storeu_si512((__m512i*)sum_data, sum);
        result_sum += sum_data[0];
        
        printf("First element of blended sum: %d\n", sum_data[0]);
    }
    
    // ============================================
    // V8DI - 8-qword vectors of 64-bit integers
    // ============================================
    {
        printf("\nTesting V8DI (_mm512_mask_blend_epi64):\n");
        
        // Initialize source vectors
        int64_t a_data[8], b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = i * 100LL;
            b_data[i] = i * 200LL;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask
        __m512i cmp_val = _mm512_set1_epi64(400);
        __mmask8 mask = _mm512_cmplt_epi64_mask(a, cmp_val);
        
        printf("Mask (8-bit): ");
        print_mask8(mask);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        global_v8di = result;
        
        // Use result
        __m512i sum = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        int64_t sum_data[8];
        _mm512_storeu_si512((__m512i*)sum_data, sum);
        result_sum += (int)sum_data[0];
        
        printf("First element of blended sum: %ld\n", sum_data[0]);
    }
    
    // ============================================
    // V16SF - 16-dword vectors of single-precision floats
    // ============================================
    {
        printf("\nTesting V16SF (_mm512_mask_blend_ps):\n");
        
        // Initialize source vectors
        float a_data[16], b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 1.5f;
            b_data[i] = i * 2.5f;
        }
        
        __m512 a = _mm512_loadu_ps(a_data);
        __m512 b = _mm512_loadu_ps(b_data);
        
        // Create mask using comparison
        __m512 cmp_val = _mm512_set1_ps(12.0f);
        __mmask16 mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_LT_OQ);
        
        printf("Mask (16-bit): ");
        print_mask16(mask);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        global_v16sf = result;
        
        // Use result
        __m512 sum = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        float sum_data[16];
        _mm512_storeu_ps(sum_data, sum);
        result_sum += (int)sum_data[0];
        
        printf("First element of blended sum: %f\n", sum_data[0]);
    }
    
    // ============================================
    // V8DF - 8-qword vectors of double-precision floats
    // ============================================
    {
        printf("\nTesting V8DF (_mm512_mask_blend_pd):\n");
        
        // Initialize source vectors
        double a_data[8], b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = i * 1.25;
            b_data[i] = i * 2.75;
        }
        
        __m512d a = _mm512_loadu_pd(a_data);
        __m512d b = _mm512_loadu_pd(b_data);
        
        // Create mask
        __m512d cmp_val = _mm512_set1_pd(5.0);
        __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_LT_OQ);
        
        printf("Mask (8-bit): ");
        print_mask8(mask);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        global_v8df = result;
        
        // Use result
        __m512d sum = _mm512_add_pd(result, _mm512_set1_pd(1.0));
        double sum_data[8];
        _mm512_storeu_pd(sum_data, sum);
        result_sum += (int)sum_data[0];
        
        printf("First element of blended sum: %f\n", sum_data[0]);
    }
    
#ifdef __AVX512FP16__
    // ============================================
    // V32HF - 32-word vectors of half-precision floats
    // ============================================
    {
        printf("\nTesting V32HF (_mm512_mask_blend_ph):\n");
        
        // Initialize source vectors
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 0.5f);
            b_data[i] = (_Float16)(i * 1.5f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask
        __m512h cmp_val = _mm512_set1_ph((_Float16)8.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_LT_OQ);
        
        printf("Mask (32-bit): ");
        print_mask32(mask);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        global_v32hf = result;
        
        // Use result
        __m512h sum = _mm512_add_ph(result, _mm512_set1_ph((_Float16)1.0f));
        _Float16 sum_data[32];
        _mm512_storeu_ph(sum_data, sum);
        result_sum += (int)sum_data[0];
        
        printf("First element of blended sum: %f\n", (float)sum_data[0]);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // ============================================
    // V32BF - 32-word vectors of brain float
    // ============================================
    {
        printf("\nTesting V32BF (_mm512_mask_blend_ph):\n");
        
        // Note: BF16 uses the same intrinsics as FP16 for blending
        
        // Initialize source vectors
        __bf16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern for bfloat16
            a_data[i] = (__bf16)(i & 0xFF);  // Lower 8 bits
            b_data[i] = (__bf16)((i << 8) & 0xFF00);  // Upper 8 bits
        }
        
        __m512bh a = _mm512_loadu_ph((const void*)a_data);
        __m512bh b = _mm512_loadu_ph((const void*)b_data);
        
        // Create mask by comparing with zero
        __m512bh zero = _mm512_setzero_ph();
        __mmask32 mask = _mm512_cmp_ph_mask(a, zero, _CMP_NEQ_OQ);
        
        printf("Mask (32-bit): ");
        print_mask32(mask);
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        global_v32bf = result;
        
        // Use result - convert to float for computation
        __m512 result_f32 = _mm512_cvtpbh_ps(_mm512_castph_si512(result));
        float sum_data[16];
        _mm512_storeu_ps(sum_data, result_f32);
        result_sum += (int)sum_data[0];
        
        printf("First element of converted result: %f\n", sum_data[0]);
    }
#endif // __AVX512BF16__
    
    printf("\nFinal aggregated result: %d\n", result_sum);
    printf("All AVX-512 blend operations tested.\n");
    
#else
    printf("AVX-512 not supported on this platform.\n");
#endif // __AVX512F__
    
    return 0;
}
