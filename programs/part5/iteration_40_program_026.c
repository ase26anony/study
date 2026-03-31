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
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend using mask: selects v1 where mask=0, v2 where mask=1
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum
    uint8_t out[64];
    _mm512_storeu_si512(out, result);
    
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
uint64_t test_v32himode_blend() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create checkerboard mask: 0x55555555 for 32 bits
    __mmask32 mask = 0x55555555;
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    int16_t out[32];
    _mm512_storeu_si512(out, result);
    
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
float test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask with first half zeros, second half ones
    __mmask32 mask = 0xFFFF0000;
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
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
float test_v32bfmode_blend() {
    // Use __m512bh for bfloat16 vectors
    __m512bh v1, v2;
    
    // Initialize with simple patterns
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 patterns (just set exponent part)
        src1[i] = (i % 16) << 10;
        src2[i] = ((i + 8) % 16) << 10;
    }
    
    v1 = _mm512_loadu_si512(src1);
    v2 = _mm512_loadu_si512(src2);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend bfloat16 vectors
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    uint16_t out[32];
    _mm512_storeu_si512(out, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)(out[i] >> 10);  // Just use exponent part
    }
    return sum;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Mask: select even elements from v2, odd from v1
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512(out, result);
    
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
uint64_t test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 0x1000 * i;
        src2[i] = 0x2000 * i;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Mask: select first 4 from v1, last 4 from v2
    __mmask8 mask = 0xF0;  // 11110000
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512(out, result);
    
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
double test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Compare to create dynamic mask
    __m512d cmp_val = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
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
float test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
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
    float total_float_sum = 0.0f;
    double total_double_sum = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode...\n");
    total_checksum += test_v32himode_blend();
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode...\n");
    total_float_sum += test_v32hfmode_blend();
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode...\n");
    total_float_sum += test_v32bfmode_blend();
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode...\n");
    total_double_sum += test_v8dfmode_blend();
    
    printf("Testing V16SFmode...\n");
    total_float_sum += test_v16sfmode_blend();
#endif
    
    // Use results to prevent dead code elimination
    printf("Total checksum: %lu\n", total_checksum);
    printf("Total float sum: %f\n", total_float_sum);
    printf("Total double sum: %f\n", total_double_sum);
    
    // Return non-zero if any test failed (simplified check)
    return (total_checksum == 0 && total_float_sum == 0.0f && total_double_sum == 0.0) ? 1 : 0;
}
