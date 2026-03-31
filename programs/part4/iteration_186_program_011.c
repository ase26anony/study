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
void print_mask8(__mmask8 m) {
    for (int i = 7; i >= 0; i--) printf("%d", (m >> i) & 1);
    printf("\n");
}

void print_mask16(__mmask16 m) {
    for (int i = 15; i >= 0; i--) printf("%d", (m >> i) & 1);
    printf("\n");
}

void print_mask32(__mmask32 m) {
    for (int i = 31; i >= 0; i--) printf("%d", (m >> i) & 1);
    printf("\n");
}

void print_mask64(__mmask64 m) {
    for (int i = 63; i >= 0; i--) printf("%d", (m >> i) & 1);
    printf("\n");
}

int main() {
    int total_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512 blend operations...\n");
    
    // ==================== V64QI (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
    {
        printf("Testing V64QI (_mm512_mask_blend_epi8)...\n");
        
        // Initialize source vectors with alternating patterns
        int8_t a64qi_data[64];
        int8_t b64qi_data[64];
        for (int i = 0; i < 64; i++) {
            a64qi_data[i] = i % 16;          // 0-15 repeating
            b64qi_data[i] = 31 - (i % 16);   // 31-16 repeating
        }
        
        __m512i a64qi = _mm512_loadu_si512((const __m512i*)a64qi_data);
        __m512i b64qi = _mm512_loadu_si512((const __m512i*)b64qi_data);
        
        // Create mask by comparing elements: mask = (a > 7)
        __mmask64 mask64qi = _mm512_cmpgt_epi8_mask(a64qi, _mm512_set1_epi8(7));
        
        // Perform blend: result = mask ? b : a
        __m512i result64qi = _mm512_mask_blend_epi8(mask64qi, a64qi, b64qi);
        
        // Store to volatile to prevent optimization
        v64qi_result = result64qi;
        
        // Extract first element and add to sum
        int8_t first_elem = _mm512_extract_epi8(result64qi, 0);
        total_sum += first_elem;
        
        printf("  V64QI mask bits (first 16): ");
        for (int i = 0; i < 16; i++) printf("%d", (mask64qi >> i) & 1);
        printf("...\n");
    }
    
    // ==================== V32HI (32 x 16-bit integers) ====================
    {
        printf("Testing V32HI (_mm512_mask_blend_epi16)...\n");
        
        // Initialize source vectors
        int16_t a32hi_data[32];
        int16_t b32hi_data[32];
        for (int i = 0; i < 32; i++) {
            a32hi_data[i] = i * 100;
            b32hi_data[i] = i * 200;
        }
        
        __m512i a32hi = _mm512_loadu_si512((const __m512i*)a32hi_data);
        __m512i b32hi = _mm512_loadu_si512((const __m512i*)b32hi_data);
        
        // Create mask: select where a is even
        __mmask32 mask32hi = _mm512_test_epi16_mask(a32hi, _mm512_set1_epi16(1));
        mask32hi = ~mask32hi;  // Invert: select where LSB is 0 (even)
        
        // Perform blend
        __m512i result32hi = _mm512_mask_blend_epi16(mask32hi, a32hi, b32hi);
        
        // Store to volatile
        v32hi_result = result32hi;
        
        // Extract and accumulate
        int16_t first_elem = _mm512_extract_epi16(result32hi, 0);
        total_sum += first_elem;
        
        printf("  V32HI mask bits (first 8): ");
        for (int i = 0; i < 8; i++) printf("%d", (mask32hi >> i) & 1);
        printf("...\n");
    }
#endif  // __AVX512BW__
    
    // ==================== V16SI (16 x 32-bit integers) ====================
    {
        printf("Testing V16SI (_mm512_mask_blend_epi32)...\n");
        
        // Initialize source vectors
        int32_t a16si_data[16];
        int32_t b16si_data[16];
        for (int i = 0; i < 16; i++) {
            a16si_data[i] = i * 1000;
            b16si_data[i] = i * 2000;
        }
        
        __m512i a16si = _mm512_loadu_si512((const __m512i*)a16si_data);
        __m512i b16si = _mm512_loadu_si512((const __m512i*)b16si_data);
        
        // Create mask using comparison
        __mmask16 mask16si = _mm512_cmpeq_epi32_mask(
            _mm512_and_epi32(a16si, _mm512_set1_epi32(1)),
            _mm512_setzero_si512()
        );
        
        // Perform blend
        __m512i result16si = _mm512_mask_blend_epi32(mask16si, a16si, b16si);
        
        // Store to volatile
        v16si_result = result16si;
        
        // Extract and accumulate
        int32_t first_elem = _mm512_extract_epi32(result16si, 0);
        total_sum += first_elem;
        
        printf("  V16SI mask: ");
        print_mask16(mask16si);
    }
    
    // ==================== V8DI (8 x 64-bit integers) ====================
    {
        printf("Testing V8DI (_mm512_mask_blend_epi64)...\n");
        
        // Initialize source vectors
        int64_t a8di_data[8];
        int64_t b8di_data[8];
        for (int i = 0; i < 8; i++) {
            a8di_data[i] = i * 10000LL;
            b8di_data[i] = i * 30000LL;
        }
        
        __m512i a8di = _mm512_loadu_si512((const __m512i*)a8di_data);
        __m512i b8di = _mm512_loadu_si512((const __m512i*)b8di_data);
        
        // Create mask: select where index is odd
        __mmask8 mask8di = 0;
        for (int i = 0; i < 8; i++) {
            if (i % 2 == 1) mask8di |= (1ULL << i);
        }
        
        // Perform blend
        __m512i result8di = _mm512_mask_blend_epi64(mask8di, a8di, b8di);
        
        // Store to volatile
        v8di_result = result8di;
        
        // Extract and accumulate
        int64_t first_elem = _mm512_extract_epi64(result8di, 0);
        total_sum += (int)first_elem;
        
        printf("  V8DI mask: ");
        print_mask8(mask8di);
    }
    
    // ==================== V8DF (8 x double-precision floats) ====================
    {
        printf("Testing V8DF (_mm512_mask_blend_pd)...\n");
        
        // Initialize source vectors
        double a8df_data[8];
        double b8df_data[8];
        for (int i = 0; i < 8; i++) {
            a8df_data[i] = i * 1.5;
            b8df_data[i] = i * 2.5;
        }
        
        __m512d a8df = _mm512_loadu_pd(a8df_data);
        __m512d b8df = _mm512_loadu_pd(b8df_data);
        
        // Create mask using comparison: select where a > threshold
        __mmask8 mask8df = _mm512_cmp_pd_mask(a8df, _mm512_set1_pd(5.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result8df = _mm512_mask_blend_pd(mask8df, a8df, b8df);
        
        // Store to volatile
        v8df_result = result8df;
        
        // Extract and accumulate (convert to int)
        double first_elem = _mm512_cvtsd_f64(result8df);
        total_sum += (int)first_elem;
        
        printf("  V8DF mask: ");
        print_mask8(mask8df);
    }
    
    // ==================== V16SF (16 x single-precision floats) ====================
    {
        printf("Testing V16SF (_mm512_mask_blend_ps)...\n");
        
        // Initialize source vectors
        float a16sf_data[16];
        float b16sf_data[16];
        for (int i = 0; i < 16; i++) {
            a16sf_data[i] = i * 0.5f;
            b16sf_data[i] = i * 1.5f;
        }
        
        __m512 a16sf = _mm512_loadu_ps(a16sf_data);
        __m512 b16sf = _mm512_loadu_ps(b16sf_data);
        
        // Create mask using comparison: select where a < threshold
        __mmask16 mask16sf = _mm512_cmp_ps_mask(a16sf, _mm512_set1_ps(4.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512 result16sf = _mm512_mask_blend_ps(mask16sf, a16sf, b16sf);
        
        // Store to volatile
        v16sf_result = result16sf;
        
        // Extract and accumulate (convert to int)
        float first_elem = _mm512_cvtss_f32(result16sf);
        total_sum += (int)first_elem;
        
        printf("  V16SF mask: ");
        print_mask16(mask16sf);
    }
    
#ifdef __AVX512FP16__
    // ==================== V32HF (32 x half-precision floats) ====================
    {
        printf("Testing V32HF (_mm512_mask_blend_ph)...\n");
        
        // Initialize source vectors
        _Float16 a32hf_data[32];
        _Float16 b32hf_data[32];
        for (int i = 0; i < 32; i++) {
            a32hf_data[i] = (_Float16)(i * 0.25f);
            b32hf_data[i] = (_Float16)(i * 0.75f);
        }
        
        __m512h a32hf = _mm512_loadu_ph(a32hf_data);
        __m512h b32hf = _mm512_loadu_ph(b32hf_data);
        
        // Create mask using comparison
        __mmask32 mask32hf = _mm512_cmp_ph_mask(a32hf, _mm512_set1_ph(8.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512h result32hf = _mm512_mask_blend_ph(mask32hf, a32hf, b32hf);
        
        // Store to volatile
        v32hf_result = result32hf;
        
        // Extract and accumulate (convert to int)
        _Float16 first_elem = _mm512_cvtsh_h(result32hf);
        total_sum += (int)first_elem;
        
        printf("  V32HF mask bits (first 8): ");
        for (int i = 0; i < 8; i++) printf("%d", (mask32hf >> i) & 1);
        printf("...\n");
    }
#endif  // __AVX512FP16__
    
#ifdef __AVX512BF16__
    // ==================== V32BF (32 x brain float) ====================
    {
        printf("Testing V32BF (_mm512_mask_blend_ph for bfloat16)...\n");
        
        // Initialize source vectors (using __m512bh for bfloat16)
        __m512bh a32bf, b32bf;
        
        // Load from float arrays (convert from float to bfloat16)
        float temp_a[32], temp_b[32];
        for (int i = 0; i < 32; i++) {
            temp_a[i] = i * 0.3f;
            temp_b[i] = i * 0.7f;
        }
        
        // Convert float to bfloat16
        a32bf = (__m512bh)_mm512_loadu_si512(temp_a);
        b32bf = (__m512bh)_mm512_loadu_si512(temp_b);
        
        // Create mask (use the same comparison as for half-precision)
        __m512h a32bf_as_half = (__m512h)a32bf;
        __m512h b32bf_as_half = (__m512h)b32bf;
        __mmask32 mask32bf = _mm512_cmp_ph_mask(a32bf_as_half, 
                                               _mm512_set1_ph(10.0f), 
                                               _CMP_GT_OQ);
        
        // Perform blend (using the same intrinsic as half-precision)
        __m512h result32bf = _mm512_mask_blend_ph(mask32bf, 
                                                 a32bf_as_half, 
                                                 b32bf_as_half);
        
        // Store to volatile
        v32bf_result = (__m512bh)result32bf;
        
        // Extract first element (convert through float)
        _Float16 first_elem_half = _mm512_cvtsh_h(result32bf);
        float first_elem_float = (float)first_elem_half;
        total_sum += (int)first_elem_float;
        
        printf("  V32BF mask bits (first 8): ");
        for (int i = 0; i < 8; i++) printf("%d", (mask32bf >> i) & 1);
        printf("...\n");
    }
#endif  // __AVX512BF16__
    
    printf("\nTotal sum from all blend operations: %d\n", total_sum);
    printf("(This value is meaningless but ensures computations aren't optimized away)\n");
    
#else
    printf("AVX-512 not supported on this compiler/platform.\n");
#endif  // __AVX512F__
    
    return 0;
}
