#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi __attribute__((used));
volatile __m512i global_v32hi __attribute__((used));
volatile __m512i global_v16si __attribute__((used));
volatile __m512i global_v8di __attribute__((used));
volatile __m512 global_v16sf __attribute__((used));
volatile __m512d global_v8df __attribute__((used));

#ifdef __AVX512FP16__
volatile __m512h global_v32hf __attribute__((used));
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf __attribute__((used));
#endif

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    // Seed for reproducible patterns
    unsigned int seed = 0x12345678;
    
    // ==================== V64QI (64-byte vectors of 8-bit integers) ====================
#ifdef __AVX512BW__
    {
        // Initialize source vectors with alternating patterns
        int8_t a_data[64], b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = (int8_t)(i * 3 + seed);
            b_data[i] = (int8_t)(i * 5 - seed);
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask by comparing elements (non-zero mask ensures blend isn't optimized away)
        __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)), 
                                               _mm512_setzero_si512());
        
        // Perform the blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = result;
        
        // Extract first element for verification
        int8_t first_elem = _mm512_extract_epi8(result, 0);
        result_sum += first_elem;
    }
    
    // ==================== V32HI (32-word vectors of 16-bit integers) ====================
    {
        int16_t a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (int16_t)(i * 7 + seed);
            b_data[i] = (int16_t)(i * 11 - seed);
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask using comparison
        __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)), 
                                                _mm512_setzero_si512());
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        global_v32hi = result;
        
        int16_t first_elem = _mm512_extract_epi16(result, 0);
        result_sum += first_elem;
    }
#endif // __AVX512BW__
    
    // ==================== V16SI (16-dword vectors of 32-bit integers) ====================
    {
        int32_t a_data[16], b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 13 + seed;
            b_data[i] = i * 17 - seed;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_si512(a, _mm512_set1_epi32(1)), 
                                                _mm512_setzero_si512());
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result in computation
        int32_t first_elem = _mm512_extract_epi32(result, 0);
        result_sum += first_elem;
        
        // Additional use to prevent optimization
        __m512i temp = _mm512_add_epi32(result, _mm512_set1_epi32(1));
        global_v16si = temp;
    }
    
    // ==================== V8DI (8-qword vectors of 64-bit integers) ====================
    {
        int64_t a_data[8], b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = (int64_t)i * 19 + seed;
            b_data[i] = (int64_t)i * 23 - seed;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask using comparison
        __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_si512(a, _mm512_set1_epi64(1)), 
                                               _mm512_setzero_si512());
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        global_v8di = result;
        
        int64_t first_elem = _mm512_extract_epi64(result, 0);
        result_sum += (int)first_elem;
    }
    
    // ==================== V8DF (8-qword vectors of double-precision floats) ====================
    {
        double a_data[8], b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = i * 1.5 + seed * 0.01;
            b_data[i] = i * 2.5 - seed * 0.01;
        }
        
        __m512d a = _mm512_loadu_pd(a_data);
        __m512d b = _mm512_loadu_pd(b_data);
        
        // Create mask using floating-point comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(0.0), _CMP_LT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        global_v8df = result;
        
        // Use result in computation
        double first_elem = _mm512_cvtsd_f64(result);
        result_sum += (int)first_elem;
    }
    
    // ==================== V16SF (16-dword vectors of single-precision floats) ====================
    {
        float a_data[16], b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 0.75f + seed * 0.001f;
            b_data[i] = i * 1.25f - seed * 0.001f;
        }
        
        __m512 a = _mm512_loadu_ps(a_data);
        __m512 b = _mm512_loadu_ps(b_data);
        
        // Create mask using floating-point comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(0.0f), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        global_v16sf = result;
        
        // Use result in computation
        float first_elem = _mm512_cvtss_f32(result);
        result_sum += (int)first_elem;
    }
    
#ifdef __AVX512FP16__
    // ==================== V32HF (32-word vectors of half-precision floats) ====================
    {
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 0.25f + seed * 0.0001f);
            b_data[i] = (_Float16)(i * 0.5f - seed * 0.0001f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask using half-precision comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(0.0f), _CMP_LT_OQ);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        global_v32hf = result;
        
        // Extract and use first element
        _Float16 first_elem = _mm512_cvtsh_h(result);
        result_sum += (int)first_elem;
    }
#endif // __AVX512FP16__
    
#ifdef __AVX512BF16__
    // ==================== V32BF (32-word vectors of brain float) ====================
    {
        // BF16 uses the same intrinsics as FP16
        __m512bh a, b;
        
        // Initialize with patterns
        uint16_t a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern for BF16 values
            a_data[i] = (i * 3 + seed) & 0xFFFF;
            b_data[i] = (i * 5 - seed) & 0xFFFF;
        }
        
        a = _mm512_loadu_si512((const __m512i*)a_data);
        b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask (using integer comparison since BF16 comparison intrinsics might not exist)
        __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_castsi512_si512((__m512i)a), 
                                                _mm512_setzero_si512());
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        global_v32bf = result;
        
        // Extract first element
        uint16_t first_elem = _mm512_extract_epi16(_mm512_castsi512_si512((__m512i)result), 0);
        result_sum += first_elem;
    }
#endif // __AVX512BF16__
    
#endif // __AVX512F__
    
    printf("Result sum: %d\n", result_sum);
    
    // Additional volatile assembly to ensure all operations are used
    asm volatile ("" : : "r"(global_v64qi), "r"(global_v32hi), "r"(global_v16si),
                       "r"(global_v8di), "r"(global_v16sf), "r"(global_v8df)
#ifdef __AVX512FP16__
                       , "r"(global_v32hf)
#endif
#ifdef __AVX512BF16__
                       , "r"(global_v32bf)
#endif
                       : "memory");
    
    return 0;
}
