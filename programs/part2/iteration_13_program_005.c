#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Constants for predictable patterns
#define PATTERN_A 0xAAAAAAAAAAAAAAAAULL
#define PATTERN_5 0x5555555555555555ULL
#define PATTERN_C 0xCCCCCCCCCCCCCCCCULL
#define PATTERN_3 0x3333333333333333ULL

// Initialize arrays with distinct patterns
void init_arrays(void* data_int8, void* data_int16, void* data_int32, 
                 void* data_int64, void* data_float, void* data_double,
                 void* data_half, void* data_bf16, size_t elements) {
    // 8-bit integer pattern
    int8_t* int8_arr = (int8_t*)data_int8;
    for (size_t i = 0; i < elements * 64; i++) {
        int8_arr[i] = (int8_t)(i * 3 + 1);
    }
    
    // 16-bit integer pattern
    int16_t* int16_arr = (int16_t*)data_int16;
    for (size_t i = 0; i < elements * 32; i++) {
        int16_arr[i] = (int16_t)(i * 5 - 2);
    }
    
    // 32-bit integer pattern
    int32_t* int32_arr = (int32_t*)data_int32;
    for (size_t i = 0; i < elements * 16; i++) {
        int32_arr[i] = (int32_t)(i * 7 + 3);
    }
    
    // 64-bit integer pattern
    int64_t* int64_arr = (int64_t*)data_int64;
    for (size_t i = 0; i < elements * 8; i++) {
        int64_arr[i] = (int64_t)(i * 11 - 5);
    }
    
    // Single-precision float pattern
    float* float_arr = (float*)data_float;
    for (size_t i = 0; i < elements * 16; i++) {
        float_arr[i] = (float)(i * 0.5f + 1.0f);
    }
    
    // Double-precision float pattern
    double* double_arr = (double*)data_double;
    for (size_t i = 0; i < elements * 8; i++) {
        double_arr[i] = (double)(i * 0.25 + 2.0);
    }
    
    // Half-precision pattern (stored as uint16_t)
    uint16_t* half_arr = (uint16_t*)data_half;
    for (size_t i = 0; i < elements * 32; i++) {
        half_arr[i] = (uint16_t)((i % 1024) + 0x3C00); // 1.0 in half-float
    }
    
    // Brain-float pattern (stored as uint16_t)
    uint16_t* bf16_arr = (uint16_t*)data_bf16;
    for (size_t i = 0; i < elements * 32; i++) {
        bf16_arr[i] = (uint16_t)((i % 256) + 0x3F80); // ~1.0 in bfloat16
    }
}

// Multi-stage processing pipeline for each data type
#ifdef __AVX512BW__
void process_v64qi(__m512i* src1, __m512i* src2, __m512i* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Stage 1: Blend with constant pattern
        __m512i v1 = _mm512_loadu_si512(&src1[i]);
        __m512i v2 = _mm512_loadu_si512(&src2[i]);
        __m512i blended1 = _mm512_mask_blend_epi8(PATTERN_A, v1, v2);
        
        // Stage 2: Data-dependent blend in loop
        __mmask64 mask = 0;
        for (int j = 0; j < 64; j++) {
            if ((i + j) % 3 == 0) {
                mask |= (1ULL << j);
            }
        }
        __m512i blended2 = _mm512_mask_blend_epi8(mask, blended1, v1);
        
        // Stage 3: Another constant pattern blend
        __m512i blended3 = _mm512_mask_blend_epi8(PATTERN_C, blended2, v2);
        
        // Store result
        _mm512_storeu_si512(&dst[i], blended3);
        
        // Force materialization with inline assembly
        asm volatile("" : "+v"(blended3) : : "memory");
    }
}

void process_v32hi(__m512i* src1, __m512i* src2, __m512i* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        __m512i v1 = _mm512_loadu_si512(&src1[i]);
        __m512i v2 = _mm512_loadu_si512(&src2[i]);
        
        // Multi-stage blending with different patterns
        __m512i b1 = _mm512_mask_blend_epi16(PATTERN_A, v1, v2);
        __m512i b2 = _mm512_mask_blend_epi16(PATTERN_5, b1, v1);
        
        // Loop-dependent mask
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            mask |= ((i % (j + 2)) > 0) ? (1U << j) : 0;
        }
        __m512i b3 = _mm512_mask_blend_epi16(mask, b2, v2);
        
        _mm512_storeu_si512(&dst[i], b3);
        asm volatile("" : "+v"(b3) : : "memory");
    }
}
#endif // __AVX512BW__

