#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

int main() {
    printf("AVX-512 Blend Expansion Test\n");
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blends...\n");
    
    // Test 1: V64QImode - 64x 8-bit integers
    {
        alignas(64) uint8_t src1[64], src2[64], dst[64];
        for (int i = 0; i < 64; i++) {
            src1[i] = i;
            src2[i] = 64 - i;
        }
        
        __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
        __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
        
        // Constant mask for compile-time expansion
        __mmask64 mask_const = 0xAAAAAAAAAAAAAAAAULL;
        __m512i result_const = _mm512_mask_blend_epi8(mask_const, v1, v2);
        
        // Loop-based blend with varying mask
        __m512i result_loop = v1;
        for (int i = 0; i < 10; i++) {
            __mmask64 mask_loop = (i % 3) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
            result_loop = _mm512_mask_blend_epi8(mask_loop, result_loop, v2);
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(result_loop) : : "memory");
        }
        
        // Multi-stage pipeline
        __m512i temp = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, v1, v2);
        temp = _mm512_add_epi8(temp, _mm512_set1_epi8(1));
        __m512i final_result = _mm512_mask_blend_epi8(0xF0F0F0F0F0F0F0F0ULL, temp, v1);
        
        _mm512_storeu_si512((__m512i*)dst, final_result);
        total_checksum += checksum_512i(&final_result, 1);
    }
    
    // Test 2: V32HImode - 32x 16-bit integers
    {
        alignas(64) uint16_t src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = i * 2;
            src2[i] = i * 3;
        }
        
        __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
        __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
        
        // Loop with data-dependent mask
        __m512i accum = v1;
        for (int i = 0; i < 8; i++) {
            __mmask32 loop_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((j + i) % 4 < 2) loop_mask |= (1 << j);
            }
            accum = _mm512_mask_blend_epi16(loop_mask, accum, v2);
        }
        
        _mm512_storeu_si512((__m512i*)dst, accum);
        total_checksum += checksum_512i(&accum, 1);
    }
    
    // Test 3: V32HFmode - 32x half-precision floats
    {
#ifdef __AVX512FP16__
        alignas(64) _Float16 src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = (float)i / 2.0f;
            src2[i] = (float)i / 3.0f;
        }
        
        __m512h v1 = _mm512_loadu_ph(src1);
        __m512h v2 = _mm512_loadu_ph(src2);
        
        __mmask32 mask = 0x55555555;
        __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
        
        // Multi-stage processing
        __m512h temp = _mm512_mask_blend_ph(0x33333333, v1, v2);
        temp = _mm512_add_ph(temp, _mm512_set1_ph(1.0f));
        __m512h final = _mm512_mask_blend_ph(0x0F0F0F0F, temp, v1);
        
        _mm512_storeu_ph(dst, final);
        
        // Convert to integer for checksum
        __m512i int_result = _mm512_castph_si512(final);
        total_checksum += checksum_512i(&int_result, 1);
