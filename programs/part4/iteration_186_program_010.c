#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512 global_v16sf;
volatile __m512d global_v8df;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
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
    uint64_t final_result = 0;
    
#ifdef __AVX512BW__
    // ==================== V64QImode ====================
    {
        printf("Testing V64QImode (64x 8-bit integers)...\n");
        
        // Initialize source vectors with distinct patterns
        __m512i a64qi = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b64qi = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create a non-trivial mask by comparing elements
        __mmask64 mask64qi = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a64qi, _mm512_set1_epi8(1)),
            _mm512_setzero_si512()
        );
        
        // Perform the masked blend
        __m512i result64qi = _mm512_mask_blend_epi8(mask64qi, a64qi, b64qi);
        
        // Use the result to prevent optimization
        global_v64qi = result64qi;
        
        // Extract first element and add to final result
        int8_t first_elem = _mm512_extract_epi8(result64qi, 0);
        final_result += (uint64_t)first_elem;
        
        printf("  Mask: ");
        print_mask64(mask64qi);
        printf("  First element of result: %d\n", first_elem);
    }
    
    // ==================== V32HImode ====================
    {
        printf("\nTesting V32HImode (32x 16-bit integers)...\n");
        
        // Initialize source vectors
        __m512i a32hi = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b32hi = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask: select from b where a is even
        __mmask32 mask32hi = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(a32hi, _mm512_set1_epi16(1)),
            _mm512_setzero_si512()
        );
        
        // Perform the masked blend
        __m512i result32hi = _mm512_mask_blend_epi16(mask32hi, a32hi, b32hi);
        
        // Use the result
        global_v32hi = result32hi;
        
        // Extract and accumulate
        int16_t first_elem = _mm512_extract_epi16(result32hi, 0);
        final_result += (uint64_t)first_elem;
        
        printf("  Mask: ");
        print_mask32(mask32hi);
        printf("  First element of result: %d\n", first_elem);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    // ==================== V32HFmode ====================
    {
        printf("\nTesting V32HFmode (32x half-precision floats)...\n");
        
        // Initialize source vectors
        __m512h a32hf = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __m512h b32hf = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask: select from b where a > 15.0
        __mmask32 mask32hf = _mm512_cmp_ph_mask(
            a32hf, _mm512_set1_ph(15.0f), _CMP_GT_OQ
        );
        
        // Perform the masked blend
        __m512h result32hf = _mm512_mask_blend_ph(mask32hf, a32hf, b32hf);
        
        // Use the result
        global_v32hf = result32hf;
        
        // Extract first element (need to store and read)
        _Float16 first_elem;
        _mm512_storeu_ph(&first_elem, result32hf);
        final_result += (uint64_t)(first_elem * 1000); // Scale to get integer
        
        printf("  Mask: ");
        print_mask32(mask32hf);
        printf("  First element of result: %f\n", (float)first_elem);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // ==================== V32BFmode ====================
    {
        printf("\nTesting V32BFmode (32x brain float)...\n");
        
        // Initialize source vectors (using __m512bh for bfloat16)
        __m512bh a32bf = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh b32bf = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create a pattern by alternating values
        uint16_t pattern_a[32], pattern_b[32];
        for (int i = 0; i < 32; i++) {
            pattern_a[i] = (i % 2 == 0) ? 0x3F80 : 0x4000; // 1.0 or 2.0
            pattern_b[i] = (i % 2 == 0) ? 0x4040 : 0x3F00; // 3.0 or 0.5
        }
        
        a32bf = _mm512_loadu_epi16(pattern_a);
        b32bf = _mm512_loadu_epi16(pattern_b);
        
        // Create mask: select from b where pattern index is even
        __mmask32 mask32bf = 0xAAAAAAAA; // Alternating bits: 10101010...
        
        // Perform the masked blend
        __m512bh result32bf = _mm512_mask_blend_ph(mask32bf, a32bf, b32bf);
        
        // Use the result
        global_v32bf = result32bf;
        
        // Extract first element
        uint16_t first_elem;
        _mm512_storeu_epi16(&first_elem, result32bf);
        final_result += (uint64_t)first_elem;
        
        printf("  Mask: ");
        print_mask32(mask32bf);
        printf("  First element of result (hex): 0x%04X\n", first_elem);
    }
#endif // __AVX512BF16__

#ifdef __AVX512F__
    // ==================== V16SImode ====================
    {
        printf("\nTesting V16SImode (16x 32-bit integers)...\n");
        
        // Initialize source vectors
        __m512i a16si = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b16si = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask: select from b where a < 8
        __mmask16 mask16si = _mm512_cmplt_epi32_mask(a16si, _mm512_set1_epi32(8));
        
        // Perform the masked blend
        __m512i result16si = _mm512_mask_blend_epi32(mask16si, a16si, b16si);
        
        // Use the result
        global_v16si = result16si;
        
        // Extract and accumulate
        int32_t first_elem = _mm512_extract_epi32(result16si, 0);
        final_result += (uint64_t)first_elem;
        
        printf("  Mask: ");
        print_mask16(mask16si);
        printf("  First element of result: %d\n", first_elem);
    }
    
    // ==================== V8DImode ====================
    {
        printf("\nTesting V8DImode (8x 64-bit integers)...\n");
        
        // Initialize source vectors
        __m512i a8di = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b8di = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask: select from b where a is odd
        __mmask8 mask8di = _mm512_cmpeq_epi64_mask(
            _mm512_and_si512(a8di, _mm512_set1_epi64(1)),
            _mm512_set1_epi64(1)
        );
        
        // Perform the masked blend
        __m512i result8di = _mm512_mask_blend_epi64(mask8di, a8di, b8di);
        
        // Use the result
        global_v8di = result8di;
        
        // Extract and accumulate
        int64_t first_elem = _mm512_extract_epi64(result8di, 0);
        final_result += (uint64_t)first_elem;
        
        printf("  Mask: ");
        print_mask8(mask8di);
        printf("  First element of result: %ld\n", first_elem);
    }
    
    // ==================== V8DFmode ====================
    {
        printf("\nTesting V8DFmode (8x double-precision floats)...\n");
        
        // Initialize source vectors
        __m512d a8df = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b8df = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask: select from b where a > 3.5
        __mmask8 mask8df = _mm512_cmp_pd_mask(a8df, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Perform the masked blend
        __m512d result8df = _mm512_mask_blend_pd(mask8df, a8df, b8df);
        
        // Use the result
        global_v8df = result8df;
        
        // Extract first element and accumulate
        double first_elem = _mm512_cvtsd_f64(result8df);
        final_result += (uint64_t)(first_elem * 1000);
        
        printf("  Mask: ");
        print_mask8(mask8df);
        printf("  First element of result: %f\n", first_elem);
    }
    
    // ==================== V16SFmode ====================
    {
        printf("\nTesting V16SFmode (16x single-precision floats)...\n");
        
        // Initialize source vectors
        __m512 a16sf = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b16sf = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask: select from b where a < 7.5
        __mmask16 mask16sf = _mm512_cmp_ps_mask(a16sf, _mm512_set1_ps(7.5f), _CMP_LT_OQ);
        
        // Perform the masked blend
        __m512 result16sf = _mm512_mask_blend_ps(mask16sf, a16sf, b16sf);
        
        // Use the result
        global_v16sf = result16sf;
        
        // Extract first element and accumulate
        float first_elem = _mm512_cvtss_f32(result16sf);
        final_result += (uint64_t)(first_elem * 1000);
        
        printf("  Mask: ");
        print_mask16(mask16sf);
        printf("  First element of result: %f\n", first_elem);
    }
#endif // __AVX512F__
    
    printf("\nFinal aggregated result: %lu\n", final_result);
    
    // Force use of all volatile globals to prevent optimization
    asm volatile("" : : "m"(global_v64qi), "m"(global_v32hi), 
                   "m"(global_v16si), "m"(global_v8di),
                   "m"(global_v16sf), "m"(global_v8df)
#ifdef __AVX512FP16__
                   , "m"(global_v32hf)
#endif
#ifdef __AVX512BF16__
                   , "m"(global_v32bf)
#endif
                   );
    
    return 0;
}
