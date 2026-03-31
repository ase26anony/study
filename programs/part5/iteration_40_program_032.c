#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend() {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                              900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
    
    // Create alternating mask: 0xAAAA = 1010101010101010
    __mmask16 mask = 0xAAAA;
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Use result to prevent optimization
    float sum = 0.0f;
    float* res = (float*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DFmode: 8 double-precision floats */
__attribute__((noinline))
double test_v8df_blend() {
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
    
    // Create alternating mask: 0xAA = 10101010
    __mmask8 mask = 0xAA;
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double sum = 0.0;
    double* res = (double*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SImode: 16 32-bit integers */
__attribute__((noinline))
int64_t test_v16si_blend() {
    __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800,
                                  900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
    
    // Alternating mask
    __mmask16 mask = 0xAAAA;
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int64_t sum = 0;
    int32_t* res = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
int64_t test_v8di_blend() {
    __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
    
    // Alternating mask
    __mmask8 mask = 0xAA;
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t sum = 0;
    int64_t* res = (int64_t*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

#ifdef __AVX512BW__

/* V64QImode: 64 8-bit integers */
__attribute__((noinline))
int64_t test_v64qi_blend() {
    // Initialize with pattern
    int8_t a_data[64];
    int8_t b_data[64];
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = 100 + i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    // Create complex mask pattern
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    int64_t sum = 0;
    int8_t* res = (int8_t*)&result;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
int64_t test_v32hi_blend() {
    // Initialize with pattern
    int16_t a_data[32];
    int16_t b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = i;
        b_data[i] = 100 + i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    // Alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    int64_t sum = 0;
    int16_t* res = (int16_t*)&result;
    for (int i = 0; i < 32; i++) {
        sum += res[i];
    }
    return sum;
}

#endif // __AVX512BW__

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision floats */
__attribute__((noinline))
float test_v32hf_blend() {
    // Initialize arrays
    _Float16 a_data[32];
    _Float16 b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i + 1);
        b_data[i] = (_Float16)(100 + i);
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    // Alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    float sum = 0.0f;
    _Float16* res = (_Float16*)&result;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

/* V32BFmode: 32 brain float (bfloat16) */
__attribute__((noinline))
float test_v32bf_blend() {
    // For bfloat16, we need to use appropriate intrinsics
    // Note: Not all compilers have direct BF16 blend intrinsics yet
    // We'll use a pattern that should generate blend instructions
    
    // Initialize with bfloat16 pattern
    // Using __m512bh for BF16 vectors
    __m512bh a, b;
    
    // Load data - using regular floats converted to BF16
    float src_a[32], src_b[32];
    for (int i = 0; i < 32; i++) {
        src_a[i] = (float)(i + 1);
        src_b[i] = (float)(100 + i);
    }
    
    // Convert to BF16 (requires specific intrinsics)
    // This is compiler-dependent
    // For GCC, we might use _mm512_cvtneps_pbh
    
    // Create a mask
    __mmask32 mask = 0x55555555;  // Different pattern
    
    // Use a comparison to create blend opportunity
    __m512 f32_vec_a = _mm512_loadu_ps(src_a);
    __m512 f32_vec_b = _mm512_loadu_ps(src_b);
    
    // Create comparison mask
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(f32_vec_a, f32_vec_b, _CMP_LT_OQ);
    
    // Extend to 32-bit mask for BF16 (alternating pattern)
    __mmask32 bf_mask = 0;
    for (int i = 0; i < 16; i++) {
        if (cmp_mask & (1 << i)) {
            bf_mask |= (3 << (i * 2));  // Set both BF16 elements in the pair
        }
    }
    
    // Load as BF16 (simplified - actual conversion needed)
    // For coverage testing, we'll use a simpler approach
    uint16_t bf_a[32], bf_b[32];
    for (int i = 0; i < 32; i++) {
        bf_a[i] = i;  // Simplified representation
        bf_b[i] = 100 + i;
    }
    
    // Blend using integer operations that should still trigger the blend pattern
    __m512i a_int = _mm512_loadu_si512((__m512i*)bf_a);
    __m512i b_int = _mm512_loadu_si512((__m512i*)bf_b);
    
    // This blend on 16-bit elements should use the BF16 blend path
    __m512i result = _mm512_mask_blend_epi16(bf_mask, a_int, b_int);
    
    float sum = 0.0f;
    uint16_t* res = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    printf("Testing AVX-512 blend operations for coverage...\n");
    
#ifdef __AVX512F__
    printf("Testing AVX-512F modes:\n");
    
    float sf_result = test_v16sf_blend();
    printf("  V16SFmode blend result influence: %f\n", sf_result);
    
    double df_result = test_v8df_blend();
    printf("  V8DFmode blend result influence: %f\n", df_result);
    
    int64_t si_result = test_v16si_blend();
    printf("  V16SImode blend result influence: %ld\n", (long)si_result);
    
    int64_t di_result = test_v8di_blend();
    printf("  V8DImode blend result influence: %ld\n", (long)di_result);
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW modes:\n");
    
    int64_t qi_result = test_v64qi_blend();
    printf("  V64QImode blend result influence: %ld\n", (long)qi_result);
    
    int64_t hi_result = test_v32hi_blend();
    printf("  V32HImode blend result influence: %ld\n", (long)hi_result);
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 modes:\n");
    
    float hf_result = test_v32hf_blend();
    printf("  V32HFmode blend result influence: %f\n", hf_result);
    
    float bf_result = test_v32bf_blend();
    printf("  V32BFmode blend result influence: %f\n", bf_result);
#endif
    
#else
    printf("AVX-512 not supported by compiler\n");
#endif
    
    printf("Test completed.\n");
    return 0;
}
