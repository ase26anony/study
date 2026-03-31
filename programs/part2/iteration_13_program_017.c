#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Initialize arrays with distinct patterns
void init_arrays(void) {
    // Arrays will be initialized in main to avoid optimization
}

// Simple checksum function to prevent optimization
uint64_t checksum_512i(const __m512i* vec, size_t count) {
    uint64_t sum = 0;
    const uint8_t* data = (const uint8_t*)vec;
    for (size_t i = 0; i < count * 64; i++) {
        sum += data[i];
    }
    return sum;
}

float checksum_512f(const __m512* vec, size_t count) {
    float sum = 0.0f;
    const float* data = (const float*)vec;
    for (size_t i = 0; i < count * 16; i++) {
        sum += data[i];
    }
    return sum;
}

double checksum_512d(const __m512d* vec, size_t count) {
    double sum = 0.0;
    const double* data = (const double*)vec;
    for (size_t i = 0; i < count * 8; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    // Initialize source arrays with distinct patterns
    alignas(64) uint8_t src1_bytes[64];
    alignas(64) uint8_t src2_bytes[64];
    alignas(64) uint8_t dst_bytes[64];
    
    alignas(64) uint16_t src1_words[32];
    alignas(64) uint16_t src2_words[32];
    alignas(64) uint16_t dst_words[32];
    
    alignas(64) uint32_t src1_dwords[16];
    alignas(64) uint32_t src2_dwords[16];
    alignas(64) uint32_t dst_dwords[16];
    
    alignas(64) uint64_t src1_qwords[8];
    alignas(64) uint64_t src2_qwords[8];
    alignas(64) uint64_t dst_qwords[8];
    
    alignas(64) float src1_floats[16];
    alignas(64) float src2_floats[16];
    alignas(64) float dst_floats[16];
    
    alignas(64) double src1_doubles[8];
    alignas(64) double src2_doubles[8];
    alignas(64) double dst_doubles[8];
    
    // Initialize with distinct patterns
    for (int i = 0; i < 64; i++) {
        src1_bytes[i] = i;
        src2_bytes[i] = 64 - i;
    }
    
    for (int i = 0; i < 32; i++) {
        src1_words[i] = i * 2;
        src2_words[i] = 1000 - i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        src1_dwords[i] = i * 100;
        src2_dwords[i] = 5000 - i * 200;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_qwords[i] = i * 1000ULL;
        src2_qwords[i] = 1000000ULL - i * 5000ULL;
    }
    
    for (int i = 0; i < 16; i++) {
        src1_floats[i] = i * 1.5f;
        src2_floats[i] = 50.0f - i * 2.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_doubles[i] = i * 2.5;
        src2_doubles[i] = 100.0 - i * 5.0;
    }
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    // Test E_V64QImode (64x char)
    {
        __m512i v1 = _mm512_load_si512((const __m512i*)src1_bytes);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2_bytes);
        
        // Multi-stage pipeline with varying masks
        __m512i result = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            // Different constant masks for each iteration
            __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL >> (i * 4);
            result = _mm512_mask_blend_epi8(mask, v1, v2);
            
            // Chain operations
            __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(i));
            result = _mm512_mask_blend_epi8(0x5555555555555555ULL, result, temp);
        }
        
        _mm512_store_si512((__m512i*)dst_bytes, result);
        
        // Force materialization with inline assembly
        asm volatile("" : "+v"(result) : : "memory");
        
        total_checksum += checksum_512i(&result, 1);
    }
    
    // Test E_V32HImode (32x short)
    {
        __m512i v1 = _mm512_load_si512((const __m512i*)src1_words);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2_words);
        
        // Loop-based blend with data-dependent mask
        __m512i result = v1;
        for (int i = 0; i < 8; i++) {
            __mmask32 mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((j + i) % 3 == 0) {
                    mask |= (1ULL << j);
                }
            }
            result = _mm512_mask_blend_epi16(mask, result, v2);
            
            // Additional arithmetic to prevent optimization
            result = _mm512_add_epi16(result, _mm512_set1_epi16(i));
        }
        
        _mm512_store_si512((__m512i*)dst_words, result);
        total_checksum += checksum_512i(&result, 1);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // Test E_V16SImode (16x int)
    {
        __m512i v1 = _mm512_load_si512((const __m512i*)src1_dwords);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2_dwords);
        
        // Multi-stage processing pipeline
        __m512i result = v1;
        for (int stage = 0; stage < 3; stage++) {
            __mmask16 mask = 0xAAAA >> (stage * 2);
            result = _mm512_mask_blend_epi32(mask, result, v2);
            
            // Blend with modified version
            __m512i shifted = _mm512_slli_epi32(result, 1);
            result = _mm512_mask_blend_epi32(0x5555, result, shifted);
        }
        
        _mm512_store_si512((__m512i*)dst_dwords, result);
        total_checksum += checksum_512i(&result, 1);
    }
    
    // Test E_V8DImode (8x long)
    {
        __m512i v1 = _mm512_load_si512((const __m512i*)src1_qwords);
        __m512i v2 = _mm512_load_si512((const __m512i*)src2_qwords);
        
        __m512i result = v1;
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (0xAA >> i) & 0xFF;
            result = _mm512_mask_blend_epi64(mask, result, v2);
            
            // Chain with arithmetic operation
            result = _mm512_add_epi64(result, _mm512_set1_epi64(i * 10));
        }
        
        _mm512_store_si512((__m512i*)dst_qwords, result);
        total_checksum += checksum_512i(&result, 1);
    }
    
    // Test E_V16SFmode (16x float)
    {
        __m512 v1 = _mm512_load_ps(src1_floats);
        __m512 v2 = _mm512_load_ps(src2_floats);
        
        // Computational kernel with multiple blends
        __m512 result = v1;
        for (int iter = 0; iter < 5; iter++) {
            __mmask16 mask = 0;
            for (int i = 0; i < 16; i++) {
                if ((i + iter) % 2 == 0) {
                    mask |= (1 << i);
                }
            }
            result = _mm512_mask_blend_ps(mask, result, v2);
            
            // Additional operation to create dependency
            result = _mm512_add_ps(result, _mm512_set1_ps(iter * 0.5f));
        }
        
        _mm512_store_ps(dst_floats, result);
        total_checksum += (uint64_t)checksum_512f(&result, 1);
    }
    
    // Test E_V8DFmode (8x double)
    {
        __m512d v1 = _mm512_load_pd(src1_doubles);
        __m512d v2 = _mm512_load_pd(src2_doubles);
        
        __m512d result = v1;
        for (int i = 0; i < 6; i++) {
            __mmask8 mask = 0x55 << (i % 4);
            result = _mm512_mask_blend_pd(mask, result, v2);
            
            // Blend with scaled version
            __m512d scaled = _mm512_mul_pd(result, _mm512_set1_pd(1.1));
            result = _mm512_mask_blend_pd(0xAA, result, scaled);
        }
        
        _mm512_store_pd(dst_doubles, result);
        total_checksum += (uint64_t)checksum_512d(&result, 1);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    // Test E_V32HFmode (32x half-float) and E_V32BFmode (32x brain-float)
    {
        // Use __m512i as base type and cast for operations
        __m512i v1_hf = _mm512_load_si512((const __m512i*)src1_words);
        __m512i v2_hf = _mm512_load_si512((const __m512i*)src2_words);
        
        // Cast to half-precision types if supported
        #ifdef __AVX512FP16__
        __m512h v1_h = _mm512_castsi512_ph(v1_hf);
        __m512h v2_h = _mm512_castsi512_ph(v2_hf);
        __m512h result_h = v1_h;
        
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = 0xAAAAAAAAUL >> (i * 4);
            // Use appropriate intrinsic for half-precision blend
            // Note: Actual intrinsic name may vary
            result_h = _mm512_mask_blend_ph(mask, result_h, v2_h);
        }
        
        // Force materialization
        asm volatile("" : "+v"(result_h) : : "memory");
        #endif
        
        // Brain-float (BF16) test
        __m512i v1_bf = _mm512_load_si512((const __m512i*)src1_words);
        __m512i v2_bf = _mm512_load_si512((const __m512i*)src2_words);
        
        #ifdef __AVX512BF16__
        __m512bh v1_b = _mm512_castsi512_pbh(v1_bf);
        __m512bh v2_b = _mm512_castsi512_pbh(v2_bf);
        __m512bh result_b = v1_b;
        
        for (int i = 0; i < 3; i++) {
            __mmask32 mask = 0x55555555UL << (i % 2);
            // Use appropriate intrinsic for bfloat16 blend
            result_b = _mm512_mask_blend_epi16(mask, 
                _mm512_castpbh_si512(result_b),
                _mm512_castpbh_si512(v2_b));
            result_b = _mm512_castsi512_pbh(
                _mm512_mask_blend_epi16(mask, 
                    _mm512_castpbh_si512(result_b),
                    _mm512_castpbh_si512(v2_b)));
        }
        
        // Force materialization
        asm volatile("" : "+v"(result_b) : : "memory");
        #endif
        
        total_checksum += checksum_512i(&v1_hf, 1);
    }
#endif // __AVX512BF16__

    // Print checksum to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    
    // Use results to prevent dead code elimination
    volatile uint8_t sink = dst_bytes[0] + dst_words[0] + dst_dwords[0] + 
                           (uint8_t)dst_qwords[0] + (uint8_t)dst_floats[0] + 
                           (uint8_t)dst_doubles[0];
    
    return 0;
}
