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
    
    // Initialize source arrays with distinct patterns
    alignas(64) uint8_t src8_a[64], src8_b[64];
    alignas(64) uint16_t src16_a[32], src16_b[32];
    alignas(64) uint32_t src32_a[16], src32_b[16];
    alignas(64) uint64_t src64_a[8], src64_b[8];
    alignas(64) float srcf_a[16], srcf_b[16];
    alignas(64) double srcd_a[8], srcd_b[8];
    
    for (int i = 0; i < 64; i++) {
        src8_a[i] = i;
        src8_b[i] = 64 - i;
    }
    for (int i = 0; i < 32; i++) {
        src16_a[i] = i * 2;
        src16_b[i] = 1000 - i * 3;
    }
    for (int i = 0; i < 16; i++) {
        src32_a[i] = i * 100;
        src32_b[i] = 5000 - i * 200;
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = 100.0f - i * 3.0f;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 1000ULL;
        src64_b[i] = 1000000ULL - i * 5000ULL;
        srcd_a[i] = i * 2.5;
        srcd_b[i] = 200.0 - i * 10.0;
    }
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blend operations...\n");
    
    // Test 1: V64QImode - 64x 8-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src8_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src8_b);
        
        // Multi-stage processing pipeline
        __m512i result = _mm512_setzero_si512();
        
        // Loop with varying masks to prevent optimization
        for (int iter = 0; iter < 4; iter++) {
            // Create mask based on iteration
            __mmask64 mask = (iter % 2) ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
            
            // Chain blend operations
            __m512i temp = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
            
            // Additional arithmetic operation
            temp = _mm512_add_epi8(temp, _mm512_set1_epi8(iter));
            
            // Second blend in the pipeline
            __mmask64 mask2 = (iter % 3) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
            result = _mm512_mask_blend_epi8(mask2, result, temp);
            
            // Force compiler to materialize vectors
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        uint64_t cs = checksum_512i(result);
        total_checksum += cs;
        printf("  V64QImode checksum: %lu\n", cs);
    }
    
    // Test 2: V32HImode - 32x 16-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src16_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src16_b);
        
        __m512i result = _mm512_setzero_si512();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask32 mask = (iter % 2) ? 0xAAAAAAAA : 0x55555555;
            
            __m512i temp = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
            temp = _mm512_add_epi16(temp, _mm512_set1_epi16(iter));
            
            __mmask32 mask2 = (iter % 3) ? 0xCCCCCCCC : 0x33333333;
            result = _mm512_mask_blend_epi16(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        uint64_t cs = checksum_512i(result);
        total_checksum += cs;
        printf("  V32HImode checksum: %lu\n", cs);
    }
    
