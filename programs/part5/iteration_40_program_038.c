#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to print results for debugging */
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    if (size % 16 != 0) printf("\n");
}

/* ========== V64QImode: 64 x 8-bit integers ========== */
#ifdef __AVX512BW__
__m512i test_v64qimode_blend(__m512i a, __m512i b, __mmask64 mask) {
    // Use vblendmb instruction
    return _mm512_mask_blend_epi8(mask, a, b);
}

void exercise_v64qimode() {
    printf("Testing V64QImode (64x8-bit integers)...\n");
    
    // Create test data
    uint8_t data_a[64], data_b[64];
    for (int i = 0; i < 64; i++) {
        data_a[i] = i;           // 0, 1, 2, ...
        data_b[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA for 01010101 pattern
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Perform blend
    __m512i result = test_v64qimode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    uint8_t result_data[64];
    _mm512_storeu_si512(result_data, result);
    
    // Simple verification: check pattern
    int errors = 0;
    for (int i = 0; i < 64; i++) {
        uint8_t expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V64QImode blend completed with %d errors\n", errors);
}
#endif

/* ========== V32HImode: 32 x 16-bit integers ========== */
#ifdef __AVX512BW__
__m512i test_v32himode_blend(__m512i a, __m512i b, __mmask32 mask) {
    // Use vblendmw instruction
    return _mm512_mask_blend_epi16(mask, a, b);
}

void exercise_v32himode() {
    printf("Testing V32HImode (32x16-bit integers)...\n");
    
    // Create test data
    uint16_t data_a[32], data_b[32];
    for (int i = 0; i < 32; i++) {
        data_a[i] = i * 100;
        data_b[i] = 0xFFFF - i * 100;
    }
    
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    
    // Create checkerboard mask: 0xAAAAAAAA
    __mmask32 mask = 0xAAAAAAAA;
    
    // Perform blend
    __m512i result = test_v32himode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    uint16_t result_data[32];
    _mm512_storeu_si512(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 32; i++) {
        uint16_t expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V32HImode blend completed with %d errors\n", errors);
}
#endif

/* ========== V32HFmode: 32 x half-precision floats ========== */
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend(__m512h a, __m512h b, __mmask32 mask) {
    // Use vblendmph instruction
    return _mm512_mask_blend_ph(mask, a, b);
}

void exercise_v32hfmode() {
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    
    // Create test data
    _Float16 data_a[32], data_b[32];
    for (int i = 0; i < 32; i++) {
        data_a[i] = (_Float16)(i * 1.5f);
        data_b[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h vec_a = _mm512_loadu_ph(data_a);
    __m512h vec_b = _mm512_loadu_ph(data_b);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;  // Opposite pattern from before
    
    // Perform blend
    __m512h result = test_v32hfmode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    _Float16 result_data[32];
    _mm512_storeu_ph(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 32; i++) {
        _Float16 expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        // Compare with tolerance for floating-point
        if (__builtin_fabs(result_data[i] - expected) > (_Float16)0.001f) errors++;
    }
    
    printf("  V32HFmode blend completed with %d errors\n", errors);
}
#endif

/* ========== V32BFmode: 32 x bfloat16 ========== */
#ifdef __AVX512BF16__
#ifdef __AVX512VL__
__m512bh test_v32bfmode_blend(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use appropriate blend intrinsic for bfloat16
    // Note: There's no direct _mm512_mask_blend_* for BF16 in current intrinsics,
    // but the compiler will generate vblendmps when using appropriate operations
    __m512 a_f32 = _mm512_castb512_ps(a);
    __m512 b_f32 = _mm512_castb512_ps(b);
    __m512 result_f32 = _mm512_mask_blend_ps(mask, a_f32, b_f32);
    return _mm512_castps_bh(result_f32);
}

void exercise_v32bfmode() {
    printf("Testing V32BFmode (32xbfloat16)...\n");
    
    // Create test data as bfloat16
    __m512bh data_a, data_b;
    
    // Initialize with some pattern
    float temp_a[16] = {0};
    float temp_b[16] = {0};
    for (int i = 0; i < 16; i++) {
        temp_a[i] = i * 2.0f;
        temp_b[i] = 100.0f - i * 2.0f;
    }
    
    // Convert to bfloat16 (simplified - actual conversion needed)
    data_a = _mm512_cvtneps_pbh(_mm512_loadu_ps(temp_a));
    data_b = _mm512_cvtneps_pbh(_mm512_loadu_ps(temp_b));
    
    // Create mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Perform blend
    __m512bh result = test_v32bfmode_blend(data_a, data_b, mask);
    
    printf("  V32BFmode blend completed (verification simplified)\n");
}
#endif
#endif

/* ========== V16SImode: 16 x 32-bit integers ========== */
#ifdef __AVX512F__
__m512i test_v16simode_blend(__m512i a, __m512i b, __mmask16 mask) {
    // Use vblendmd instruction
    return _mm512_mask_blend_epi32(mask, a, b);
}

void exercise_v16simode() {
    printf("Testing V16SImode (16x32-bit integers)...\n");
    
    // Create test data
    int32_t data_a[16], data_b[16];
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 1000;
        data_b[i] = -i * 1000;
    }
    
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    
    // Create mask: select even indices from a, odd from b
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // Perform blend
    __m512i result = test_v16simode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    int32_t result_data[16];
    _mm512_storeu_si512(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 16; i++) {
        int32_t expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V16SImode blend completed with %d errors\n", errors);
}
#endif

/* ========== V8DImode: 8 x 64-bit integers ========== */
#ifdef __AVX512F__
__m512i test_v8dimode_blend(__m512i a, __m512i b, __mmask8 mask) {
    // Use vblendmq instruction
    return _mm512_mask_blend_epi64(mask, a, b);
}

void exercise_v8dimode() {
    printf("Testing V8DImode (8x64-bit integers)...\n");
    
    // Create test data
    int64_t data_a[8], data_b[8];
    for (int i = 0; i < 8; i++) {
        data_a[i] = 0x100000000LL * i;
        data_b[i] = 0xFFFFFFFFFFFFFFFFLL - 0x100000000LL * i;
    }
    
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    
    // Create mask: 0xAA = 10101010
    __mmask8 mask = 0xAA;
    
    // Perform blend
    __m512i result = test_v8dimode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    int64_t result_data[8];
    _mm512_storeu_si512(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 8; i++) {
        int64_t expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V8DImode blend completed with %d errors\n", errors);
}
#endif

/* ========== V8DFmode: 8 x double-precision floats ========== */
#ifdef __AVX512F__
__m512d test_v8dfmode_blend(__m512d a, __m512d b, __mmask8 mask) {
    // Use vblendmpd instruction
    return _mm512_mask_blend_pd(mask, a, b);
}

void exercise_v8dfmode() {
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    
    // Create test data
    double data_a[8], data_b[8];
    for (int i = 0; i < 8; i++) {
        data_a[i] = i * 1.5;
        data_b[i] = 100.0 - i * 1.5;
    }
    
    __m512d vec_a = _mm512_loadu_pd(data_a);
    __m512d vec_b = _mm512_loadu_pd(data_b);
    
    // Create mask: 0x55 = 01010101
    __mmask8 mask = 0x55;
    
    // Perform blend
    __m512d result = test_v8dfmode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    double result_data[8];
    _mm512_storeu_pd(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 8; i++) {
        double expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V8DFmode blend completed with %d errors\n", errors);
}
#endif

/* ========== V16SFmode: 16 x single-precision floats ========== */
#ifdef __AVX512F__
__m512 test_v16sfmode_blend(__m512 a, __m512 b, __mmask16 mask) {
    // Use vblendmps instruction
    return _mm512_mask_blend_ps(mask, a, b);
}

void exercise_v16sfmode() {
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    
    // Create test data
    float data_a[16], data_b[16];
    for (int i = 0; i < 16; i++) {
        data_a[i] = i * 0.5f;
        data_b[i] = 50.0f - i * 0.5f;
    }
    
    __m512 vec_a = _mm512_loadu_ps(data_a);
    __m512 vec_b = _mm512_loadu_ps(data_b);
    
    // Create mask: alternating pattern
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // Perform blend
    __m512 result = test_v16sfmode_blend(vec_a, vec_b, mask);
    
    // Store and verify
    float result_data[16];
    _mm512_storeu_ps(result_data, result);
    
    int errors = 0;
    for (int i = 0; i < 16; i++) {
        float expected = ((mask >> i) & 1) ? data_b[i] : data_a[i];
        if (result_data[i] != expected) errors++;
    }
    
    printf("  V16SFmode blend completed with %d errors\n", errors);
}
#endif

/* ========== Main driver ========== */
int main() {
    printf("AVX-512 Vector Blend Coverage Test\n");
    printf("==================================\n");
    
    // Track total errors
    int total_errors = 0;
    
#ifdef __AVX512F__
    printf("\nAVX-512F modes:\n");
    exercise_v16simode();
    exercise_v8dimode();
    exercise_v8dfmode();
    exercise_v16sfmode();
#endif

#ifdef __AVX512BW__
    printf("\nAVX-512BW modes:\n");
    exercise_v64qimode();
    exercise_v32himode();
#endif

#ifdef __AVX512FP16__
    printf("\nAVX-512FP16 modes:\n");
    exercise_v32hfmode();
#endif

#ifdef __AVX512BF16__
#ifdef __AVX512VL__
    printf("\nAVX-512BF16 modes:\n");
    exercise_v32bfmode();
#endif
#endif

    printf("\n==================================\n");
    printf("All tests completed.\n");
    
    return 0;
}
