#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
#define ARRAY_SIZE 64

// For integer types
static uint8_t src8_a[ARRAY_SIZE];
static uint8_t src8_b[ARRAY_SIZE];
static uint8_t dst8[ARRAY_SIZE];

static uint16_t src16_a[ARRAY_SIZE];
static uint16_t src16_b[ARRAY_SIZE];
static uint16_t dst16[ARRAY_SIZE];

static uint32_t src32_a[ARRAY_SIZE];
static uint32_t src32_b[ARRAY_SIZE];
static uint32_t dst32[ARRAY_SIZE];

static uint64_t src64_a[ARRAY_SIZE];
static uint64_t src64_b[ARRAY_SIZE];
static uint64_t dst64[ARRAY_SIZE];

// For floating-point types
static float srcf_a[ARRAY_SIZE];
static float srcf_b[ARRAY_SIZE];
static float dstf[ARRAY_SIZE];

static double srcd_a[ARRAY_SIZE];
static double srcd_b[ARRAY_SIZE];
static double dstd[ARRAY_SIZE];

// For half-precision (stored as uint16_t)
static uint16_t srch_a[ARRAY_SIZE];
static uint16_t srch_b[ARRAY_SIZE];
static uint16_t dsth[ARRAY_SIZE];

// For brain-float (stored as uint16_t)
static uint16_t srcb_a[ARRAY_SIZE];
static uint16_t srcb_b[ARRAY_SIZE];
static uint16_t dstb[ARRAY_SIZE];

// Initialize all arrays with distinct patterns
void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Integer patterns
        src8_a[i] = i;
        src8_b[i] = 255 - i;
        
        src16_a[i] = i * 2;
        src16_b[i] = 65535 - i * 2;
        
        src32_a[i] = i * 100;
        src32_b[i] = 0xFFFFFFFF - i * 100;
        
        src64_a[i] = (uint64_t)i * 1000;
        src64_b[i] = 0xFFFFFFFFFFFFFFFFULL - (uint64_t)i * 1000;
        
        // Floating-point patterns
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = 100.0f - i * 1.5f;
        
        srcd_a[i] = i * 2.5;
        srcd_b[i] = 200.0 - i * 2.5;
        
        // Half-precision patterns (store as raw bits)
        float fh_a = i * 0.5f;
        float fh_b = 50.0f - i * 0.5f;
        srch_a[i] = _cvtsh_ss(fh_a);
        srch_b[i] = _cvtsh_ss(fh_b);
        
        // Brain-float patterns (store as raw bits)
        srcb_a[i] = _cvtsh_ss(i * 0.25f);
        srcb_b[i] = _cvtsh_ss(25.0f - i * 0.25f);
    }
}

