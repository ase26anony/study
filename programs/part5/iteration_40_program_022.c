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
        else if ((i + 1) % 4 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;          // 0, 1, 2, ...
        src2[i] = 255 - i;    // 255, 254, 253, ...
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits (extended to 64 bytes)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend based on mask: where mask bit=1, take from v2; where 0, take from v1
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

// ==================== V32HImode (32 x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 0xFFFF - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create checkerboard mask: 0xAAAAAAAA for 32 bits
    __mmask32 mask = 0xAAAAAAAA;
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t out[32];
    _mm512_storeu_si512(out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
_Float16 test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask where elements with even index are selected from v2
    __mmask32 mask = 0x55555555;  // Opposite pattern from above
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x bfloat16) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
uint32_t test_v32bfmode_blend() {
    // Use integer representation for bfloat16
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple patterns: 1.0f = 0x3F80, 2.0f = 0x4000 as bfloat16
        src1[i] = 0x3F80 + i;      // ~1.0 + small increment
        src2[i] = 0x4000 - i;      // ~2.0 - small decrement
    }
    
    __m512bh v1 = _mm512_loadu_si512(src1);
    __m512bh v2 = _mm512_loadu_si512(src2);
    
    // Create mask using comparison (since direct blend intrinsic may not exist)
    __m512bh ones = _mm512_set1_epi16(0x3F80);
    __mmask32 mask = _mm512_cmp_epi16_mask((__m512i)v1, (__m512i)ones, _MM_CMPINT_LT);
    
    // Blend using integer blend since bfloat16 blend might not have direct intrinsic
    __m512i vi1 = _mm512_castps_si512(_mm512_castph_ps(v1));
    __m512i vi2 = _mm512_castps_si512(_mm512_castph_ps(v2));
    __m512i result_i = _mm512_mask_blend_epi32(mask, vi1, vi2);
    
    uint32_t out[16];  // 32 bfloat16s = 16 dwords
    _mm512_storeu_si512(out, result_i);
    
    uint32_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V16SImode (16 x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create mask using comparison
    __m512i threshold = _mm512_set1_epi32(5000);
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, threshold, _MM_CMPINT_LT);
    
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

// ==================== V8DImode (8 x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 0x100000000LL * i;
        src2[i] = -0x100000000LL * i;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create mask: select from v2 where index is odd
    __mmask8 mask = 0xAA;  // 0b10101010
    
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

// ==================== V8DFmode (8 x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_LT_OQ);
    
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

// ==================== V16SFmode (16 x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f - i * 0.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_LT_OQ);
    
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

// ==================== Main driver ====================
int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations for coverage...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode...\n");
    total_checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode...\n");
    total_checksum += (uint64_t)test_v32hfmode_blend();
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode...\n");
    total_checksum += test_v32bfmode_blend();
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode...\n");
    total_checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode...\n");
    total_checksum += (uint64_t)test_v16sfmode_blend();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero if any test failed (simplified check)
    // In a real test, you would compare against expected values
    return total_checksum == 0 ? 1 : 0;
}
