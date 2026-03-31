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
        uint64_t a[8];
    } u;
    u.v = vec;
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += u.a[i];
    }
    return sum;
}

float checksum_512f(__m512 vec) {
    union {
        __m512 v;
        float a[16];
    } u;
    u.v = vec;
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += u.a[i];
    }
    return sum;
}

double checksum_512d(__m512d vec) {
    union {
        __m512d v;
        double a[8];
    } u;
    u.v = vec;
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += u.a[i];
    }
    return sum;
}

int main() {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blend expansions...\n");
    
    // Test E_V64QImode (64x char)
    {
        alignas(64) int8_t src1[64], src2[64];
        for (int i = 0; i < 64; i++) {
            src1[i] = i;
            src2[i] = 64 - i;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Multi-stage pipeline with varying masks
        __m512i result = _mm512_setzero_si512();
        for (int iter = 0; iter < 4; iter++) {
            // Constant mask for compile-time expansion
            __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
            
            // Blend operation that should trigger E_V64QImode
            __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
            
            // Data-dependent operation to prevent optimization
            if (iter % 2 == 0) {
                result = _mm512_add_epi8(result, blended);
            } else {
                result = _mm512_sub_epi8(result, blended);
            }
            
            // Modify vectors for next iteration
            v1 = _mm512_add_epi8(v1, _mm512_set1_epi8(1));
            v2 = _mm512_sub_epi8(v2, _mm512_set1_epi8(1));
        }
        
        // Force materialization with inline assembly
        __m512i final_result;
        asm volatile ("vmovdqa64 %1, %0\n\t"
                      : "=v"(final_result)
                      : "v"(result)
                      : "memory");
        
        total_checksum += checksum_512i(final_result);
    }
    
    // Test E_V32HImode (32x short)
    {
        alignas(64) int16_t src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Loop with varying mask pattern
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            // Create mask based on loop index
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0xAAAAAAAA;
            
            // Blend that should trigger E_V32HImode
            __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
            
            // Chain operations
            accum = _mm512_add_epi16(accum, blended);
            
            // Rotate vectors
            v1 = _mm512_alignr_epi8(v1, v1, 2);
            v2 = _mm512_alignr_epi8(v2, v2, 2);
        }
        
        total_checksum += checksum_512i(accum);
    }
    
    // Test E_V32HFmode (32x half precision)
    {
        alignas(64) uint16_t src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = 0x3C00 | (i & 0x1F);  // 1.0 + small variations
            src2[i] = 0x4000 | (i & 0x1F);  // 2.0 + small variations
        }
        
        __m512i v1_i = _mm512_load_si512((__m512i*)src1);
        __m512i v2_i = _mm512_load_si512((__m512i*)src2);
        
        // Cast to half precision if supported
#ifdef __AVX512FP16__
        __m512h v1 = _mm512_castsi512_ph(v1_i);
        __m512h v2 = _mm512_castsi512_ph(v2_i);
        
        __m512h result_h = _mm512_setzero_ph();
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = 0x55555555 << (i % 4);
            __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
            result_h = _mm512_add_ph(result_h, blended);
        }
        
        // Convert back for checksum
        __m512i result_i = _mm512_castph_si512(result_h);
        total_checksum += checksum_512i(result_i);
#else
        // Use integer blend as fallback - still triggers the mode
        __mmask32 mask = 0xAAAAAAAA;
        __m512i blended = _mm512_mask_blend_epi16(mask, v1_i, v2_i);
        total_checksum += checksum_512i(blended);