#endif
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blends...\n");
    
    // Test 4: V16SImode - 16x 32-bit integers
    {
        alignas(64) int32_t src1[16], src2[16], dst[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = i * 10;
            src2[i] = i * 20;
        }
        
        __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
        __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
        
        __mmask16 mask = 0xAAAA;
        __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
        
        // Pipeline with arithmetic
        __m512i temp = _mm512_mask_blend_epi32(0xCCCC, v1, v2);
        temp = _mm512_add_epi32(temp, _mm512_set1_epi32(5));
        __m512i final_result = _mm512_mask_blend_epi32(0xF0F0, temp, v1);
        
        _mm512_storeu_si512((__m512i*)dst, final_result);
        total_checksum += checksum_512i(&final_result, 1);
    }
    
    // Test 5: V8DImode - 8x 64-bit integers
    {
        alignas(64) int64_t src1[8], src2[8], dst[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = i * 100LL;
            src2[i] = i * 200LL;
        }
        
        __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
        __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
        
        __mmask8 mask = 0xAA;
        __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
        
        // Loop with varying masks
        __m512i accum = v1;
        for (int i = 0; i < 5; i++) {
            __mmask8 loop_mask = (1 << (i % 8));
            accum = _mm512_mask_blend_epi64(loop_mask, accum, v2);
        }
        
        _mm512_storeu_si512((__m512i*)dst, accum);
        total_checksum += checksum_512i(&accum, 1);
    }
    
    // Test 6: V16SFmode - 16x single-precision floats
    {
        alignas(64) float src1[16], src2[16], dst[16];
        for (int i = 0; i < 16; i++) {
            src1[i] = (float)i * 1.5f;
            src2[i] = (float)i * 2.5f;
        }
        
        __m512 v1 = _mm512_loadu_ps(src1);
        __m512 v2 = _mm512_loadu_ps(src2);
        
        __mmask16 mask = 0x5555;
        __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
        
        // Multi-stage floating-point pipeline
        __m512 temp = _mm512_mask_blend_ps(0x3333, v1, v2);
        temp = _mm512_add_ps(temp, _mm512_set1_ps(1.0f));
        __m512 final_result = _mm512_mask_blend_ps(0x0F0F, temp, v1);
        
        _mm512_storeu_ps(dst, final_result);
        total_checksum += (uint64_t)checksum_512f(&final_result, 1);
    }
    
    // Test 7: V8DFmode - 8x double-precision floats
    {
        alignas(64) double src1[8], src2[8], dst[8];
        for (int i = 0; i < 8; i++) {
            src1[i] = (double)i * 1.25;
            src2[i] = (double)i * 2.75;
        }
        
        __m512d v1 = _mm512_loadu_pd(src1);
        __m512d v2 = _mm512_loadu_pd(src2);
        
        __mmask8 mask = 0x55;
        __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
        
        // Computational kernel
        __m512d temp = _mm512_mask_blend_pd(0x33, v1, v2);
        temp = _mm512_add_pd(temp, _mm512_set1_pd(0.5));
        __m512d final_result = _mm512_mask_blend_pd(0x0F, temp, v1);
        
        _mm512_storeu_pd(dst, final_result);
        total_checksum += (uint64_t)checksum_512d(&final_result, 1);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blends...\n");
    
    // Test 8: V32BFmode - 32x brain-float
    {
        alignas(64) __bf16 src1[32], src2[32], dst[32];
        for (int i = 0; i < 32; i++) {
            src1[i] = (float)i / 4.0f;
            src2[i] = (float)i / 8.0f;
        }
        
        // Load as integers and cast
        __m512i v1_int = _mm512_loadu_si512((const __m512i*)src1);
        __m512i v2_int = _mm512_loadu_si512((const __m512i*)src2);
        
        __m512bh v1 = _mm512_castsi512_pbh(v1_int);
        __m512bh v2 = _mm512_castsi512_pbh(v2_int);
        
        __mmask32 mask = 0xAAAAAAAA;
        
        // Use inline assembly to force blend expansion
        __m512bh result;
        asm volatile(
            "vblendmpb %[mask], %[src2], %[src1], %[dst]\n\t"
            : [dst] "=v"(result)
            : [src1] "v"(v1), [src2] "v"(v2), [mask] "k"(mask)
            : "memory"
        );
        
        // Additional blend in loop
        __m512bh accum = result;
        for (int i = 0; i < 4; i++) {
            __mmask32 loop_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((j + i) % 3 == 0) loop_mask |= (1 << j);
            }
            
            // Force another blend operation
            asm volatile(
                "vblendmpb %[mask], %[src2], %[src1], %[dst]\n\t"
                : [dst] "=v"(accum)
                : [src1] "v"(accum), [src2] "v"(v2), [mask] "k"(loop_mask)
                : "memory"
            );
        }
        
        // Convert back to integer for checksum
        __m512i final_int = _mm512_castpbh_si512(accum);
        _mm512_storeu_si512((__m512i*)dst, final_int);
        total_checksum += checksum_512i(&final_int, 1);
    }
#endif // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
