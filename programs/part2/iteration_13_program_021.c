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
void init_arrays(void) {
    // Arrays will be defined as global to prevent optimization
}

// Simple checksum function to prevent optimization
static inline uint64_t checksum_512i(__m512i vec) {
    alignas(64) uint64_t data[8];
    _mm512_store_si512(data, vec);
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) sum += data[i];
    return sum;
}

static inline float checksum_512f(__m512 vec) {
    alignas(64) float data[16];
    _mm512_store_ps(data, vec);
    float sum = 0;
    for (int i = 0; i < 16; i++) sum += data[i];
    return sum;
}

static inline double checksum_512d(__m512d vec) {
    alignas(64) double data[8];
    _mm512_store_pd(data, vec);
    double sum = 0;
    for (int i = 0; i < 8; i++) sum += data[i];
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX512F blend tests\n");
    
    // V16SImode: 16x int32
    {
        alignas(64) int32_t src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3 + 1;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        // Multi-stage pipeline with varying masks
        __m512i result = _mm512_setzero_si512();
        for (int iter = 0; iter < 4; iter++) {
            __mmask16 mask = (iter % 2) ? PATTERN_A : PATTERN_5;
            __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
            
            // Chain operations
            __m512i temp = _mm512_add_epi32(blended, _mm512_set1_epi32(iter));
            result = _mm512_mask_blend_epi32(PATTERN_C, result, temp);
        }
        
        total_checksum += checksum_512i(result);
        
        // Force materialization with inline asm
        __asm__ volatile ("" : "+v"(result) : : "memory");
    }
    
    // V8DImode: 8x int64
    {
        alignas(64) int64_t src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 5;
            src2[i] = i * 7 + 2;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        // Loop with data-dependent mask
        __m512i result = v1;
        for (int i = 0; i < 8; i++) {
            __mmask8 mask = (i % 3) ? PATTERN_A : PATTERN_5;
            result = _mm512_mask_blend_epi64(mask, result, v2);
            
            // Additional operation to create dependency
            result = _mm512_add_epi64(result, _mm512_set1_epi64(1));
        }
        
        total_checksum += checksum_512i(result);
    }
    
    // V16SFmode: 16x float
    {
        alignas(64) float src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 1.5f;
            src2[i] = i * 2.5f + 1.0f;
        }
        
        __m512 v1 = _mm512_load_ps(src1);
        __m512 v2 = _mm512_load_ps(src2);
        
        // Multi-stage blending pipeline
        __m512 result = _mm512_setzero_ps();
        for (int stage = 0; stage < 3; stage++) {
            __mmask16 mask;
            switch(stage) {
                case 0: mask = PATTERN_A; break;
                case 1: mask = PATTERN_5; break;
                default: mask = PATTERN_C; break;
            }
            
            __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
            result = _mm512_add_ps(result, blended);
            
            // Blend again with different sources
            __m512 temp = _mm512_mul_ps(blended, _mm512_set1_ps(0.5f));
            result = _mm512_mask_blend_ps(PATTERN_3, result, temp);
        }
        
        total_checksum += (uint64_t)checksum_512f(result);
    }
    
    // V8DFmode: 8x double
    {
        alignas(64) double src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 1.25;
            src2[i] = i * 2.75 + 0.5;
        }
        
        __m512d v1 = _mm512_load_pd(src1);
        __m512d v2 = _mm512_load_pd(src2);
        
        // Complex blend chain
        __m512d result = v1;
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (1ULL << i) | (1ULL << (i + 4));
            __m512d blended = _mm512_mask_blend_pd(mask, result, v2);
            
            // Create computational pipeline
            __m512d scaled = _mm512_mul_pd(blended, _mm512_set1_pd(1.1));
            result = _mm512_mask_blend_pd(PATTERN_A, blended, scaled);
        }
        
        total_checksum += (uint64_t)checksum_512d(result);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("AVX512BW blend tests\n");
    
    // V64QImode: 64x int8
    {
        alignas(64) int8_t src1[64], src2[64];
        for (int i = 0; i < 64; i++) {
            src1[i] = i;
            src2[i] = 127 - i;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        // Loop with varying blend patterns
        __m512i result = _mm512_setzero_si512();
        for (int phase = 0; phase < 2; phase++) {
            __mmask64 mask = (phase == 0) ? PATTERN_A : PATTERN_5;
            __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
            
            // Chain operations to prevent optimization
            result = _mm512_add_epi8(result, blended);
            
            // Second blend in the same iteration
            __m512i temp = _mm512_slli_epi16(blended, 1);
            result = _mm512_mask_blend_epi8(PATTERN_C, result, temp);
        }
        
        total_checksum += checksum_512i(result);
        
        // Force compiler to materialize
        __asm__ volatile ("# BW blend" : "+v"(v1), "+v"(v2) : : "memory");
    }
    
    // V32HImode: 32x int16
    {
        alignas(64) int16_t src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 10;
            src2[i] = i * 20 + 5;
        }
        
        __m512i v1 = _mm512_load_si512(src1);
        __m512i v2 = _mm512_load_si512(src2);
        
        // Data-dependent blending in loop
        __m512i result = v1;
        for (int i = 0; i < 32; i++) {
            __mmask32 mask = (i % 4 == 0) ? PATTERN_A : PATTERN_5;
            result = _mm512_mask_blend_epi16(mask, result, v2);
            
            // Prevent optimization with arithmetic
            result = _mm512_add_epi16(result, _mm512_set1_epi16(1));
        }
        
        total_checksum += checksum_512i(result);
    }
    
    // V32HFmode: 32x half-precision float
    {
#ifdef __AVX512FP16__
        alignas(64) _Float16 src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 0.5f;
            src2[i] = i * 1.5f + 0.25f;
        }
        
        __m512h v1 = _mm512_load_ph(src1);
        __m512h v2 = _mm512_load_ph(src2);
        
        // Multi-stage half-float blend pipeline
        __m512h result = _mm512_setzero_ph();
        for (int stage = 0; stage < 3; stage++) {
            __mmask32 mask = (stage == 0) ? PATTERN_A : 
                            (stage == 1) ? PATTERN_5 : PATTERN_C;
            
            // Use appropriate intrinsic for half-precision
            __m512h blended;
            // Note: _mm512_mask_blend_ph may not exist directly
            // Use cast through integer as fallback
            __m512i iv1 = _mm512_castph_si512(v1);
            __m512i iv2 = _mm512_castph_si512(v2);
            __m512i iblended = _mm512_mask_blend_epi16(mask, iv1, iv2);
            blended = _mm512_castsi512_ph(iblended);
            
            // Additional operations
            __m512h temp = _mm512_add_ph(blended, _mm512_set1_ph(1.0f));
            result = _mm512_add_ph(result, temp);
        }
        
        // Store and checksum
        alignas(64) _Float16 data[32];
        _mm512_store_ph(data, result);
        for (int i = 0; i < 32; i++) {
            total_checksum += (uint64_t)data[i];
        }
