#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
void test_blend_epi32(int32_t* result) {
    int32_t src1[16] __attribute__((aligned(64)));
    int32_t src2[16] __attribute__((aligned(64)));
    
    // Fill with non-uniform values
    for (int i = 0; i < 16; i++) {
        src1[i] = rand() % 100;
        src2[i] = rand() % 100 + 100;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create non-constant mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(50);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend operation
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
}

// Function to test 64-bit integer blend (E_V8DImode)
void test_blend_epi64(int64_t* result) {
    int64_t src1[8] __attribute__((aligned(64)));
    int64_t src2[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = rand() % 100;
        src2[i] = rand() % 100 + 100;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi64(50);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
}

// Function to test single-precision float blend (E_V16SFmode)
void test_blend_ps(float* result) {
    float src1[16] __attribute__((aligned(64)));
    float src2[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(rand() % 100) / 10.0f;
        src2[i] = (float)(rand() % 100 + 100) / 10.0f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Create mask using floating-point comparison
    __m512 cmp_val = _mm512_set1_ps(5.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(result, blended);
}

// Function to test double-precision float blend (E_V8DFmode)
void test_blend_pd(double* result) {
    double src1[8] __attribute__((aligned(64)));
    double src2[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(rand() % 100) / 10.0;
        src2[i] = (double)(rand() % 100 + 100) / 10.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(result, blended);
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
void test_blend_epi16(int16_t* result) {
    int16_t src1[32] __attribute__((aligned(64)));
    int16_t src2[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = rand() % 100;
        src2[i] = rand() % 100 + 100;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi16(50);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
}

// Function to test 8-bit integer blend (E_V64QImode)
void test_blend_epi8(int8_t* result) {
    int8_t src1[64] __attribute__((aligned(64)));
    int8_t src2[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        src1[i] = rand() % 100;
        src2[i] = rand() % 100 + 100;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi8(50);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision float blend (E_V32HFmode)
void test_blend_ph(_Float16* result) {
    _Float16 src1[32] __attribute__((aligned(64)));
    _Float16 src2[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(rand() % 100) / 10.0f;
        src2[i] = (_Float16)(rand() % 100 + 100) / 10.0f;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    __m512h cmp_val = _mm512_set1_ph(5.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(result, blended);
}

// Function to test bfloat16 blend (E_V32BFmode)
void test_blend_bf16(__bf16* result) {
    __bf16 src1[32] __attribute__((aligned(64)));
    __bf16 src2[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bf16)(rand() % 100) / 10.0f;
        src2[i] = (__bf16)(rand() % 100 + 100) / 10.0f;
    }
    
    __m512bh v1 = _mm512_load_si512((__m512i*)src1);
    __m512bh v2 = _mm512_load_si512((__m512i*)src2);
    
    __m512bh cmp_val = _mm512_set1_epi16(0x4200); // ~5.0f in bfloat16
    __mmask32 mask = _mm512_cmp_epi16_mask((__m512i)v1, (__m512i)cmp_val, _MM_CMPINT_GT);
    
    __m512bh blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
}
#endif

int main() {
    srand(42); // Seed for reproducible results
    
    // Arrays to store results
    int32_t result_i32[16] __attribute__((aligned(64)));
    int64_t result_i64[8] __attribute__((aligned(64)));
    float result_f32[16] __attribute__((aligned(64)));
    double result_f64[8] __attribute__((aligned(64)));
    
    // Test all blend operations
    test_blend_epi32(result_i32);
    test_blend_epi64(result_i64);
    test_blend_ps(result_f32);
    test_blend_pd(result_f64);
    
    // Compute checksums to prevent dead code elimination
    int64_t checksum = 0;
    for (int i = 0; i < 16; i++) checksum += result_i32[i];
    for (int i = 0; i < 8; i++) checksum += result_i64[i];
    for (int i = 0; i < 16; i++) checksum += (int64_t)result_f32[i];
    for (int i = 0; i < 8; i++) checksum += (int64_t)result_f64[i];
    
#ifdef __AVX512BW__
    int16_t result_i16[32] __attribute__((aligned(64)));
    int8_t result_i8[64] __attribute__((aligned(64)));
    
    test_blend_epi16(result_i16);
    test_blend_epi8(result_i8);
    
    for (int i = 0; i < 32; i++) checksum += result_i16[i];
    for (int i = 0; i < 64; i++) checksum += result_i8[i];
#endif

#ifdef __AVX512FP16__
    _Float16 result_f16[32] __attribute__((aligned(64)));
    __bf16 result_bf16[32] __attribute__((aligned(64)));
    
    test_blend_ph(result_f16);
    test_blend_bf16(result_bf16);
    
    for (int i = 0; i < 32; i++) checksum += (int64_t)result_f16[i];
    for (int i = 0; i < 32; i++) checksum += (int64_t)result_bf16[i];
#endif
    
    printf("Final checksum: %ld\n", checksum);
    printf("All blend operations completed successfully!\n");
    
    return 0;
}
