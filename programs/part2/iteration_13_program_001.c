#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
void init_arrays() {
    // Arrays will be initialized in main to avoid optimization
}

// Simple checksum function to prevent optimization
uint64_t checksum_512i(__m512i vec) {
    union {
        __m512i v;
        uint8_t bytes[64];
    } u = {vec};
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += u.bytes[i];
    }
    return sum;
}

float checksum_512f(__m512 vec) {
    union {
        __m512 v;
        float floats[16];
    } u = {vec};
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += u.floats[i];
    }
    return sum;
}

double checksum_512d(__m512d vec) {
    union {
        __m512d v;
        double doubles[8];
    } u = {vec};
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += u.doubles[i];
    }
    return sum;
}

int main() {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blend operations...\n");
    
    // Test case 1: V64QImode (64x 8-bit integers)
    {
        alignas(64) uint8_t src1[64], src2[64];
        for (int i = 0; i < 64; i++) {
            src1[i] = i;
            src2[i] = 64 - i;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Multi-stage pipeline with varying masks
        __m512i result = _mm512_setzero_si512();
        for (int iter = 0; iter < 4; iter++) {
            // Constant mask pattern
            __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
            
            // Blend with constant mask
            __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
            
            // Data-dependent mask from loop
            __mmask64 dynamic_mask = 0;
            for (int i = 0; i < 64; i++) {
                if ((i + iter) % 3 == 0) {
                    dynamic_mask |= (1ULL << i);
                }
            }
            
            // Second blend with dynamic mask
            result = _mm512_mask_blend_epi8(dynamic_mask, blended, result);
            
            // Arithmetic operation
            result = _mm512_add_epi8(result, _mm512_set1_epi8(1));
        }
        
        // Force materialization with inline assembly
        __m512i final_result;
        asm volatile (
            "vmovdqa64 %1, %0\n\t"
            : "=v"(final_result)
            : "v"(result)
            : 
        );
        
        total_checksum += checksum_512i(final_result);
    }
    
    // Test case 2: V32HImode (32x 16-bit integers)
    {
        alignas(64) uint16_t src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Multi-stage processing
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            // Different constant masks
            __mmask32 mask = (i % 2) ? 0x55555555 : 0xAAAAAAAA;
            
            __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
            
            // Chain operations
            blended = _mm512_add_epi16(blended, accum);
            accum = _mm512_mask_blend_epi16(0xFFFFFFFF, blended, _mm512_set1_epi16(i));
        }
        
        total_checksum += checksum_512i(accum);
    }
    
    // Test case 3: V32HFmode (32x half-precision floats)
#ifdef __AVX512FP16__
    {
        alignas(64) _Float16 src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = (_Float16)(i * 0.5f);
            src2[i] = (_Float16)(i * 1.5f);
        }
        
        __m512h v1 = _mm512_load_ph(src1);
        __m512h v2 = _mm512_load_ph(src2);
        
        __m512h result = _mm512_setzero_ph();
        for (int iter = 0; iter < 3; iter++) {
            __mmask32 mask = 0xAAAAAAAA;
            if (iter == 1) mask = 0x55555555;
            if (iter == 2) mask = 0x33333333;
            
            result = _mm512_mask_blend_ph(mask, v1, v2);
            
            // Prevent optimization
            asm volatile (
                "vfmadd132ph %1, %2, %0\n\t"
                : "+v"(result)
                : "v"(_mm512_set1_ph(1.1f)), "v"(result)
                :
            );
        }
        
        // Convert to integer for checksum
        __m512i int_result = _mm512_castph_si512(result);
        total_checksum += checksum_512i(int_result);
    }