// Simple checksum function
uint64_t checksum_8(const uint8_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

uint64_t checksum_16(const uint16_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

uint64_t checksum_32(const uint32_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

uint64_t checksum_64(const uint64_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

double checksum_f(const float* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

double checksum_d(const double* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    init_arrays();
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    // ==================== V64QImode (64x char) ====================
    {
        // Load vectors
        __m512i va = _mm512_loadu_si512((__m512i*)src8_a);
        __m512i vb = _mm512_loadu_si512((__m512i*)src8_b);
        
        // Multi-stage blend pipeline
        __m512i vresult = _mm512_setzero_si512();
        
        // Loop with varying masks to prevent optimization
        for (int i = 0; i < 4; i++) {
            // Different constant masks for each iteration
            __mmask64 mask;
            switch (i) {
                case 0: mask = 0xAAAAAAAAAAAAAAAAULL; break;  // Alternating bits
                case 1: mask = 0x5555555555555555ULL; break;  // Opposite alternating
                case 2: mask = 0xFFFFFFFFFFFFFFFFULL; break;  // All ones
                case 3: mask = 0x0F0F0F0F0F0F0F0FULL; break;  // Checkerboard
            }
            
            // Blend operation
            vresult = _mm512_mask_blend_epi8(mask, va, vb);
            
            // Additional arithmetic to create dependency chain
            vresult = _mm512_add_epi8(vresult, _mm512_set1_epi8(1));
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(vresult) : : "memory");
        }
        
        // Final blend with data-dependent mask
        __mmask64 final_mask = 0;
        for (int i = 0; i < 64; i++) {
            if ((i % 3) == 0) {
                final_mask |= (1ULL << i);
            }
        }
        vresult = _mm512_mask_blend_epi8(final_mask, va, vb);
        
        _mm512_storeu_si512((__m512i*)dst8, vresult);
        total_checksum += checksum_8(dst8, ARRAY_SIZE);
    }
    
    // ==================== V32HImode (32x short) ====================
    {
        __m512i va = _mm512_loadu_si512((__m512i*)src16_a);
        __m512i vb = _mm512_loadu_si512((__m512i*)src16_b);
        
        __m512i vresult = _mm512_setzero_si512();
        
        // Multi-stage processing
        for (int iter = 0; iter < 3; iter++) {
            __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
            
            // Blend with different masks in each stage
            if (iter == 0) {
                mask = 0xAAAAAAAA;
            } else if (iter == 1) {
                mask = 0x55555555;
            } else {
                mask = 0x33333333;
            }
            
            vresult = _mm512_mask_blend_epi16(mask, va, vb);
            
            // Create arithmetic dependency
            vresult = _mm512_add_epi16(vresult, _mm512_set1_epi16(iter));
            
            // Force compiler to keep the operation
            asm volatile("" : "+v"(vresult) : : "memory");
        }
        
        // Data-dependent blend
        __mmask32 dyn_mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((src16_a[i] % 5) < 2) {
                dyn_mask |= (1U << i);
            }
        }
        vresult = _mm512_mask_blend_epi16(dyn_mask, va, vb);
        
        _mm512_storeu_si512((__m512i*)dst16, vresult);
        total_checksum += checksum_16(dst16, ARRAY_SIZE);
    }
    
    // ==================== V32HFmode (32x half-float) ====================
    {
        // Load as integers and cast
        __m512i va_i = _mm512_loadu_si512((__m512i*)srch_a);
        __m512i vb_i = _mm512_loadu_si512((__m512i*)srch_b);
        
        // Cast to half-precision vectors if supported
        #ifdef __AVX512FP16__
        __m512h va = _mm512_castsi512_ph(va_i);
        __m512h vb = _mm512_castsi512_ph(vb_i);
        __m512h vresult_h = _mm512_setzero_ph();
        
        // Blend operations
        __mmask32 mask = 0xAAAAAAAA;
        vresult_h = _mm512_mask_blend_ph(mask, va, vb);
        
        // Store back
        __m512i result_i = _mm512_castph_si512(vresult_h);
        _mm512_storeu_si512((__m512i*)dsth, result_i);
        #else
        // Fallback: use integer blend with same bit pattern
        __mmask32 mask = 0xAAAAAAAA;
        __m512i vresult = _mm512_mask_blend_epi16(mask, va_i, vb_i);
        _mm512_storeu_si512((__m512i*)dsth, vresult);
        #endif
        
        total_checksum += checksum_16(dsth, ARRAY_SIZE);
    }
#endif  // __AVX512BW__

#ifdef __AVX512F__
    // ==================== V16SImode (16x int) ====================
    {
        __m512i va = _mm512_loadu_si512((__m512i*)src32_a);
        __m512i vb = _mm512_loadu_si512((__m512i*)src32_b);
        
        __m512i vresult = _mm512_setzero_si512();
        
        // Chain of blend operations
        for (int stage = 0; stage < 2; stage++) {
            __mmask16 mask;
            if (stage == 0) {
                mask = 0xAAAA;  // Alternating
            } else {
                mask = 0x5555;  // Opposite alternating
            }
            
            vresult = _mm512_mask_blend_epi32(mask, va, vb);
            
            // Prevent optimization
            asm volatile("" : "+v"(vresult) : : "memory");
        }
        
        // Final blend with computation
        __mmask16 final_mask = 0;
        for (int i = 0; i < 16; i++) {
            if ((i % 4) == 0) {
                final_mask |= (1U << i);
            }
        }
        vresult = _mm512_mask_blend_epi32(final_mask, va, vb);
        
        _mm512_storeu_si512((__m512i*)dst32, vresult);
        total_checksum += checksum_32(dst32, ARRAY_SIZE);
    }
    
    // ==================== V8DImode (8x long) ====================
    {
        __m512i va = _mm512_loadu_si512((__m512i*)src64_a);
        __m512i vb = _mm512_loadu_si512((__m512i*)src64_b);
        
        __m512i vresult = _mm512_setzero_si512();
        
        // Multiple blend stages
        __mmask8 mask1 = 0xAA;  // 10101010
        __mmask8 mask2 = 0x55;  // 01010101
        
        vresult = _mm512_mask_blend_epi64(mask1, va, vb);
        vresult = _mm512_mask_blend_epi64(mask2, vresult, va);
        
        // Force materialization
        asm volatile("" : "+v"(vresult) : : "memory");
        
        _mm512_storeu_si512((__m512i*)dst64, vresult);
        total_checksum += checksum_64(dst64, ARRAY_SIZE);
    }
    
    // ==================== V16SFmode (16x float) ====================
    {
        __m512 va = _mm512_loadu_ps(srcf_a);
        __m512 vb = _mm512_loadu_ps(srcf_b);
        
        __m512 vresult = _mm512_setzero_ps();
        
        // Pipeline with multiple blend operations
        for (int i = 0; i < 3; i++) {
            __mmask16 mask;
            switch (i) {
                case 0: mask = 0xAAAA; break;
                case 1: mask = 0x5555; break;
                case 2: mask = 0x3333; break;
            }
            
            vresult = _mm512_mask_blend_ps(mask, va, vb);
            
            // Add some arithmetic
            vresult = _mm512_add_ps(vresult, _mm512_set1_ps(0.5f));
            
            asm volatile("" : "+v"(vresult) : : "memory");
        }
        
        _mm512_storeu_ps(dstf, vresult);
        total_checksum += (uint64_t)checksum_f(dstf, ARRAY_SIZE);
    }
    
    // ==================== V8DFmode (8x double) ====================
    {
        __m512d va = _mm512_loadu_pd(srcd_a);
        __m512d vb = _mm512_loadu_pd(srcd_b);
        
        __m512d vresult = _mm512_setzero_pd();
        
        // Multi-stage blend
        __mmask8 mask = 0xAA;
        vresult = _mm512_mask_blend_pd(mask, va, vb);
        
        // Second blend with different mask
        mask = 0x55;
        vresult = _mm512_mask_blend_pd(mask, vresult, va);
        
        // Arithmetic operation
        vresult = _mm512_add_pd(vresult, _mm512_set1_pd(1.0));
        
        asm volatile("" : "+v"(vresult) : : "memory");
        
        _mm512_storeu_pd(dstd, vresult);
        total_checksum += (uint64_t)checksum_d(dstd, ARRAY_SIZE);
    }
#endif  // __AVX512F__

#ifdef __AVX512BF16__
    // ==================== V32BFmode (32x brain-float) ====================
    {
        // Load as integers
        __m512i va_i = _mm512_loadu_si512((__m512i*)srcb_a);
        __m512i vb_i = _mm512_loadu_si512((__m512i*)srcb_b);
        
        #ifdef __AVX512BF16__
        // Cast to brain-float vectors
        __m512bh va = _mm512_castsi512_pbh(va_i);
        __m512bh vb = _mm512_castsi512_pbh(vb_i);
        
        // Blend operation
        __mmask32 mask = 0xAAAAAAAA;
        __m512bh vresult = _mm512_mask_blend_epi16(mask, va, vb);
        
        // Cast back to store
        __m512i result_i = _mm512_castpbh_si512(vresult);
        _mm512_storeu_si512((__m512i*)dstb, result_i);
        #else
        // Fallback using integer blend
        __mmask32 mask = 0xAAAAAAAA;
        __m512i vresult = _mm512_mask_blend_epi16(mask, va_i, vb_i);
        _mm512_storeu_si512((__m512i*)dstb, vresult);
        #endif
        
        total_checksum += checksum_16(dstb, ARRAY_SIZE);
    }
#endif  // __AVX512BF16__

    // Print final checksum to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
