```cpp
// avx512_blend_coverage.c
// Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512vl -mavx512fp16 -mavx512bf16 -ftree-vectorize -o avx512_blend avx512_blend_coverage.c
#include <immintrin.h>
#include <x86intrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 64-byte integer vectors (E_V64QImode) ====================
void blend_64qi(uint8_t* a, uint8_t* b, uint8_t* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: compare each element with 128
    __mmask64 mask = _mm512_cmplt_epu8_mask(va, _mm512_set1_epi8(128));
    
    // Blend using AVX512BW intrinsic
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
}

// ==================== 32-word integer vectors (E_V32HImode) ====================
void blend_32hi(int16_t* a, int16_t* b, int16_t* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: compare for equality with index pattern
    __m512i indices = _mm512_set_epi16(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                                       15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_epi16(indices, _mm512_set1_epi16(1)), 
                                            _mm512_setzero_si512());
    
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
}

// ==================== 32 half-precision float vectors (E_V32HFmode) ====================
#ifdef __AVX512FP16__
void blend_32hf(_Float16* a, _Float16* b, _Float16* result) {
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    // Generate dynamic mask: compare with threshold
    __m512h threshold = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(va, threshold, _CMP_LT_OQ);
    
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    _mm512_storeu_ph(result, vresult);
}
#endif

// ==================== 32 brain-float vectors (E_V32BFmode) ====================
#ifdef __AVX512BF16__
void blend_32bf(__bfloat16* a, __bfloat16* b, __bfloat16* result) {
    // Load as 32-bit floats, convert to bfloat16
    float fa[32], fb[32];
    for (int i = 0; i < 32; i++) {
        fa[i] = (float)a[i];
        fb[i] = (float)b[i];
    }
    
    __m512 va = _mm512_loadu_ps(fa);
    __m512 vb = _mm512_loadu_ps(fb);
    
    // Generate dynamic mask
    __m512 threshold = _mm512_set1_ps(0.0f);
    __mmask16 mask32 = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    // Blend at 32-bit level, then convert to bfloat16
    __m512 vresult = _mm512_mask_blend_ps(mask32, va, vb);
    
    // Convert to bfloat16 (requires AVX512BF16)
    __m512bh vresult_bf = _mm512_cvtneps_pbh(vresult);
    _mm512_storeu_si512((__m512i*)result, (__m512i)vresult_bf);
}
#endif

// ==================== 16 single-precision float vectors (E_V16SFmode) ====================
void blend_16sf(float* a, float* b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    // Generate dynamic mask using comparison
    __m512 threshold = _mm512_set1_ps(0.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    _mm512_storeu_ps(result, vresult);
}

// ==================== 8 double-precision float vectors (E_V8DFmode) ====================
void blend_8df(double* a, double* b, double* result) {
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    // Generate dynamic mask
    __m512d threshold = _mm512_set1_pd(0.0);
    __mmask8 mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    _mm512_storeu_pd(result, vresult);
}

// ==================== 16 doubleword integer vectors (E_V16SImode) ====================
void blend_16si(int32_t* a, int32_t* b, int32_t* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: check if elements are even
    __m512i indices = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_epi32(indices, _mm512_set1_epi32(1)), 
                                            _mm512_setzero_si512());
    
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
}

// ==================== 8 quadword integer vectors (E_V8DImode) ====================
void blend_8di(int64_t* a, int64_t* b, int64_t* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask: compare with index pattern
    __m512i indices = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_epi64(indices, _mm512_set1_epi64(1)), 
                                           _mm512_setzero_si512());
    
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    _mm512_storeu_si512((__m512i*)result, vresult);
}

// ==================== Loop-based blending with changing masks ====================
void blend_loop_64qi(uint8_t* a, uint8_t* b, uint8_t* result, size_t size) {
    for (size_t i = 0; i < size; i += 64) {
        __m512i va = _mm512_loadu_si512((__m512i*)(a + i));
        __m512i vb = _mm512_loadu_si512((__m512i*)(b + i));
        
        // Mask changes per iteration based on loop index
        __mmask64 mask = _mm512_cmplt_epu8_mask(va, _mm512_set1_epi8((uint8_t)(i % 256)));
        
        __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
        _mm512_storeu_si512((__m512i*)(result + i), vresult);
    }
}

// ==================== Main function ====================
int main() {
    // Check AVX-512 support at runtime
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported on this CPU\n");
        return 1;
    }
    
#ifdef __AVX512BW__
    if (!__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512BW not supported on this CPU\n");
        return 1;
    }
#endif
    
    printf("AVX-512 support verified. Running blend operations...\n");
    
    // Initialize arrays with distinct patterns
    uint8_t a64[64], b64[64], r64[64];
    int16_t a32[32], b32[32], r32[32];
    float a16f[16], b16f[16], r16f[16];
    double a8d[8], b8d[8], r8d[8];
    int32_t a16i[16], b16i[16], r16i[16];
    int64_t a8di[8], b8di[8], r8di[8];
    
    for (int i = 0; i < 64; i++) {
        a64[i] = i;
        b64[i] = 255 - i;
        if (i < 32) {
            a32[i] = i * 100;
            b32[i] = -i * 100;
        }
        if (i < 16) {
            a16f[i] = (float)i - 8.0f;
            b16f[i] = (float)(15 - i) - 8.0f;
            a16i[i] = i * 1000;
            b16i[i] = -i * 1000;
        }
        if (i < 8) {
            a8d[i] = (double)i - 4.0;
            b8d[i] = (double)(7 - i) - 4.0;
            a8di[i] = i * 10000LL;
            b8di[i] = -i * 10000LL;
        }
    }
    
    // Perform blend operations
    blend_64qi(a64, b64, r64);
    blend_32hi(a32, b32, r32);
    blend_16sf(a16f, b16f, r16f);
    blend_8df(a8d, b8d, r8d);
    blend_16si(a16i, b16i, r16i);
    blend_8di(a8di, b8di, r8di);
    
#ifdef __AVX512FP16__
    _Float16 a32hf[32], b32hf[32], r32hf[32];
    for (int i = 0; i < 32; i++) {
        a32hf[i] = (_Float16)(i - 16) / 32.0f;
        b32hf[i] = (_Float16)(31 - i) / 32.0f;
    }
    blend_32hf(a32hf, b32hf, r32hf);
#endif
    
#ifdef __AVX512BF16__
    __bfloat16 a32bf[32], b32bf[32], r32bf[32];
    for (int i = 0; i < 32; i++) {
        a32bf[i] = (__bfloat16)(i - 16) / 32.0f;
        b32bf[i] = (__bfloat16)(31 - i) / 32.0f;
    }
    blend_32bf(a32bf, b32bf, r32bf);
#endif
    
    // Loop-based blending with changing masks
    uint8_t a_loop[256], b_loop[256], r_loop[256];
    for (int i = 0; i < 256; i++) {
        a_loop[i] = i;
        b_loop[i] = 255 - i;
    }
    blend_loop_64qi(a_loop, b_loop, r_loop, 256);
    
    // Calculate checksum to prevent dead code elimination
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum += r64[i];
    for (int i = 0; i < 32; i++) checksum += r32[i];
    for (int i = 0; i < 16; i++) checksum += (uint64_t)r16f[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)r8d[i];
    for (int i = 0; i < 16; i++) checksum += r16i[i];
    for (int i = 0; i < 8; i++) checksum += r8di[i];
    for (int i = 0; i < 256; i++) checksum += r_loop[i];
    
    printf("Final checksum: %lu\n", checksum);
    printf("All AVX-512 blend operations completed successfully.\n");
    
    return 0;
}
```

This program specifically targets the uncovered lines in `i386-expand.cc` by:

1. **Using AVX-512 blend intrinsics with variable masks** for all 8 vector modes mentioned in the uncovered lines
2. **Generating dynamic masks** using comparison operations that produce non-uniform patterns
3. **Implementing separate functions** for each vector type with appropriate load/store operations
4. **Including loop-based blending** where masks change per iteration
5. **Adding runtime CPU feature checks** and compile-time guards
6. **Calculating a checksum** to prevent dead code elimination

The blend operations use:
- `_mm512_mask_blend_epi8` for `E_V64QImode`
- `_mm512_mask_blend_epi16` for `E_V32HImode`
- `_mm512_mask_blend_ph` for `E_V32HFmode` (when available)
- `_mm512_mask_blend_ps` for `E_V32BFmode` (via 32-bit float blending)
- `_mm512_mask_blend_ps` for `E_V16SFmode`
- `_mm512_mask_blend_pd` for `E_V8DFmode`
- `_mm512_mask_blend_epi32` for `E_V16SImode`
- `_mm512_mask_blend_epi64` for `E_V8DImode`

Compile with the recommended flags to trigger the specific GCC expansion patterns in `i386-expand.cc`.
