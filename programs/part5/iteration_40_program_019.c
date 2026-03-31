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
    
    // Use result
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
    
    // Use result
    int32_t* res = (int32_t*)&result;
    int64_t sum = 0;
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
    
    // Use result
    int64_t* res = (int64_t*)&result;
    int64_t sum = 0;
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
    int8_t a_data[64], b_data[64];
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = i + 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Use result
    int8_t* res = (int8_t*)&result;
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
int64_t test_v32hi_blend() {
    // Initialize with pattern
    int16_t a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = i;
        b_data[i] = i + 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    // Alternating mask: 0xAAAAAAAA
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Use result
    int16_t* res = (int16_t*)&result;
    int64_t sum = 0;
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
    // Initialize half-precision arrays
    _Float16 a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i + 1);
        b_data[i] = (_Float16)((i + 1) * 100.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    // Alternating mask: 0xAAAAAAAA
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Use result
    _Float16* res = (_Float16*)&result;
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

/* V32BFmode: 32 brain floats */
__attribute__((noinline))
float test_v32bf_blend() {
    // Initialize bfloat16 arrays
    __bfloat16 a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = bfloat16_from_float((float)(i + 1));
        b_data[i] = bfloat16_from_float((float)((i + 1) * 100.0f));
    }
    
    __m512bh a = _mm512_loadu_bf16(a_data);
    __m512bh b = _mm512_loadu_bf16(b_data);
    
    // Alternating mask: 0xAAAAAAAA
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use blend intrinsic for bfloat16
    // Note: _mm512_mask_blend_epi16 works on the underlying 16-bit data
    __m512i a_int = _mm512_castps_si512(_mm512_castbf16_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_castbf16_ps(b));
    __m512i result_int = _mm512_mask_blend_epi16(mask, a_int, b_int);
    
    // Convert back
    __m512bh result = _mm512_castsi512_bf16(result_int);
    
    // Use result
    __bfloat16* res = (__bfloat16*)&result;
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(res[i]);
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    float total = 0.0f;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend modes...\n");
    
    total += test_v16sf_blend();
    printf("  V16SFmode blend completed\n");
    
    total += (float)test_v8df_blend();
    printf("  V8DFmode blend completed\n");
    
    total += (float)test_v16si_blend();
    printf("  V16SImode blend completed\n");
    
    total += (float)test_v8di_blend();
    printf("  V8DImode blend completed\n");
    
#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend modes...\n");
    
    total += (float)test_v64qi_blend();
    printf("  V64QImode blend completed\n");
    
    total += (float)test_v32hi_blend();
    printf("  V32HImode blend completed\n");
#endif
    
#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend modes...\n");
    
    total += test_v32hf_blend();
    printf("  V32HFmode blend completed\n");
    
    total += test_v32bf_blend();
    printf("  V32BFmode blend completed\n");
#endif
    
    printf("Total checksum: %f\n", total);
    printf("All AVX-512 blend tests completed successfully!\n");
    
    // Return non-zero if total is 0 (shouldn't happen)
    return (total == 0.0f) ? 1 : 0;
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 0;
#endif
}
