#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
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
    int result_sum = 0;
    
#ifdef __AVX512BW__
    // ==================== V64QImode ====================
    {
        printf("Testing V64QImode (64x 8-bit integers)...\n");
        
        // Initialize source vectors with distinct patterns
        int8_t a_data[64];
        int8_t b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;          // 0, 1, 2, ..., 63
            b_data[i] = 64 - i;     // 64, 63, 62, ..., 1
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create a non-trivial mask by comparing elements
        // Use a pattern: select from 'a' where (i % 3 == 0), otherwise from 'b'
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((i % 3) == 0) {
                mask |= (1ULL << i);
            }
        }
        
        // Perform the blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use the result to prevent optimization
        global_v64qi = result;
        
        // Extract and accumulate first element
        int8_t first_elem = _mm512_extract_epi8(result, 0);
        result_sum += first_elem;
        
        printf("  Mask bits (first 16): ");
        print_mask64(mask & 0xFFFF);
        printf("  First element: %d\n", first_elem);
    }
    
    // ==================== V32HImode ====================
    {
        printf("Testing V32HImode (32x 16-bit integers)...\n");
        
        // Initialize source vectors
        int16_t a_data[32];
        int16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = i * 100;        // 0, 100, 200, ...
            b_data[i] = i * 50 + 10;    // 10, 60, 110, ...
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask using comparison intrinsic
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, _mm512_set1_epi16(0));
        
        // Modify mask to be non-trivial (not all zeros)
        mask = mask ^ 0xAAAAAAAA;  // XOR with pattern 1010...
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_v32hi = result;
        
        // Extract and accumulate
        int16_t first_elem = _mm512_extract_epi16(result, 0);
        result_sum += first_elem;
        
        printf("  Mask: ");
        print_mask32(mask);
        printf("  First element: %d\n", first_elem);
    }
#endif  // __AVX512BW__

#ifdef __AVX512FP16__
    // ==================== V32HFmode ====================
    {
        printf("Testing V32HFmode (32x half-precision floats)...\n");
        
        // Initialize source vectors
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i * 1.5f);
            b_data[i] = (_Float16)(i * 0.75f + 0.5f);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask using comparison
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(2.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32hf = result;
        
        // Extract and accumulate (convert to int for summation)
        _Float16 first_elem = _mm512_extract_ph(result, 0);
        result_sum += (int)first_elem;
        
        printf("  Mask (first 8 bits): ");
        print_mask32(mask & 0xFF);
        printf("  First element: %.2f\n", (float)first_elem);
    }
#endif  // __AVX512FP16__

#ifdef __AVX512BF16__
    // ==================== V32BFmode ====================
    {
        printf("Testing V32BFmode (32x brain floats)...\n");
        
        // Initialize source vectors
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern
            uint16_t val_a = (i * 137) & 0xFFFF;
            uint16_t val_b = (i * 79 + 0x4000) & 0xFFFF;
            memcpy(&a_data[i], &val_a, sizeof(__bfloat16));
            memcpy(&b_data[i], &val_b, sizeof(__bfloat16));
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // Create mask (use same intrinsic as half-precision)
        // Note: __m512bh can be cast to __m512h for comparison
        __mmask32 mask = _mm512_cmp_ph_mask((__m512h)a, 
                                           _mm512_set1_ph(0.5f), 
                                           _CMP_GT_OQ);
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32bf = result;
        
        // Extract first element
        __bfloat16 first_elem;
        _mm_store_sd((double*)&first_elem, 
                    _mm_castsi128_pd(_mm512_extracti32x4_epi32((__m512i)result, 0)));
        result_sum += first_elem;
        
        printf("  Mask (first 8 bits): ");
        print_mask32(mask & 0xFF);
        printf("  First element (as uint16): 0x%04x\n", *(uint16_t*)&first_elem);
    }
#endif  // __AVX512BF16__

#ifdef __AVX512F__
    // ==================== V16SImode ====================
    {
        printf("Testing V16SImode (16x 32-bit integers)...\n");
        
        // Initialize source vectors
        int32_t a_data[16];
        int32_t b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 1000;
            b_data[i] = i * 500 + 250;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, _mm512_set1_epi32(0));
        
        // Make mask non-trivial
        mask = mask ^ 0xAAAA;  // Pattern: 10101010...
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_v16si = result;
        
        // Extract and accumulate
        int32_t first_elem = _mm512_extract_epi32(result, 0);
        result_sum += first_elem;
        
        printf("  Mask: ");
        print_mask16(mask);
        printf("  First element: %d\n", first_elem);
    }
    
    // ==================== V8DImode ====================
    {
        printf("Testing V8DImode (8x 64-bit integers)...\n");
        
        // Initialize source vectors
        int64_t a_data[8];
        int64_t b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = i * 10000LL;
            b_data[i] = i * 5000LL + 1250LL;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(0));
        
        // Make mask non-trivial
        mask = mask ^ 0xAA;  // Pattern: 10101010
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_v8di = result;
        
        // Extract and accumulate
        int64_t first_elem = _mm512_extract_epi64(result, 0);
        result_sum += (int)first_elem;
        
        printf("  Mask: ");
        print_mask8(mask);
        printf("  First element: %ld\n", first_elem);
    }
    
    // ==================== V8DFmode ====================
    {
        printf("Testing V8DFmode (8x double-precision floats)...\n");
        
        // Initialize source vectors
        double a_data[8];
        double b_data[8];
        for (int i = 0; i < 8; i++) {
            a_data[i] = i * 1.25;
            b_data[i] = i * 2.5 + 0.5;
        }
        
        __m512d a = _mm512_loadu_pd(a_data);
        __m512d b = _mm512_loadu_pd(b_data);
        
        // Create mask using comparison
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(2.0), _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_v8df = result;
        
        // Extract and accumulate
        double first_elem = _mm512_extract_pd(result, 0);
        result_sum += (int)first_elem;
        
        printf("  Mask: ");
        print_mask8(mask);
        printf("  First element: %.2f\n", first_elem);
    }
    
    // ==================== V16SFmode ====================
    {
        printf("Testing V16SFmode (16x single-precision floats)...\n");
        
        // Initialize source vectors
        float a_data[16];
        float b_data[16];
        for (int i = 0; i < 16; i++) {
            a_data[i] = i * 0.75f;
            b_data[i] = i * 1.5f + 0.25f;
        }
        
        __m512 a = _mm512_loadu_ps(a_data);
        __m512 b = _mm512_loadu_ps(b_data);
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(1.0f), _CMP_GT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        global_v16sf = result;
        
        // Extract and accumulate
        float first_elem = _mm512_extract_ps(result, 0);
        result_sum += (int)first_elem;
        
        printf("  Mask (first 8 bits): ");
        print_mask16(mask & 0xFF);
        printf("  First element: %.2f\n", first_elem);
    }
#endif  // __AVX512F__

    printf("\nFinal accumulated result: %d\n", result_sum);
    printf("(This value is meaningless, but ensures computations aren't optimized away)\n");
    
    return 0;
}