#ifdef __AVX512F__
void process_v16si(__m512i* src1, __m512i* src2, __m512i* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        __m512i v1 = _mm512_loadu_si512(&src1[i]);
        __m512i v2 = _mm512_loadu_si512(&src2[i]);
        
        // Chain of blend operations
        __m512i b1 = _mm512_mask_blend_epi32(PATTERN_A, v1, v2);
        __m512i b2 = _mm512_mask_blend_epi32(PATTERN_3, b1, v1);
        __m512i b3 = _mm512_mask_blend_epi32(PATTERN_C, b2, v2);
        
        // Additional arithmetic operation
        b3 = _mm512_add_epi32(b3, _mm512_set1_epi32(1));
        
        _mm512_storeu_si512(&dst[i], b3);
        asm volatile("" : "+v"(b3) : : "memory");
    }
}

void process_v8di(__m512i* src1, __m512i* src2, __m512i* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        __m512i v1 = _mm512_loadu_si512(&src1[i]);
        __m512i v2 = _mm512_loadu_si512(&src2[i]);
        
        __m512i b1 = _mm512_mask_blend_epi64(PATTERN_A, v1, v2);
        
        // Data-dependent mask
        __mmask8 mask = 0;
        for (int j = 0; j < 8; j++) {
            mask |= ((i >> j) & 1) ? (1U << j) : 0;
        }
        __m512i b2 = _mm512_mask_blend_epi64(mask, b1, v1);
        
        _mm512_storeu_si512(&dst[i], b2);
        asm volatile("" : "+v"(b2) : : "memory");
    }
}

void process_v16sf(__m512* src1, __m512* src2, __m512* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        __m512 v1 = _mm512_loadu_ps(&src1[i]);
        __m512 v2 = _mm512_loadu_ps(&src2[i]);
        
        // Multi-stage floating-point blend pipeline
        __m512 b1 = _mm512_mask_blend_ps(PATTERN_A, v1, v2);
        __m512 b2 = _mm512_mask_blend_ps(PATTERN_5, b1, v1);
        
        // Additional operation to create dependency
        b2 = _mm512_mul_ps(b2, _mm512_set1_ps(1.5f));
        
        __m512 b3 = _mm512_mask_blend_ps(PATTERN_C, b2, v2);
        
        _mm512_storeu_ps(&dst[i], b3);
        asm volatile("" : "+v"(b3) : : "memory");
    }
}

void process_v8df(__m512d* src1, __m512d* src2, __m512d* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        __m512d v1 = _mm512_loadu_pd(&src1[i]);
        __m512d v2 = _mm512_loadu_pd(&src2[i]);
        
        __m512d b1 = _mm512_mask_blend_pd(PATTERN_A, v1, v2);
        
        // Loop-varying mask
        __mmask8 mask = (i % 2) ? PATTERN_A : PATTERN_5;
        __m512d b2 = _mm512_mask_blend_pd(mask, b1, v1);
        
        _mm512_storeu_pd(&dst[i], b2);
        asm volatile("" : "+v"(b2) : : "memory");
    }
}
#endif // __AVX512F__

#ifdef __AVX512BF16__
// For half-precision (HF) using __m512h when available
void process_v32hf(void* src1, void* src2, void* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Load as integers and cast to half-precision
        __m512i v1_i = _mm512_loadu_si512((__m512i*)src1 + i);
        __m512i v2_i = _mm512_loadu_si512((__m512i*)src2 + i);
        
        __m512h v1 = _mm512_castsi512_ph(v1_i);
        __m512h v2 = _mm512_castsi512_ph(v2_i);
        
        // Blend operation
        __m512h blended = _mm512_mask_blend_ph(PATTERN_A, v1, v2);
        
        // Store back
        __m512i result_i = _mm512_castph_si512(blended);
        _mm512_storeu_si512((__m512i*)dst + i, result_i);
        
        asm volatile("" : "+v"(blended) : : "memory");
    }
}

