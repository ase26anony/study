#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Initialize arrays with distinct patterns
#define ARRAY_SIZE 64

// For integer types
static uint8_t src1_u8[ARRAY_SIZE];
static uint8_t src2_u8[ARRAY_SIZE];
static uint8_t dst_u8[ARRAY_SIZE];

static uint16_t src1_u16[ARRAY_SIZE];
static uint16_t src2_u16[ARRAY_SIZE];
static uint16_t dst_u16[ARRAY_SIZE];

static int32_t src1_s32[ARRAY_SIZE];
static int32_t src2_s32[ARRAY_SIZE];
static int32_t dst_s32[ARRAY_SIZE];

static int64_t src1_s64[ARRAY_SIZE];
static int64_t src2_s64[ARRAY_SIZE];
static int64_t dst_s64[ARRAY_SIZE];

// For floating-point types
static float src1_f32[ARRAY_SIZE];
static float src2_f32[ARRAY_SIZE];
static float dst_f32[ARRAY_SIZE];

static double src1_f64[ARRAY_SIZE];
static double src2_f64[ARRAY_SIZE];
static double dst_f64[ARRAY_SIZE];

// For half-precision (stored as uint16_t)
static uint16_t src1_f16[ARRAY_SIZE];
static uint16_t src2_f16[ARRAY_SIZE];
static uint16_t dst_f16[ARRAY_SIZE];

// For brain-float (stored as uint16_t)
static uint16_t src1_bf16[ARRAY_SIZE];
static uint16_t src2_bf16[ARRAY_SIZE];
static uint16_t dst_bf16[ARRAY_SIZE];

// Initialize all arrays with predictable patterns
void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Integer patterns
        src1_u8[i] = i;
        src2_u8[i] = 255 - i;
        
        src1_u16[i] = i * 2;
        src2_u16[i] = 65535 - i * 2;
        
        src1_s32[i] = i * 100;
        src2_s32[i] = -i * 100;
        
        src1_s64[i] = i * 1000LL;
        src2_s64[i] = -i * 1000LL;
        
        // Floating-point patterns
        src1_f32[i] = i * 1.5f;
        src2_f32[i] = i * 2.5f;
        
        src1_f64[i] = i * 1.25;
        src2_f64[i] = i * 3.75;
        
        // Half-precision patterns (simple integer values)
        src1_f16[i] = i & 0xFFFF;
        src2_f16[i] = (i + 100) & 0xFFFF;
        
        // Brain-float patterns
        src1_bf16[i] = (i * 2) & 0xFFFF;
        src2_bf16[i] = (i * 3) & 0xFFFF;
    }
}

