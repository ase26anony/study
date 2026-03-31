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

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__attribute__((noinline))
uint64_t test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64 bits
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum to prevent optimization
    uint8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V32HImode: 32 x 16-bit integers
__attribute__((noinline))
uint64_t test_v32himode() {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 0xFFFF - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Alternating mask for 32 elements
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__attribute__((noinline))
float test_v32hfmode() {
    // Use _Float16 if available, otherwise use __m512h directly
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask using comparison
    __m512h zero = _mm512_setzero_ph();
    __mmask32 mask = _mm512_cmp_ph_mask(v1, zero, _CMP_GT_OQ);
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)out[i];
    }
    return sum;
}

// V32BFmode: 32 x brain float (bfloat16)
__attribute__((noinline))
float test_v32bfmode() {
    // Use __m512bh for bfloat16
    __m512bh v1, v2;
    
    // Initialize with patterns
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 patterns (just set exponent part)
        src1[i] = (i + 1) << 8;  // Varying exponents
        src2[i] = (32 - i) << 8; // Reverse pattern
    }
    
    v1 = _mm512_loadu_si512((__m512i*)src1);
    v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: select even elements from v1, odd from v2
    __mmask32 mask = 0x55555555; // 01010101...
    
    // Use integer blend since there's no direct BF16 blend intrinsic
    // This should still trigger the blend logic for V32BFmode
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        // Convert bfloat16 pattern to approximate float
        uint32_t temp = out[i] << 16;
        sum += *(float*)&temp;
    }
    return sum;
}
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__attribute__((noinline))
uint64_t test_v16simode() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask using comparison
    __m512i zero = _mm512_setzero_si512();
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, zero, _MM_CMPINT_GT);
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)out[i];
    }
    return checksum;
}

// V8DImode: 8 x 64-bit integers
__attribute__((noinline))
uint64_t test_v8dimode() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 1LL << (i * 4);
        src2[i] = -(1LL << (i * 4));
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: select first 4 from v1, last 4 from v2
    __mmask8 mask = 0xF0; // 11110000
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)out[i];
    }
    return checksum;
}

// V8DFmode: 8 x double-precision floats
__attribute__((noinline))
double test_v8dfmode() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d zero = _mm512_setzero_pd();
    __mmask8 mask = _mm512_cmp_pd_mask(v1, zero, _CMP_GT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}

// V16SFmode: 16 x single-precision floats
__attribute__((noinline))
float test_v16sfmode() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f - i * 0.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 zero = _mm512_setzero_ps();
    __mmask16 mask = _mm512_cmp_ps_mask(v1, zero, _CMP_GT_OQ);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif // __AVX512F__

int main() {
    uint64_t total_checksum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    total_checksum += test_v64qimode();
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    total_checksum += test_v32himode();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    float_sum += test_v32hfmode();
    
    printf("Testing V32BFmode (32xbfloat16)...\n");
    float_sum += test_v32bfmode();
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    total_checksum += test_v16simode();
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    total_checksum += test_v8dimode();
    
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    double_sum += test_v8dfmode();
    
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    float_sum += test_v16sfmode();
#endif
    
    // Use results to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    
    // Return non-zero if any test produced zero (unlikely with our data)
    return (total_checksum == 0 && float_sum == 0.0f && double_sum == 0.0) ? 1 : 0;
}
