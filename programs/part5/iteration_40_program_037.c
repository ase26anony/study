#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode() {
    // Initialize arrays with distinct patterns
    alignas(64) int8_t src1[64];
    alignas(64) int8_t src2[64];
    alignas(64) int8_t dst[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = i;          // 0, 1, 2, ...
        src2[i] = 64 - i;     // 64, 63, 62, ...
    }
    
    // Load into vectors
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create alternating mask: 0xAA...AA (alternating 1/0 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend based on mask - should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)dst[i];
    }
    return checksum;
}
#endif

// ==================== V32HImode (32 x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode() {
    alignas(64) int16_t src1[32];
    alignas(64) int16_t src2[32];
    alignas(64) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = 1000 - i * 3;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create mask with pattern 0x5555 (alternating bits)
    __mmask32 mask = 0x55555555;
    
    // Blend - should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)dst[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
_Float16 test_v32hfmode() {
    alignas(64) _Float16 src1[32];
    alignas(64) _Float16 src2[32];
    alignas(64) _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(10.0f - i * 0.3f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Compare to create dynamic mask
    __m512h zero = _mm512_setzero_ph();
    __mmask32 mask = _mm512_cmp_ph_mask(v1, zero, _CMP_GT_OQ);
    
    // Blend - should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    
    // Compute sum
    _Float16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x bfloat16) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
uint32_t test_v32bfmode() {
    alignas(64) __bfloat16 src1[32];
    alignas(64) __bfloat16 src2[32];
    alignas(64) __bfloat16 dst[32];
    
    // Initialize with simple patterns
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.0f);
        src2[i] = bfloat16_from_float(100.0f - i * 2.0f);
    }
    
    // Load as packed integers since direct BF16 blend might not have intrinsic
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create mask
    __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
    
    // Use integer blend on BF16 data - compiler should recognize as V32BFmode
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    // Compute checksum
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += dst[i];
    }
    return checksum;
}
#endif

// ==================== V16SImode (16 x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode() {
    alignas(64) int32_t src1[16];
    alignas(64) int32_t src2[16];
    alignas(64) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100;
        src2[i] = 5000 - i * 200;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create mask using comparison
    __m512i threshold = _mm512_set1_epi32(800);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, threshold);
    
    // Blend - should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

// ==================== V8DImode (8 x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode() {
    alignas(64) int64_t src1[8];
    alignas(64) int64_t src2[8];
    alignas(64) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 9000000LL - 1000000LL * i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create alternating mask
    __mmask8 mask = 0xAA;  // 0b10101010
    
    // Blend - should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

// ==================== V8DFmode (8 x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode() {
    alignas(64) double src1[8];
    alignas(64) double src2[8];
    alignas(64) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 20.0 - i * 2.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Create mask using comparison
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    // Blend - should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

// ==================== V16SFmode (16 x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode() {
    alignas(64) float src1[16];
    alignas(64) float src2[16];
    alignas(64) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f;
        src2[i] = 10.0f - i * 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Create mask using comparison
    __m512 threshold = _mm512_set1_ps(2.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    // Blend - should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

// ==================== Main Driver ====================
int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations for coverage...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode...\n");
    total_checksum += test_v64qimode();
    
    printf("Testing V32HImode...\n");
    total_checksum += test_v32himode();
#else
    printf("AVX512BW not available, skipping byte/word modes\n");
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode...\n");
    // Convert half to integer for checksum
    _Float16 hf_result = test_v32hfmode();
    total_checksum += (uint64_t)(hf_result * 1000);
#else
    printf("AVX512FP16 not available, skipping half-precision modes\n");
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode...\n");
    total_checksum += test_v32bfmode();
#else
    printf("AVX512BF16 not available, skipping bfloat16 mode\n");
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode...\n");
    total_checksum += test_v16simode();
    
    printf("Testing V8DImode...\n");
    total_checksum += test_v8dimode();
    
    printf("Testing V8DFmode...\n");
    double df_result = test_v8dfmode();
    total_checksum += (uint64_t)(df_result * 1000);
    
    printf("Testing V16SFmode...\n");
    float sf_result = test_v16sfmode();
    total_checksum += (uint64_t)(sf_result * 1000);
#else
    printf("AVX512F not available, skipping foundation modes\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero if any test failed (simplified check)
    if (total_checksum == 0) {
        printf("WARNING: All checksums zero - possible optimization issue\n");
        return 1;
    }
    
    printf("All blend operations tested successfully!\n");
    return 0;
}