#ifdef __AVX512FP16__
    // Test 3: V32HFmode - 32x half-precision floats
    {
        // Initialize half-precision arrays
        alignas(64) uint16_t srchf_a[32], srchf_b[32];
        for (int i = 0; i < 32; i++) {
            srchf_a[i] = i * 256;  // Simple pattern
            srchf_b[i] = 8192 - i * 128;
        }
        
        __m512h vec_a = _mm512_castsi512_ph(_mm512_load_si512((__m512i*)srchf_a));
        __m512h vec_b = _mm512_castsi512_ph(_mm512_load_si512((__m512i*)srchf_b));
        
        __m512h result = _mm512_setzero_ph();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask32 mask = (iter % 2) ? 0xAAAAAAAA : 0x55555555;
            
            // Use appropriate intrinsic for half-precision blend
            __m512h temp = _mm512_mask_blend_ph(mask, vec_a, vec_b);
            
            // Add constant
            temp = _mm512_add_ph(temp, _mm512_set1_ph(_cvtsh_ss(iter)));
            
            __mmask32 mask2 = (iter % 3) ? 0xCCCCCCCC : 0x33333333;
            result = _mm512_mask_blend_ph(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        // Convert to integer for checksum
        __m512i result_i = _mm512_castph_si512(result);
        uint64_t cs = checksum_512i(result_i);
        total_checksum += cs;
        printf("  V32HFmode checksum: %lu\n", cs);
    }
#endif  // __AVX512FP16__
#endif  // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blend operations...\n");
    
    // Test 4: V16SImode - 16x 32-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src32_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src32_b);
        
        __m512i result = _mm512_setzero_si512();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask16 mask = (iter % 2) ? 0xAAAA : 0x5555;
            
            __m512i temp = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
            temp = _mm512_add_epi32(temp, _mm512_set1_epi32(iter));
            
            __mmask16 mask2 = (iter % 3) ? 0xCCCC : 0x3333;
            result = _mm512_mask_blend_epi32(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        uint64_t cs = checksum_512i(result);
        total_checksum += cs;
        printf("  V16SImode checksum: %lu\n", cs);
    }
    
    // Test 5: V8DImode - 8x 64-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src64_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src64_b);
        
        __m512i result = _mm512_setzero_si512();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask8 mask = (iter % 2) ? 0xAA : 0x55;
            
            __m512i temp = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
            temp = _mm512_add_epi64(temp, _mm512_set1_epi64(iter));
            
            __mmask8 mask2 = (iter % 3) ? 0xCC : 0x33;
            result = _mm512_mask_blend_epi64(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        uint64_t cs = checksum_512i(result);
        total_checksum += cs;
        printf("  V8DImode checksum: %lu\n", cs);
    }
    
    // Test 6: V16SFmode - 16x single-precision floats
    {
        __m512 vec_a = _mm512_load_ps(srcf_a);
        __m512 vec_b = _mm512_load_ps(srcf_b);
        
        __m512 result = _mm512_setzero_ps();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask16 mask = (iter % 2) ? 0xAAAA : 0x5555;
            
            __m512 temp = _mm512_mask_blend_ps(mask, vec_a, vec_b);
            temp = _mm512_add_ps(temp, _mm512_set1_ps(iter * 0.5f));
            
            __mmask16 mask2 = (iter % 3) ? 0xCCCC : 0x3333;
            result = _mm512_mask_blend_ps(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        float cs = checksum_512f(result);
        total_checksum += (uint64_t)(cs * 1000);
        printf("  V16SFmode checksum: %.2f\n", cs);
    }
    
    // Test 7: V8DFmode - 8x double-precision floats
    {
        __m512d vec_a = _mm512_load_pd(srcd_a);
        __m512d vec_b = _mm512_load_pd(srcd_b);
        
        __m512d result = _mm512_setzero_pd();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask8 mask = (iter % 2) ? 0xAA : 0x55;
            
            __m512d temp = _mm512_mask_blend_pd(mask, vec_a, vec_b);
            temp = _mm512_add_pd(temp, _mm512_set1_pd(iter * 0.25));
            
            __mmask8 mask2 = (iter % 3) ? 0xCC : 0x33;
            result = _mm512_mask_blend_pd(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        double cs = checksum_512d(result);
        total_checksum += (uint64_t)(cs * 1000);
        printf("  V8DFmode checksum: %.2f\n", cs);
    }
#endif  // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blend operations...\n");
    
    // Test 8: V32BFmode - 32x brain-float
    {
        // Initialize brain-float arrays
        alignas(64) uint16_t srcbf_a[32], srcbf_b[32];
        for (int i = 0; i < 32; i++) {
            srcbf_a[i] = i * 128;  // Simple pattern for bfloat16
            srcbf_b[i] = 32768 - i * 64;
        }
        
        __m512bh vec_a = _mm512_castsi512_pbh(_mm512_load_si512((__m512i*)srcbf_a));
        __m512bh vec_b = _mm512_castsi512_pbh(_mm512_load_si512((__m512i*)srcbf_b));
        
        __m512bh result = _mm512_setzero_bh();
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask32 mask = (iter % 2) ? 0xAAAAAAAA : 0x55555555;
            
            // Use appropriate intrinsic for bfloat16 blend
            __m512bh temp = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
            
            // Convert to float, add, convert back for processing
            __m512 temp_f = _mm512_cvtpbh_ps(temp);
            temp_f = _mm512_add_ps(temp_f, _mm512_set1_ps(iter * 0.1f));
            temp = _mm512_cvtne2ps_pbh(temp_f, temp_f);  // Approximation
            
            __mmask32 mask2 = (iter % 3) ? 0xCCCCCCCC : 0x33333333;
            result = _mm512_mask_blend_epi16(mask2, result, temp);
            
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        // Convert to integer for checksum
        __m512i result_i = _mm512_castpbh_si512(result);
        uint64_t cs = checksum_512i(result_i);
        total_checksum += cs;
        printf("  V32BFmode checksum: %lu\n", cs);
    }
#endif  // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
