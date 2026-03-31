#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 32 == 0) printf("\n");
    }
    printf("\n");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits (every other element)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend using vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store and compute checksum
    uint8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 100;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create checkerboard mask: 0x55555555 (select even elements from b)
    __mmask32 mask = 0x55555555;
    
    // Blend using vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    int16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
float test_v32hfmode() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Create mask selecting first half from b, second half from a
    __mmask32 mask = 0x0000FFFF;  // Lower 16 elements from b
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32x brain float) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
float test_v32bfmode() {
    // BF16 is typically converted from float for operations
    float src1_f[32], src2_f[32];
    for (int i = 0; i < 32; i++) {
        src1_f[i] = i * 1.1f;
        src2_f[i] = i * 2.2f;
    }
    
    // Load as float and convert to BF16
    __m512 a_f = _mm512_loadu_ps(src1_f);
    __m512 b_f = _mm512_loadu_ps(src2_f);
    
    __m512bh a = (__m512bh)_mm512_cvtneps_pbh(a_f);
    __m512bh b = (__m512bh)_mm512_cvtneps_pbh(b_f);
    
    // Create mask: select elements where i % 3 == 0 from b
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) mask |= (1U << i);
    }
    
    // Blend BF16 vectors - use generic blend intrinsic
    __m512bh result = _mm512_mask_blend_epi16(mask, (__m512i)a, (__m512i)b);
    
    // Convert back to float for checksum
    __m512 result_f = _mm512_cvtpbh_ps((__m256bh)_mm512_castsi512_si256((__m512i)result));
    
    float out[16];  // Only get first 16 elements back
    _mm512_storeu_ps(out, result_f);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Mask: select elements where i < 8 from b
    __mmask16 mask = 0x00FF;  // Lower 8 elements from b
    
    // Blend using vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 0x1000 * i;
        src2[i] = 0x2000 * i;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Mask: select odd elements from b
    __mmask8 mask = 0xAA;  // 0b10101010
    
    // Blend using vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.111;
        src2[i] = i * 2.222;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d threshold = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 threshold = _mm512_set1_ps(8.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
    
    // Blend using vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== Main Driver ====================
int main() {
    uint64_t total_checksum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode...\n");
    total_checksum += test_v64qimode();
    
    printf("Testing V32HImode...\n");
    total_checksum += test_v32himode();
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode...\n");
    float_sum += test_v32hfmode();
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode...\n");
    float_sum += test_v32bfmode();
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode...\n");
    total_checksum += test_v16simode();
    
    printf("Testing V8DImode...\n");
    total_checksum += test_v8dimode();
    
    printf("Testing V8DFmode...\n");
    double_sum += test_v8dfmode();
    
    printf("Testing V16SFmode...\n");
    float_sum += test_v16sfmode();
#endif
    
    // Use results to prevent optimization
    printf("Integer checksum: %lu\n", total_checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    
    // Return non-zero if any test failed (simplified check)
    int result = (total_checksum == 0 && float_sum == 0.0f && double_sum == 0.0) ? 1 : 0;
    
    // Force compiler to generate all code paths
    volatile int dummy = result;
    return dummy;
}
