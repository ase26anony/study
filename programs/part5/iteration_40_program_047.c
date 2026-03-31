#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 2 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    alignas(64) int8_t src1[64];
    alignas(64) int8_t src2[64];
    alignas(64) int8_t dst[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = i;          // 0, 1, 2, ...
        src2[i] = 100 + i;    // 100, 101, 102, ...
    }
    
    // Load into vectors
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA (even elements from v1, odd from v2)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Perform blend - should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)dst[i];
    }
    
    // Verify expected pattern
    for (int i = 0; i < 64; i++) {
        int8_t expected = (mask & (1ULL << i)) ? src2[i] : src1[i];
        assert(dst[i] == expected);
    }
    
    return checksum;
}
#endif

// ==================== V32HImode (32 x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    alignas(64) int16_t src1[32];
    alignas(64) int16_t src2[32];
    alignas(64) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i;
        src2[i] = 1000 + i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create checkerboard mask: 0x55555555 (select src2 for odd indices)
    __mmask32 mask = 0x55555555;
    
    // Should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)dst[i];
    }
    
    // Verify
    for (int i = 0; i < 32; i++) {
        int16_t expected = (mask & (1U << i)) ? src2[i] : src1[i];
        assert(dst[i] == expected);
    }
    
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
float test_v32hfmode_blend() {
    alignas(64) _Float16 src1[32];
    alignas(64) _Float16 src2[32];
    alignas(64) _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 100.0f + i;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Create mask: select first half from src1, second half from src2
    __mmask32 mask = 0x0000FFFF;
    
    // Should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    // Verify pattern
    for (int i = 0; i < 32; i++) {
        _Float16 expected = (mask & (1U << i)) ? src2[i] : src1[i];
        // Use approximate comparison for floating point
        assert(fabs(dst[i] - expected) < 0.001f);
    }
    
    return sum;
}
#endif

// ==================== V32BFmode (32 x bfloat16) ====================
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__  // BF16 often requires FP16 support
__attribute__((noinline))
float test_v32bfmode_blend() {
    alignas(64) __bfloat16 src1[32];
    alignas(64) __bfloat16 src2[32];
    alignas(64) __bfloat16 dst[32];
    
    // Initialize with simple values
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.0f);
        src2[i] = bfloat16_from_float(50.0f + i);
    }
    
    // Load as packed integers since direct BF16 blend intrinsics might not exist
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Create mask: alternating pattern
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use integer blend - compiler should recognize BF16 pattern
    // and generate appropriate vblendmps for BF16 elements
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(dst[i]);
    }
    
    return sum;
}
#endif
#endif

// ==================== V16SImode (16 x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    alignas(64) int32_t src1[16];
    alignas(64) int32_t src2[16];
    alignas(64) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i;
        src2[i] = 10000 + i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Mask: select elements where i % 3 == 0 from src2
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (i % 3 == 0) mask |= (1U << i);
    }
    
    // Should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)dst[i];
    }
    
    // Verify
    for (int i = 0; i < 16; i++) {
        int32_t expected = (mask & (1U << i)) ? src2[i] : src1[i];
        assert(dst[i] == expected);
    }
    
    return checksum;
}
#endif

// ==================== V8DImode (8 x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    alignas(64) int64_t src1[8];
    alignas(64) int64_t src2[8];
    alignas(64) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = 1000000LL + i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    // Mask: select first 4 elements from src2
    __mmask8 mask = 0x0F;  // 0b00001111
    
    // Should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512(dst, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += dst[i];
    }
    
    // Verify
    for (int i = 0; i < 8; i++) {
        int64_t expected = (mask & (1U << i)) ? src2[i] : src1[i];
        assert(dst[i] == expected);
    }
    
    return checksum;
}
#endif

// ==================== V8DFmode (8 x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode_blend() {
    alignas(64) double src1[8];
    alignas(64) double src2[8];
    alignas(64) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 0.25;
        src2[i] = 10.0 + i;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Create comparison mask: select where src1 > 1.0
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(1.0), _CMP_GT_OQ);
    
    // Should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    // Verify logic
    for (int i = 0; i < 8; i++) {
        double expected = (src1[i] > 1.0) ? src2[i] : src1[i];
        assert(fabs(dst[i] - expected) < 0.0001);
    }
    
    return sum;
}
#endif

// ==================== V16SFmode (16 x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode_blend() {
    alignas(64) float src1[16];
    alignas(64) float src2[16];
    alignas(64) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.1f;
        src2[i] = 5.0f + i;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Create mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(0.5f), _CMP_LT_OQ);
    
    // Should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    // Verify
    for (int i = 0; i < 16; i++) {
        float expected = (src1[i] < 0.5f) ? src2[i] : src1[i];
        assert(fabs(dst[i] - expected) < 0.0001f);
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
    printf("Testing V64QImode (64x8-bit)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit)...\n");
    total_checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf)...\n");
    float_sum += test_v32hfmode_blend();
#endif
    
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
    printf("Testing V32BFmode (32xbfloat16)...\n");
    float_sum += test_v32bfmode_blend();
#endif
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble)...\n");
    double_sum += test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xfloat)...\n");
    float_sum += test_v16sfmode_blend();
#endif
    
    printf("All tests completed successfully!\n");
    printf("Integer checksum: %lu\n", total_checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    
    // Return non-zero if any required feature was missing
    #if !defined(__AVX512F__)
    printf("Warning: AVX-512F not enabled\n");
    return 1;
    #endif
    
    return 0;
}