#endif // __AVX512FP16__
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blend operations...\n");
    
    // Test case 4: V16SImode (16x 32-bit integers)
    {
        alignas(64) int32_t src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 10;
            src2[i] = i * 20;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Pipeline with multiple blend stages
        __m512i stage1 = _mm512_mask_blend_epi32(0xAAAA, v1, v2);
        __m512i stage2 = _mm512_mask_blend_epi32(0x5555, stage1, _mm512_set1_epi32(42));
        __m512i stage3 = _mm512_mask_blend_epi32(0x3333, stage2, _mm512_set1_epi32(99));
        
        // Loop with varying masks
        __m512i final = stage3;
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (1 << i) - 1;
            final = _mm512_mask_blend_epi32(mask, final, _mm512_set1_epi32(i * 100));
        }
        
        total_checksum += checksum_512i(final);
    }
    
    // Test case 5: V8DImode (8x 64-bit integers)
    {
        alignas(64) int64_t src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 100LL;
            src2[i] = i * 200LL;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        __m512i result = _mm512_setzero_si512();
        for (int iter = 0; iter < 5; iter++) {
            __mmask8 mask = (iter % 2) ? 0xAA : 0x55;
            
            __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
            
            // Data-dependent condition
            if (iter > 2) {
                mask = 0xF0;
                blended = _mm512_mask_blend_epi64(mask, blended, result);
            }
            
            result = _mm512_add_epi64(blended, _mm512_set1_epi64(iter));
        }
        
        total_checksum += checksum_512i(result);
    }
    
    // Test case 6: V16SFmode (16x single-precision floats)
    {
        alignas(64) float src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 1.1f;
            src2[i] = i * 2.2f;
        }
        
        __m512 v1 = _mm512_load_ps(src1);
        __m512 v2 = _mm512_load_ps(src2);
        
        // Multi-stage floating-point blend pipeline
        __m512 accum = _mm512_setzero_ps();
        for (int stage = 0; stage < 3; stage++) {
            __mmask16 mask;
            switch (stage) {
                case 0: mask = 0xAAAA; break;
                case 1: mask = 0x5555; break;
                case 2: mask = 0x3333; break;
            }
            
            __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
            
            // Arithmetic operation chain
            blended = _mm512_add_ps(blended, accum);
            blended = _mm512_mul_ps(blended, _mm512_set1_ps(1.01f));
            
            accum = _mm512_mask_blend_ps(0xFFFF, blended, accum);
        }
        
        total_checksum += (uint64_t)checksum_512f(accum);
    }
    
    // Test case 7: V8DFmode (8x double-precision floats)
    {
        alignas(64) double src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 1.23;
            src2[i] = i * 3.21;
        }
        
        __m512d v1 = _mm512_load_pd(src1);
        __m512d v2 = _mm512_load_pd(src2);
        
        __m512d result = _mm512_setzero_pd();
        for (int i = 0; i < 6; i++) {
            // Pattern-based mask
            __mmask8 mask = (0xFF >> i) & 0xAA;
            
            result = _mm512_mask_blend_pd(mask, v1, v2);
            
            // Force use with inline assembly
            asm volatile (
                "vfmadd132pd %1, %2, %0\n\t"
                : "+v"(result)
                : "v"(_mm512_set1_pd(1.001)), "v"(result)
                :
            );
        }
        
        total_checksum += (uint64_t)checksum_512d(result);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blend operations...\n");
    
    // Test case 8: V32BFmode (32x brain-float)
    {
        alignas(64) uint16_t src1[32], src2[32];  // BF16 stored as uint16_t
        for (int i = 0; i < 32; i++) {
            // Simple BF16 pattern
            src1[i] = (i << 8) | (i & 0xFF);
            src2[i] = ((31 - i) << 8) | ((31 - i) & 0xFF);
        }
        
        __m512bh v1 = _mm512_load_si512((__m512i*)src1);
        __m512bh v2 = _mm512_load_si512((__m512i*)src2);
        
        // Blend pipeline for BF16
        __m512bh accum = _mm512_setzero_si512();
        for (int pass = 0; pass < 4; pass++) {
            __mmask32 mask = 0;
            for (int i = 0; i < 32; i++) {
                if ((i + pass) % 4 < 2) {
                    mask |= (1U << i);
                }
            }
            
            // Blend operation
            __m512bh blended = _mm512_mask_blend_epi16(mask, 
                _mm512_castsi512_bh(_mm512_castbh_si512(v1)),
                _mm512_castsi512_bh(_mm512_castbh_si512(v2)));
            
            // Chain with another operation
            if (pass > 0) {
                __mmask32 alt_mask = 0xAAAAAAAA;
                accum = _mm512_mask_blend_epi16(alt_mask, blended, accum);
            } else {
                accum = blended;
            }
        }
        
        // Convert to integer for checksum
        __m512i int_result = _mm512_castbh_si512(accum);
        total_checksum += checksum_512i(int_result);
    }
#endif // __AVX512BF16__

    printf("Final checksum: %lu\n", total_checksum);
    return 0;
}
