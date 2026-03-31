```cpp
// avx512_blend_coverage.c
// Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512vl -mavx512fp16 -mavx512bf16 -ftree-vectorize -fopt-info-vec avx512_blend_coverage.c -o avx512_blend_coverage

#include <immintrin.h>
#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ==================== 64-byte integer vectors (E_V64QImode) ====================
void blend_64qi(uint8_t* a, uint8_t* b, uint8_t* result) {
    __m512i vec_a = _mm512_loadu_si512((__m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: mask[i] = (a[i] > 128) ? 1 : 0
    __mmask64 mask = _mm512_cmpgt_epu8_mask(vec_a, _mm512_set1_epi8(128));
    
    // Blend using variable mask
    __m512i blended = _mm512_mask_blend_epi8(mask, vec_b, vec_a);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// ==================== 32-word integer vectors (E_V32HImode) ====================
void blend_32hi(uint16_t* a, uint16_t* b, uint16_t* result) {
    __m512i vec_a = _mm512_loadu_si512((__m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: mask[i] = (a[i] & 1) ? 1 : 0 (odd/even)
    __mmask32 mask = _mm512_test_epi16_mask(vec_a, _mm512_set1_epi16(1));
    
    // Blend using variable mask
    __m512i blended = _mm512_mask_blend_epi16(mask, vec_b, vec_a);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// ==================== 32 half-precision float vectors (E_V32HFmode) ====================
#ifdef __AVX512FP16__
void blend_32hf(_Float16* a, _Float16* b, _Float16* result) {
    __m512h vec_a = _mm512_loadu_ph(a);
    __m512h vec_b = _mm512_loadu_ph(b);
    
    // Generate dynamic mask: mask[i] = (a[i] > 0.5f) ? 1 : 0
    __mmask32 mask = _mm512_cmp_ph_mask(vec_a, _mm512_set1_ph(0.5f), _CMP_GT_OQ);
    
    // Blend using variable mask
    __m512h blended = _mm512_mask_blend_ph(mask, vec_b, vec_a);
    
    _mm512_storeu_ph(result, blended);
}
#endif

// ==================== 32 brain-float vectors (E_V32BFmode) ====================
#ifdef __AVX512BF16__
void blend_32bf(__bfloat16* a, __bfloat16* b, __bfloat16* result) {
    // Load as 32-bit floats, convert to bfloat16
    float temp_a[32], temp_b[32];
    for (int i = 0; i < 32; i++) {
        temp_a[i] = (float)a[i];
        temp_b[i] = (float)b[i];
    }
    
    __m512 vec_a = _mm512_loadu_ps(temp_a);
    __m512 vec_b = _mm512_loadu_ps(temp_b);
    
    // Generate dynamic mask: mask[i] = (a[i] > 0.0f) ? 1 : 0
    __mmask16 mask = _mm512_cmp_ps_mask(vec_a, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
    
    // Blend using variable mask (operating on 32-bit floats)
    __m512 blended = _mm512_mask_blend_ps(mask, vec_b, vec_a);
    
    // Convert back to bfloat16
    __m512bh blended_bf = _mm512_cvtneps_pbh(blended);
    _mm512_storeu_si512((__m512i*)result, (__m512i)blended_bf);
}
#endif

// ==================== 16 single-precision float vectors (E_V16SFmode) ====================
void blend_16sf(float* a, float* b, float* result) {
    __m512 vec_a = _mm512_loadu_ps(a);
    __m512 vec_b = _mm512_loadu_ps(b);
    
    // Generate dynamic mask: mask[i] = (a[i] > b[i]) ? 1 : 0
    __mmask16 mask = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_GT_OQ);
    
    // Blend using variable mask
    __m512 blended = _mm512_mask_blend_ps(mask, vec_b, vec_a);
    
    _mm512_storeu_ps(result, blended);
}

// ==================== 8 double-precision float vectors (E_V8DFmode) ====================
void blend_8df(double* a, double* b, double* result) {
    __m512d vec_a = _mm512_loadu_pd(a);
    __m512d vec_b = _mm512_loadu_pd(b);
    
    // Generate dynamic mask: mask[i] = (a[i] < b[i]) ? 1 : 0
    __mmask8 mask = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_LT_OQ);
    
    // Blend using variable mask
    __m512d blended = _mm512_mask_blend_pd(mask, vec_b, vec_a);
    
    _mm512_storeu_pd(result, blended);
}

// ==================== 16 doubleword integer vectors (E_V16SImode) ====================
void blend_16si(int32_t* a, int32_t* b, int32_t* result) {
    __m512i vec_a = _mm512_loadu_si512((__m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: mask[i] = (a[i] == b[i]) ? 1 : 0
    __mmask16 mask = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
    
    // Blend using variable mask
    __m512i blended = _mm512_mask_blend_epi32(mask, vec_b, vec_a);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// ==================== 8 quadword integer vectors (E_V8DImode) ====================
void blend_8di(int64_t* a, int64_t* b, int64_t* result) {
    __m512i vec_a = _mm512_loadu_si512((__m512i*)a);
    __m512i vec_b = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: mask[i] = (a[i] % 2 == 0) ? 1 : 0
    __m512i even_mask = _mm512_set1_epi64(1);
    __mmask8 mask = _mm512_test_epi64_mask(vec_a, even_mask);
    mask = ~mask; // Invert: 1 for even, 0 for odd
    
    // Blend using variable mask
    __m512i blended = _mm512_mask_blend_epi64(mask, vec_b, vec_a);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// ==================== Loop-based blending with changing masks ====================
void blend_loop_64qi(uint8_t* a, uint8_t* b, uint8_t* result, size_t size) {
    for (size_t i = 0; i < size; i += 64) {
        __m512i vec_a = _mm512_loadu_si512((__m512i*)(a + i));
        __m512i vec_b = _mm512_loadu_si512((__m512i*)(b + i));
        
        // Dynamic mask that changes per iteration based on loop index
        __m512i threshold = _mm512_set1_epi8((uint8_t)(i % 256));
        __mmask64 mask = _mm512_cmpgt_epi8_mask(vec_a, threshold);
        
        __m512i blended = _mm512_mask_blend_epi8(mask, vec_b, vec_a);
        _mm512_storeu_si512((__m512i*)(result + i), blended);
    }
}

// ==================== Main function with runtime checks ====================
int main() {
    // Runtime AVX-512 feature detection
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported on this CPU\n");
        return 1;
    }
    
    printf("AVX-512F supported\n");
    
    #ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    #endif
    
    #ifdef __AVX512FP16__
    printf("AVX-512FP16 supported\n");
    #endif
    
    #ifdef __AVX512BF16__
    printf("AVX-512BF16 supported\n");
    #endif
    
    // Initialize arrays with distinct patterns
    uint8_t a_8bit[64], b_8bit[64], res_8bit[64];
    uint16_t a_16bit[32], b_16bit[32], res_16bit[32];
    float a_float[16], b_float[16], res_float[16];
    double a_double[8], b_double[8], res_double[8];
    int32_t a_int32[16], b_int32[16], res_int32[16];
    int64_t a_int64[8], b_int64[8], res_int64[8];
    
    // Initialize with patterns
    for (int i = 0; i < 64; i++) {
        a_8bit[i] = i * 3;
        b_8bit[i] = i * 5;
    }
    
    for (int i = 0; i < 32; i++) {
        a_16bit[i] = i * 7;
        b_16bit[i] = i * 11;
    }
    
    for (int i = 0; i < 16; i++) {
        a_float[i] = i * 1.5f;
        b_float[i] = i * 2.5f;
        a_int32[i] = i * 13;
        b_int32[i] = i * 17;
    }
    
    for (int i = 0; i < 8; i++) {
        a_double[i] = i * 1.25;
        b_double[i] = i * 1.75;
        a_int64[i] = i * 19;
        b_int64[i] = i * 23;
    }
    
    // Perform blend operations
    blend_64qi(a_8bit, b_8bit, res_8bit);
    blend_32hi(a_16bit, b_16bit, res_16bit);
    blend_16sf(a_float, b_float, res_float);
    blend_8df(a_double, b_double, res_double);
    blend_16si(a_int32, b_int32, res_int32);
    blend_8di(a_int64, b_int64, res_int64);
    
    #ifdef __AVX512FP16__
    _Float16 a_half[32], b_half[32], res_half[32];
    for (int i = 0; i < 32; i++) {
        a_half[i] = (_Float16)(i * 0.5f);
        b_half[i] = (_Float16)(i * 0.75f);
    }
    blend_32hf(a_half, b_half, res_half);
    #endif
    
    #ifdef __AVX512BF16__
    __bfloat16 a_bf16[32], b_bf16[32], res_bf16[32];
    for (int i = 0; i < 32; i++) {
        a_bf16[i] = (__bfloat16)(i * 0.3f);
        b_bf16[i] = (__bfloat16)(i * 0.7f);
    }
    blend_32bf(a_bf16, b_bf16, res_bf16);
    #endif
    
    // Loop-based blending with changing masks
    uint8_t a_large[256], b_large[256], res_large[256];
    for (int i = 0; i < 256; i++) {
        a_large[i] = i;
        b_large[i] = 255 - i;
    }
    blend_loop_64qi(a_large, b_large, res_large, 256);
    
    // Calculate checksum to prevent dead code elimination
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum += res_8bit[i];
    for (int i = 0; i < 32; i++) checksum += res_16bit[i];
    for (int i = 0; i < 16; i++) checksum += (uint64_t)res_float[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)res_double[i];
    for (int i = 0; i < 16; i++) checksum += res_int32[i];
    for (int i = 0; i < 8; i++) checksum += res_int64[i];
    for (int i = 0; i < 256; i++) checksum += res_large[i];
    
    printf("Final checksum: %lu\n", checksum);
    printf("Blend operations completed successfully\n");
    
    return 0;
}
```

This program specifically targets the uncovered lines in `i386-expand.cc` by:

1. **Using AVX-512 blend intrinsics with variable masks** for all 8 vector modes mentioned in the uncovered lines
2. **Generating dynamic masks** using comparison operations that produce non-uniform patterns
3. **Implementing separate functions** for each vector type with appropriate load/store operations
4. **Including loop-based blending** where masks change per iteration
5. **Adding runtime CPU feature checks** and compile-time macros
6. **Preventing dead code elimination** through checksum calculations

The blend operations use:
- `_mm512_mask_blend_epi8` for V64QImode
- `_mm512_mask_blend_epi16` for V32HImode  
- `_mm512_mask_blend_ph` for V32HFmode (when available)
- `_mm512_mask_blend_ps` for V32BFmode (via float conversion) and V16SFmode
- `_mm512_mask_blend_pd` for V8DFmode
- `_mm512_mask_blend_epi32` for V16SImode
- `_mm512_mask_blend_epi64` for V8DImode

Compile with the recommended flags to trigger the specific GCC expansion patterns in the target file.