#endif
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blend expansions...\n");
    
    // Test E_V16SImode (16x int)
    {
        alignas(64) int32_t src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 100;
            src2[i] = i * 200;
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Multi-stage processing pipeline
        __m512i stage1 = _mm512_setzero_si512();
        __m512i stage2 = _mm512_setzero_si512();
        
        for (int i = 0; i < 4; i++) {
            // First blend with constant mask
            __mmask16 mask1 = 0xAAAA;
            __m512i blended1 = _mm512_mask_blend_epi32(mask1, v1, v2);
            
            // Second blend with different mask
            __mmask16 mask2 = 0x5555 << (i & 1);
            __m512i blended2 = _mm512_mask_blend_epi32(mask2, blended1, v1);
            
            // Arithmetic operation
            stage1 = _mm512_add_epi32(stage1, blended1);
            stage2 = _mm512_add_epi32(stage2, blended2);
            
            // Modify source vectors
            v1 = _mm512_add_epi32(v1, _mm512_set1_epi32(10));
        }
        
        // Final blend between stages
        __mmask16 final_mask = 0x3333;
        __m512i final_result = _mm512_mask_blend_epi32(final_mask, stage1, stage2);
        
        // Force compiler to materialize
        asm volatile ("" : "+v"(final_result) : : "memory");
        
        total_checksum += checksum_512i(final_result);
    }
    
    // Test E_V8DImode (8x long)
    {
        alignas(64) int64_t src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = 1LL << i;
            src2[i] = 1LL << (i + 8);
        }
        
        __m512i v1 = _mm512_load_si512((__m512i*)src1);
        __m512i v2 = _mm512_load_si512((__m512i*)src2);
        
        // Loop with data-dependent mask
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            // Mask depends on loop counter
            __mmask8 mask = (1 << (i % 8)) - 1;
            
            // Blend that should trigger E_V8DImode
            __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
            
            accum = _mm512_add_epi64(accum, blended);
            
            // Rotate vectors
            v1 = _mm512_alignr_epi32(v1, v1, 2);
        }
        
        total_checksum += checksum_512i(accum);
    }
    
    // Test E_V16SFmode (16x float)
    {
        alignas(64) float src1[16], src2[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 1.5f;
            src2[i] = i * 2.5f;
        }
        
        __m512 v1 = _mm512_load_ps(src1);
        __m512 v2 = _mm512_load_ps(src2);
        
        // Multi-stage filter simulation
        __m512 result = _mm512_setzero_ps();
        for (int stage = 0; stage < 3; stage++) {
            // Different mask pattern for each stage
            __mmask16 mask;
            switch (stage) {
                case 0: mask = 0xAAAA; break;
                case 1: mask = 0xCCCC; break;
                case 2: mask = 0xF0F0; break;
            }
            
            // Blend that should trigger E_V16SFmode
            __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
            
            // Additional arithmetic
            blended = _mm512_mul_ps(blended, _mm512_set1_ps(1.1f));
            
            result = _mm512_add_ps(result, blended);
            
            // Update vectors
            v1 = _mm512_add_ps(v1, _mm512_set1_ps(0.5f));
        }
        
        float sum = checksum_512f(result);
        total_checksum += (uint64_t)(sum * 1000);
    }
    
    // Test E_V8DFmode (8x double)
    {
        alignas(64) double src1[8], src2[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = sqrt(i + 1.0);
            src2[i] = log(i + 2.0);
        }
        
        __m512d v1 = _mm512_load_pd(src1);
        __m512d v2 = _mm512_load_pd(src2);
        
        // Pipeline with conditional blending
        __m512d accum = _mm512_setzero_pd();
        for (int i = 0; i < 6; i++) {
            // Create checkerboard mask
            __mmask8 mask = (i % 2) ? 0xAA : 0x55;
            
            // Blend that should trigger E_V8DFmode
            __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
            
            // Chain with arithmetic
            blended = _mm512_mul_pd(blended, _mm512_set1_pd(1.01));
            
            accum = _mm512_add_pd(accum, blended);
            
            // Rotate vectors
            v1 = _mm512_permutexvar_pd(_mm512_set_epi64(0,7,6,5,4,3,2,1), v1);
        }
        
        double sum = checksum_512d(accum);
        total_checksum += (uint64_t)(sum * 1000);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blend expansions...\n");
    
    // Test E_V32BFmode (32x brain float)
    {
        alignas(64) uint16_t src1[32], src2[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = 0x3F80 | (i & 0x7F);  // ~1.0 in bfloat16
            src2[i] = 0x4000 | (i & 0x7F);  // ~2.0 in bfloat16
        }
        
        __m512i v1_i = _mm512_load_si512((__m512i*)src1);
        __m512i v2_i = _mm512_load_si512((__m512i*)src2);
        
        // Cast to brain float if supported
        __m512bh v1 = _mm512_castsi512_pbh(v1_i);
        __m512bh v2 = _mm512_castsi512_pbh(v2_i);
        
        // Use integer blend with bfloat16 pattern
        __mmask32 mask = 0xAAAAAAAA;
        __m512i blended_i = _mm512_mask_blend_epi16(mask, v1_i, v2_i);
        
        // Convert back for processing
        __m512bh blended = _mm512_castsi512_pbh(blended_i);
        
        // Force materialization
        asm volatile ("vmovdqa64 %1, %0\n\t"
                      : "=v"(blended_i)
                      : "v"(_mm512_castpbh_si512(blended))
                      : "memory");
        
        total_checksum += checksum_512i(blended_i);
    }
#endif // __AVX512BF16__

    printf("Final checksum: %lu\n", total_checksum);
    return 0;
}