// Checksum functions to prevent optimization
uint64_t checksum_u8(const uint8_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

uint64_t checksum_u16(const uint16_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int64_t checksum_s32(const int32_t* arr, int size) {
    int64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int64_t checksum_s64(const int64_t* arr, int size) {
    int64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

double checksum_f32(const float* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

double checksum_f64(const double* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

// Main test function with conditional compilation
int main() {
    init_arrays();
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blend operations...\n");
    
    // Test 1: V64QImode - 64x 8-bit integers
    {
        __m512i v1, v2, result;
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;  // Alternating pattern
        
        // Load vectors
        v1 = _mm512_loadu_si512((const __m512i*)src1_u8);
        v2 = _mm512_loadu_si512((const __m512i*)src2_u8);
        
        // Blend with constant mask
        result = _mm512_mask_blend_epi8(mask, v1, v2);
        
        // Loop-based blend with varying mask
        for (int i = 0; i < 4; i++) {
            __mmask64 dynamic_mask = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
            result = _mm512_mask_blend_epi8(dynamic_mask, result, v1);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
            result = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, result, temp);
        }
        
        _mm512_storeu_si512((__m512i*)dst_u8, result);
        
        // Force materialization with inline assembly
        asm volatile ("" : "+v"(v1), "+v"(v2), "+v"(result) : : "memory");
        
        total_checksum += checksum_u8(dst_u8, ARRAY_SIZE);
    }
    
    // Test 2: V32HImode - 32x 16-bit integers
    {
        __m512i v1, v2, result;
        __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
        
        v1 = _mm512_loadu_si512((const __m512i*)src1_u16);
        v2 = _mm512_loadu_si512((const __m512i*)src2_u16);
        
        // Blend with constant mask
        result = _mm512_mask_blend_epi16(mask, v1, v2);
        
        // Loop with varying masks
        for (int i = 0; i < 3; i++) {
            __mmask32 dynamic_mask = (i % 3) ? 0xFFFFFFFF : 0x55555555;
            result = _mm512_mask_blend_epi16(dynamic_mask, result, v2);
            
            // Multi-stage processing
            __m512i temp = _mm512_slli_epi16(result, 1);
            result = _mm512_mask_blend_epi16(0x33333333, result, temp);
        }
        
        _mm512_storeu_si512((__m512i*)dst_u16, result);
        total_checksum += checksum_u16(dst_u16, ARRAY_SIZE);
    }
    
    // Test 3: V32HFmode - 32x half-precision floats
    {
#ifdef __AVX512FP16__
        __m512h v1, v2, result;
        __mmask32 mask = 0xCCCCCCCC;  // Checkerboard pattern
        
        // Load as integers and cast to half-precision
        __m512i v1i = _mm512_loadu_si512((const __m512i*)src1_f16);
        __m512i v2i = _mm512_loadu_si512((const __m512i*)src2_f16);
        v1 = _mm512_castsi512_ph(v1i);
        v2 = _mm512_castsi512_ph(v2i);
        
        // Blend operation
        result = _mm512_mask_blend_ph(mask, v1, v2);
        
        // Multi-stage pipeline
        for (int i = 0; i < 2; i++) {
            __mmask32 dynamic_mask = (i % 2) ? 0xFFFFFFFF : 0x0F0F0F0F;
            __m512h temp = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
            result = _mm512_mask_blend_ph(dynamic_mask, result, temp);
        }
        
        // Store back
        __m512i result_i = _mm512_castph_si512(result);
        _mm512_storeu_si512((__m512i*)dst_f16, result_i);
        
        total_checksum += checksum_u16(dst_f16, ARRAY_SIZE);
#endif
    }
#endif  // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blend operations...\n");
    
    // Test 4: V16SImode - 16x 32-bit integers
    {
        __m512i v1, v2, result;
        __mmask16 mask = 0xAAAA;  // Alternating pattern
        
        v1 = _mm512_loadu_si512((const __m512i*)src1_s32);
        v2 = _mm512_loadu_si512((const __m512i*)src2_s32);
        
        // Blend with constant mask
        result = _mm512_mask_blend_epi32(mask, v1, v2);
        
        // Loop-based processing
        for (int i = 0; i < 5; i++) {
            __mmask16 dynamic_mask = (i % 3) ? 0xFFFF : 0x5555;
            result = _mm512_mask_blend_epi32(dynamic_mask, result, v1);
            
            // Arithmetic + blend chain
            __m512i temp = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
            result = _mm512_mask_blend_epi32(0x3333, result, temp);
        }
        
        _mm512_storeu_si512((__m512i*)dst_s32, result);
        total_checksum += checksum_s32(dst_s32, ARRAY_SIZE);
    }
    
    // Test 5: V8DImode - 8x 64-bit integers
    {
        __m512i v1, v2, result;
        __mmask8 mask = 0xAA;  // Alternating pattern
        
        v1 = _mm512_loadu_si512((const __m512i*)src1_s64);
        v2 = _mm512_loadu_si512((const __m512i*)src2_s64);
        
        // Blend with constant mask
        result = _mm512_mask_blend_epi64(mask, v1, v2);
        
        // Multi-stage pipeline
        for (int i = 0; i < 3; i++) {
            __mmask8 dynamic_mask = (i % 2) ? 0xFF : 0x55;
            __m512i temp = _mm512_add_epi64(result, _mm512_set1_epi64(100));
            result = _mm512_mask_blend_epi64(dynamic_mask, result, temp);
        }
        
        _mm512_storeu_si512((__m512i*)dst_s64, result);
        total_checksum += checksum_s64(dst_s64, ARRAY_SIZE);
    }
    
    // Test 6: V16SFmode - 16x single-precision floats
    {
        __m512 v1, v2, result;
        __mmask16 mask = 0xCCCC;  // Checkerboard pattern
        
        v1 = _mm512_loadu_ps(src1_f32);
        v2 = _mm512_loadu_ps(src2_f32);
        
        // Blend with constant mask
        result = _mm512_mask_blend_ps(mask, v1, v2);
        
        // Loop with arithmetic operations
        for (int i = 0; i < 4; i++) {
            __mmask16 dynamic_mask = (i % 3) ? 0xFFFF : 0x0F0F;
            __m512 temp = _mm512_mul_ps(result, _mm512_set1_ps(1.5f));
            result = _mm512_mask_blend_ps(dynamic_mask, result, temp);
        }
        
        _mm512_storeu_ps(dst_f32, result);
        total_checksum += (uint64_t)checksum_f32(dst_f32, ARRAY_SIZE);
    }
    
    // Test 7: V8DFmode - 8x double-precision floats
    {
        __m512d v1, v2, result;
        __mmask8 mask = 0x99;  // Pattern
        
        v1 = _mm512_loadu_pd(src1_f64);
        v2 = _mm512_loadu_pd(src2_f64);
        
        // Blend with constant mask
        result = _mm512_mask_blend_pd(mask, v1, v2);
        
        // Multi-stage processing
        for (int i = 0; i < 3; i++) {
            __mmask8 dynamic_mask = (i % 2) ? 0xFF : 0xAA;
            __m512d temp = _mm512_add_pd(result, _mm512_set1_pd(0.5));
            result = _mm512_mask_blend_pd(dynamic_mask, result, temp);
        }
        
        _mm512_storeu_pd(dst_f64, result);
        total_checksum += (uint64_t)checksum_f64(dst_f64, ARRAY_SIZE);
    }
#endif  // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blend operations...\n");
    
    // Test 8: V32BFmode - 32x brain-float
    {
        __m512bh v1, v2, result;
        __mmask32 mask = 0xF0F0F0F0;  // Pattern
        
        // Load as integers and cast to brain-float
        __m512i v1i = _mm512_loadu_si512((const __m512i*)src1_bf16);
        __m512i v2i = _mm512_loadu_si512((const __m512i*)src2_bf16);
        v1 = _mm512_castsi512_pbh(v1i);
        v2 = _mm512_castsi512_pbh(v2i);
        
        // Use inline assembly to force blend expansion
        // The intrinsic might not be directly available
        asm volatile (
            "vmovdqu64 %1, %%zmm0\n\t"
            "vmovdqu64 %2, %%zmm1\n\t"
            "vblendmps %3, %%zmm0, %%zmm1, %%zmm2\n\t"
            "vmovdqu64 %%zmm2, %0\n\t"
            : "=m"(dst_bf16)
            : "m"(src1_bf16), "m"(src2_bf16), "k"(mask)
            : "zmm0", "zmm1", "zmm2", "memory"
        );
        
        // Alternative: use C++ wrapper if available
        #ifdef _mm512_mask_blend_epi16
        // Brain-float can use epi16 blend since it's 16-bit elements
        __m512i blend_result = _mm512_mask_blend_epi16(mask, v1i, v2i);
        _mm512_storeu_si512((__m512i*)dst_bf16, blend_result);
        #endif
        
        total_checksum += checksum_u16(dst_bf16, ARRAY_SIZE);
    }
#endif  // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
