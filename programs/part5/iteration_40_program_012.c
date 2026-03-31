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
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Extract and sum to prevent optimization
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
    
    // Mask: 0xAA = 10101010
    __mmask8 mask = 0xAA;
    
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
    
    __mmask16 mask = 0x5555; // 0101010101010101
    
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
    
    __mmask8 mask = 0x55; // 01010101
    
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
    int8_t a_arr[64], b_arr[64];
    for (int i = 0; i < 64; i++) {
        a_arr[i] = i;
        b_arr[i] = 100 + i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
    __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
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
    int16_t a_arr[32], b_arr[32];
    for (int i = 0; i < 32; i++) {
        a_arr[i] = i;
        b_arr[i] = 100 + i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
    __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // Mask: 0xAAAAAAAA = 10101010101010101010101010101010
    __mmask32 mask = 0xAAAAAAAA;
    
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
    // Initialize arrays with _Float16 values
    _Float16 a_arr[32], b_arr[32];
    for (int i = 0; i < 32; i++) {
        a_arr[i] = (_Float16)(i + 1);
        b_arr[i] = (_Float16)(100 + i);
    }
    
    __m512h a = _mm512_loadu_ph(a_arr);
    __m512h b = _mm512_loadu_ph(b_arr);
    
    // Alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
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
    // Use integer blend as fallback since specific BF16 blend might not exist
    uint16_t a_arr[32], b_arr[32];
    for (int i = 0; i < 32; i++) {
        a_arr[i] = i << 8;  // Simple bfloat16 pattern
        b_arr[i] = (100 + i) << 8;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
    __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
    
    __mmask32 mask = 0x55555555; // Different pattern
    
    // Use epi16 blend for bfloat16 (same size)
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    float sum = 0.0f;
    uint16_t* res = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        // Convert bfloat16 to float (simplified)
        uint32_t val = res[i] << 16;
        float f;
        memcpy(&f, &val, sizeof(float));
        sum += f;
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    float total = 0.0f;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F vector modes...\n");
    
    total += test_v16sf_blend();
    printf("V16SF blend completed\n");
    
    total += (float)test_v8df_blend();
    printf("V8DF blend completed\n");
    
    total += (float)test_v16si_blend();
    printf("V16SI blend completed\n");
    
    total += (float)test_v8di_blend();
    printf("V8DI blend completed\n");
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW vector modes...\n");
    
    total += (float)test_v64qi_blend();
    printf("V64QI blend completed\n");
    
    total += (float)test_v32hi_blend();
    printf("V32HI blend completed\n");
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 vector modes...\n");
    
    total += test_v32hf_blend();
    printf("V32HF blend completed\n");
    
    total += test_v32bf_blend();
    printf("V32BF blend completed\n");
#endif
    
    printf("\nTotal checksum: %f\n", total);
    printf("All AVX-512 blend tests completed successfully!\n");
    
    // Return non-zero if total is 0 (shouldn't happen)
    return (total == 0.0f) ? 1 : 0;
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 0;
#endif
}
