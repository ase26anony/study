#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_vi;
volatile __m512d global_vd;
volatile __m512 global_vf;

#ifdef __AVX512F__
#ifdef __AVX512BW__

// Function to test 64-byte vectors of 8-bit integers (V64QI)
uint64_t test_blend_epi8() {
    __m512i a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Create mask by comparing a > 31
    __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
    
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Use result to prevent optimization
    global_vi = result;
    
    // Return sum of first 8 bytes as a simple scalar result
    uint8_t temp[64];
    _mm512_storeu_si512((void*)temp, result);
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += temp[i];
    }
    return sum;
}

// Function to test 32-word vectors of 16-bit integers (V32HI)
uint64_t test_blend_epi16() {
    __m512i a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Create mask by comparing a > 15
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Use result to prevent optimization
    global_vi = result;
    
    // Return sum of first 4 words
    uint16_t temp[32];
    _mm512_storeu_si512((void*)temp, result);
    uint64_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += temp[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
// Function to test 32-word vectors of half-precision floats (V32HF)
float test_blend_ph() {
    _Float16 a_data[32], b_data[32];
    
    // Initialize with pattern
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i * 1.5f);
        b_data[i] = (_Float16)(31 - i * 0.5f);
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    // Create mask by comparing a > 15.0
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.0f), _CMP_GT_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store and compute sum
    _Float16 temp[32];
    _mm512_storeu_ph(temp, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        sum += (float)temp[i];
    }
    return sum;
}
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
// Function to test 32-word vectors of brain float (V32BF)
float test_blend_bf16() {
    // Note: BF16 uses same intrinsics as FP16 in GCC
    __m512bh a, b;
    
    // Initialize with pattern
    uint16_t a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        // Simple pattern for BF16
        a_data[i] = i * 0x100;
        b_data[i] = (31 - i) * 0x100;
    }
    
    memcpy(&a, a_data, sizeof(a));
    memcpy(&b, b_data, sizeof(b));
    
    // Create mask by comparing first 16 elements > 15
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (a_data[i] > 15 * 0x100) {
            mask |= (1ULL << i);
        }
    }
    
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store and compute sum
    uint16_t temp[32];
    memcpy(temp, &result, sizeof(result));
    
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        // Convert BF16 pattern to float for sum
        sum += (float)(temp[i] >> 8);
    }
    return sum;
}
#endif // __AVX512BF16__

#endif // __AVX512BW__

// Function to test 16-dword vectors of 32-bit integers (V16SI)
uint64_t test_blend_epi32() {
    __m512i a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i b = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Create mask by comparing a > 7
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(7));
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Use result to prevent optimization
    global_vi = result;
    
    // Return sum of first 4 dwords
    uint32_t temp[16];
    _mm512_storeu_si512((void*)temp, result);
    uint64_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += temp[i];
    }
    return sum;
}

// Function to test 8-qword vectors of 64-bit integers (V8DI)
uint64_t test_blend_epi64() {
    __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    // Create mask by comparing a > 3
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3));
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Use result to prevent optimization
    global_vi = result;
    
    // Return sum of first 2 qwords
    uint64_t temp[8];
    _mm512_storeu_si512((void*)temp, result);
    return temp[0] + temp[1];
}

// Function to test 8-qword vectors of double-precision floats (V8DF)
double test_blend_pd() {
    __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    // Create mask by comparing a > 3.5
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Use result to prevent optimization
    global_vd = result;
    
    // Return sum of first 2 doubles
    double temp[8];
    _mm512_storeu_pd(temp, result);
    return temp[0] + temp[1];
}

// Function to test 16-dword vectors of single-precision floats (V16SF)
float test_blend_ps() {
    __m512 a = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 b = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    // Create mask by comparing a > 7.5
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Use result to prevent optimization
    global_vf = result;
    
    // Return sum of first 4 floats
    float temp[16];
    _mm512_storeu_ps(temp, result);
    return temp[0] + temp[1] + temp[2] + temp[3];
}

#endif // __AVX512F__

int main() {
    uint64_t total = 0;
    float float_total = 0.0f;
    double double_total = 0.0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512 blend operations...\n");
    
    // Test AVX-512F operations
    total += test_blend_epi32();
    total += test_blend_epi64();
    double_total += test_blend_pd();
    float_total += test_blend_ps();
    
#ifdef __AVX512BW__
    // Test AVX-512BW operations
    total += test_blend_epi8();
    total += test_blend_epi16();
    
#ifdef __AVX512FP16__
    // Test AVX-512FP16 operations
    float_total += test_blend_ph();
#endif
    
#ifdef __AVX512BF16__
    // Test AVX-512BF16 operations
    float_total += test_blend_bf16();
#endif
    
#endif // __AVX512BW__
    
    // Print results to prevent optimization
    printf("Integer total: %lu\n", total);
    printf("Float total: %f\n", float_total);
    printf("Double total: %f\n", double_total);
    
    // Use volatile globals in asm to prevent optimization
    asm volatile("" : : "m"(global_vi), "m"(global_vd), "m"(global_vf));
    
#else
    printf("AVX-512 not supported on this platform\n");
#endif
    
    return 0;
}
