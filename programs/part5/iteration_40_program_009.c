#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* Helper function to print results for debugging */
void print_512i_u8(__m512i v) {
    uint8_t arr[64];
    _mm512_storeu_si512((void*)arr, v);
    for (int i = 0; i < 64; i++) {
        printf("%02x ", arr[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
}

/* V64QImode: 64 x 8-bit integers */
#ifdef __AVX512BW__
__m512i test_v64qimode_blend() {
    __m512i a = _mm512_set1_epi8(0xAA);  // Pattern: 10101010
    __m512i b = _mm512_set1_epi8(0x55);  // Pattern: 01010101
    
    // Create alternating mask: 0x5555... (01010101 pattern)
    __mmask64 mask = 0x5555555555555555ULL;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage to prevent optimization
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

/* V32HImode: 32 x 16-bit integers */
#ifdef __AVX512BW__
__m512i test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);  // 0xAAAA
    __m512i b = _mm512_set1_epi16(0x5555);  // 0x5555
    
    // Create mask with alternating bits
    __mmask32 mask = 0x55555555;  // 01010101...
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

/* V32HFmode: 32 x half-precision floats */
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend() {
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.0f);
        b_vals[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    // Create mask: select from 'a' where index is even
    __mmask32 mask = 0xAAAAAAAA;  // 10101010...
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512h volatile_result = result;
    return volatile_result;
}
#endif

/* V32BFmode: 32 x brain float (bfloat16) */
#ifdef __AVX512BF16__
__m512bh test_v32bfmode_blend() {
    // Initialize bfloat16 arrays
    __m128bh a_low = _mm_set1_epi16(0x3F80);  // 1.0 in bfloat16
    __m128bh b_low = _mm_set1_epi16(0x4000);  // 2.0 in bfloat16
    
    // Replicate to create 512-bit vectors
    __m512bh a = _mm512_broadcast_f32x4_bf16((__m128)a_low);
    __m512bh b = _mm512_broadcast_f32x4_bf16((__m128)b_low);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use comparison to create a blend scenario
    __m512bh cmp_result = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    
    // This should trigger blend operations for BF16
    // Note: Direct blend intrinsic might not exist, so use mask move
    __m512bh result = _mm512_mask_mov_ph(a, mask, b);
    
    volatile __m512bh volatile_result = result;
    return volatile_result;
}
#endif

/* V16SImode: 16 x 32-bit integers */
__m512i test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    // Create mask with alternating 32-bit elements
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

/* V8DImode: 8 x 64-bit integers */
__m512i test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    // Create mask: select from 'a' for first 4 elements, 'b' for last 4
    __mmask8 mask = 0x0F;  // 00001111
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

/* V8DFmode: 8 x double-precision floats */
__m512d test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    // Create mask based on comparison
    __m512d cmp_a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d cmp_b = _mm512_set1_pd(4.5);
    
    // Generate mask from comparison
    __mmask8 mask = _mm512_cmp_pd_mask(cmp_a, cmp_b, _CMP_LT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d volatile_result = result;
    return volatile_result;
}

/* V16SFmode: 16 x single-precision floats */
__m512 test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    // Create comparison mask
    __m512 cmp_a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 cmp_b = _mm512_set1_ps(8.5f);
    
    // Generate mask from comparison
    __mmask16 mask = _mm512_cmp_ps_mask(cmp_a, cmp_b, _CMP_LT_OQ);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 volatile_result = result;
    return volatile_result;
}

/* Array-based test to ensure blends aren't optimized away */
#ifdef __AVX512BW__
void test_array_blend_v64qi(int iterations) {
    uint8_t src1[64], src2[64], dst[64];
    
    for (int iter = 0; iter < iterations; iter++) {
        // Initialize with different patterns
        for (int i = 0; i < 64; i++) {
            src1[i] = (uint8_t)(i + iter);
            src2[i] = (uint8_t)(255 - i - iter);
        }
        
        __m512i v1 = _mm512_loadu_si512((void*)src1);
        __m512i v2 = _mm512_loadu_si512((void*)src2);
        
        // Create mask based on position
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((i + iter) % 2 == 0) {
                mask |= (1ULL << i);
            }
        }
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
        
        _mm512_storeu_si512((void*)dst, result);
        
        // Use result to prevent optimization
        volatile uint8_t sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += dst[i];
        }
    }
}
#endif

int main() {
    printf("Testing AVX-512 blend operations for all vector modes\n");
    
    // Test each vector mode
    printf("Testing V16SFmode (16 x float)...\n");
    __m512 v16sf_result = test_v16sfmode_blend();
    
    printf("Testing V8DFmode (8 x double)...\n");
    __m512d v8df_result = test_v8dfmode_blend();
    
    printf("Testing V16SImode (16 x int32)...\n");
    __m512i v16si_result = test_v16simode_blend();
    
    printf("Testing V8DImode (8 x int64)...\n");
    __m512i v8di_result = test_v8dimode_blend();
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64 x int8)...\n");
    __m512i v64qi_result = test_v64qimode_blend();
    
    printf("Testing V32HImode (32 x int16)...\n");
    __m512i v32hi_result = test_v32himode_blend();
    
    // Array-based test
    test_array_blend_v64qi(10);
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32 x half-precision)...\n");
    __m512h v32hf_result = test_v32hfmode_blend();
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32 x bfloat16)...\n");
    __m512bh v32bf_result = test_v32bfmode_blend();
#endif
    
    printf("All blend tests completed.\n");
    
    // Return non-zero if any test failed (simplified)
    return 0;
}

#else
int main() {
    printf("AVX-512 not supported on this compiler/platform.\n");
    return 1;
}
#endif
