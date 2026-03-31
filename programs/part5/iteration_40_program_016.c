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
uint64_t test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 0xFF - i;    // 255, 254, 253, ...
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits (selects even elements)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend using mask: result[i] = mask[i] ? b[i] : a[i]
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
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
uint64_t test_v32himode() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 100;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Mask: select elements where (i % 3 == 0)
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) mask |= (1ULL << i);
    }
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
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
_Float16 test_v32hfmode() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Mask: select elements where i < 16
    __mmask32 mask = 0x0000FFFF;  // Lower 16 bits set
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32x brain float) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
uint32_t test_v32bfmode() {
    // BF16 values as uint16_t
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 0x100;  // Simple pattern
        src2[i] = 0x7F00 + i; // Different pattern
    }
    
    __m512bh a = _mm512_loadu_si512(src1);
    __m512bh b = _mm512_loadu_si512(src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0x55555555;
    
    // Use integer blend since there's no direct BF16 blend intrinsic
    // The compiler should recognize this as BF16 blend
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    uint16_t out[32];
    _mm512_storeu_si512(out, (__m512i)result);
    
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
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
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Mask based on comparison
    __m512i cmp_a = _mm512_set1_epi32(5000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, cmp_a);
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
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
uint64_t test_v8dimode() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 30000LL;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Mask: select elements where i is odd
    __mmask8 mask = 0xAA;  // 0b10101010
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
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
double test_v8dfmode() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 3.5;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create mask from comparison
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(a, threshold, _CMP_LT_OQ);
    
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
        src1[i] = i * 0.25f;
        src2[i] = i * 0.75f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Complex mask pattern
    __m512 threshold = _mm512_set1_ps(2.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
    
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

int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations for coverage...\n");
    
#ifdef __AVX512BW__
    printf("V64QImode: ");
    total_checksum += test_v64qimode();
    printf("done\n");
    
    printf("V32HImode: ");
    total_checksum += test_v32himode();
    printf("done\n");
#endif
    
#ifdef __AVX512FP16__
    printf("V32HFmode: ");
    total_checksum += (uint64_t)test_v32hfmode();
    printf("done\n");
#endif
    
#ifdef __AVX512BF16__
    printf("V32BFmode: ");
    total_checksum += test_v32bfmode();
    printf("done\n");
#endif
    
#ifdef __AVX512F__
    printf("V16SImode: ");
    total_checksum += test_v16simode();
    printf("done\n");
    
    printf("V8DImode: ");
    total_checksum += test_v8dimode();
    printf("done\n");
    
    printf("V8DFmode: ");
    total_checksum += (uint64_t)test_v8dfmode();
    printf("done\n");
    
    printf("V16SFmode: ");
    total_checksum += (uint64_t)test_v16sfmode();
    printf("done\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero if any required feature was missing
    int missing = 0;
#ifndef __AVX512F__
    printf("Warning: AVX-512F not available\n");
    missing = 1;
#endif
#ifndef __AVX512BW__
    printf("Warning: AVX-512BW not available\n");
    missing = 1;
#endif
#ifndef __AVX512FP16__
    printf("Warning: AVX-512FP16 not available\n");
    missing = 1;
#endif
#ifndef __AVX512BF16__
    printf("Warning: AVX-512BF16 not available\n");
    missing = 1;
#endif
    
    return missing;
}