// For brain-float (BF) using __m512bh when available
void process_v32bf(void* src1, void* src2, void* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        // Load as integers and cast to brain-float
        __m512i v1_i = _mm512_loadu_si512((__m512i*)src1 + i);
        __m512i v2_i = _mm512_loadu_si512((__m512i*)src2 + i);
        
        __m512bh v1 = _mm512_castsi512_pbh(v1_i);
        __m512bh v2 = _mm512_castsi512_pbh(v2_i);
        
        // Blend operation
        __m512bh blended = _mm512_mask_blend_epi16(PATTERN_A, v1, v2);
        
        // Store back
        __m512i result_i = _mm512_castpbh_si512(blended);
        _mm512_storeu_si512((__m512i*)dst + i, result_i);
        
        asm volatile("" : "+v"(blended) : : "memory");
    }
}
#endif // __AVX512BF16__

// Compute checksum to prevent optimization
uint64_t compute_checksum(void* data, size_t bytes) {
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)data;
    for (size_t i = 0; i < bytes; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main() {
    const size_t ARRAY_COUNT = 16;
    
    // Allocate aligned memory for all data types
    void* src1_int8 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_int8 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_int8 = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_int16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_int16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_int16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_int32 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_int32 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_int32 = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_int64 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_int64 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_int64 = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_float = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_float = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_float = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_double = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_double = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_double = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_half = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_half = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_half = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    void* src1_bf16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* src2_bf16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    void* dst_bf16 = _mm_malloc(64 * ARRAY_COUNT, 64);
    
    // Initialize all arrays
    init_arrays(src1_int8, src1_int16, src1_int32, src1_int64,
                src1_float, src1_double, src1_half, src1_bf16, ARRAY_COUNT);
    init_arrays(src2_int8, src2_int16, src2_int32, src2_int64,
                src2_float, src2_double, src2_half, src2_bf16, ARRAY_COUNT);
    
    uint64_t total_checksum = 0;
    
    // Process each data type with appropriate intrinsics
#ifdef __AVX512BW__
    process_v64qi((__m512i*)src1_int8, (__m512i*)src2_int8, 
                  (__m512i*)dst_int8, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_int8, 64 * ARRAY_COUNT);
    
    process_v32hi((__m512i*)src1_int16, (__m512i*)src2_int16, 
                  (__m512i*)dst_int16, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_int16, 64 * ARRAY_COUNT);
#endif
    
#ifdef __AVX512F__
    process_v16si((__m512i*)src1_int32, (__m512i*)src2_int32, 
                  (__m512i*)dst_int32, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_int32, 64 * ARRAY_COUNT);
    
    process_v8di((__m512i*)src1_int64, (__m512i*)src2_int64, 
                 (__m512i*)dst_int64, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_int64, 64 * ARRAY_COUNT);
    
    process_v16sf((__m512*)src1_float, (__m512*)src2_float, 
                  (__m512*)dst_float, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_float, 64 * ARRAY_COUNT);
    
    process_v8df((__m512d*)src1_double, (__m512d*)src2_double, 
                 (__m512d*)dst_double, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_double, 64 * ARRAY_COUNT);
#endif
    
#ifdef __AVX512BF16__
    process_v32hf(src1_half, src2_half, dst_half, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_half, 64 * ARRAY_COUNT);
    
    process_v32bf(src1_bf16, src2_bf16, dst_bf16, ARRAY_COUNT);
    total_checksum += compute_checksum(dst_bf16, 64 * ARRAY_COUNT);
#endif
    
    // Print deterministic checksum
    printf("Total checksum: %lu\n", total_checksum);
    
    // Cleanup
    _mm_free(src1_int8); _mm_free(src2_int8); _mm_free(dst_int8);
    _mm_free(src1_int16); _mm_free(src2_int16); _mm_free(dst_int16);
    _mm_free(src1_int32); _mm_free(src2_int32); _mm_free(dst_int32);
    _mm_free(src1_int64); _mm_free(src2_int64); _mm_free(dst_int64);
    _mm_free(src1_float); _mm_free(src2_float); _mm_free(dst_float);
    _mm_free(src1_double); _mm_free(src2_double); _mm_free(dst_double);
    _mm_free(src1_half); _mm_free(src2_half); _mm_free(dst_half);
    _mm_free(src1_bf16); _mm_free(src2_bf16); _mm_free(dst_bf16);
    
    return 0;
}