#endif // __AVX512FP16__
    }
#endif // __AVX512BW__

#ifdef __AVX512BF16__
    printf("AVX512BF16 blend tests\n");
    
    // V32BFmode: 32x brain-float
    {
        alignas(64) __bf16 src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern for bfloat16
            src1[i] = (i % 8) * 0.125f;
            src2[i] = (i % 4) * 0.25f + 0.125f;
        }
        
        // Load as integers and cast
        __m512i iv1 = _mm512_load_si512(src1);
        __m512i iv2 = _mm512_load_si512(src2);
        
        __m512bh v1 = _mm512_castsi512_pbh(iv1);
        __m512bh v2 = _mm512_castsi512_pbh(iv2);
        
        // Blend pipeline for bfloat16
        __m512bh result = v1;
        for (int iter = 0; iter < 4; iter++) {
            __mmask32 mask = (iter % 2) ? PATTERN_A : PATTERN_5;
            
            // Blend using integer blend (similar pattern to HF)
            __m512i blended_i = _mm512_mask_blend_epi16(mask, 
                _mm512_castpbh_si512(result), 
                _mm512_castpbh_si512(v2));
            
            result = _mm512_castsi512_pbh(blended_i);
            
            // Force materialization
            __asm__ volatile ("# BF16 blend %0 %1" : "+v"(result) : "v"(v2) : "memory");
        }
        
        // Convert to float for checksum
        alignas(64) float float_data[16];
        __m512 float_vec = _mm512_cvtpbh_ps(_mm512_castpbh_si512(result));
        _mm512_store_ps(float_data, float_vec);
        
        for (int i = 0; i < 16; i++) {
            total_checksum += (uint64_t)float_data[i];
        }
    }
#endif // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
