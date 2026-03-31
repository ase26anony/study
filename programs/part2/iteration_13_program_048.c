#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Initialize arrays with distinct patterns
#define ARRAY_SIZE 64

// Byte arrays for V64QImode
static uint8_t src1_bytes[ARRAY_SIZE];
static uint8_t src2_bytes[ARRAY_SIZE];
static uint8_t dst_bytes[ARRAY_SIZE];

// Short arrays for V32HImode
static uint16_t src1_shorts[ARRAY_SIZE];
static uint16_t src2_shorts[ARRAY_SIZE];
static uint16_t dst_shorts[ARRAY_SIZE];

// Int arrays for V16SImode
static int32_t src1_ints[ARRAY_SIZE];
static int32_t src2_ints[ARRAY_SIZE];
static int32_t dst_ints[ARRAY_SIZE];

// Long arrays for V8DImode
static int64_t src1_longs[ARRAY_SIZE];
static int64_t src2_longs[ARRAY_SIZE];
static int64_t dst_longs[ARRAY_SIZE];

// Float arrays for V16SFmode
static float src1_floats[ARRAY_SIZE];
static float src2_floats[ARRAY_SIZE];
static float dst_floats[ARRAY_SIZE];

// Double arrays for V8DFmode
static double src1_doubles[ARRAY_SIZE];
static double src2_doubles[ARRAY_SIZE];
static double dst_doubles[ARRAY_SIZE];

// Half-precision arrays for V32HFmode
static uint16_t src1_half[ARRAY_SIZE * 2];  // 32 half floats
static uint16_t src2_half[ARRAY_SIZE * 2];
static uint16_t dst_half[ARRAY_SIZE * 2];

// Brain-float arrays for V32BFmode
static uint16_t src1_bfloat[ARRAY_SIZE * 2];  // 32 bfloat16
static uint16_t src2_bfloat[ARRAY_SIZE * 2];
static uint16_t dst_bfloat[ARRAY_SIZE * 2];

// Initialize all arrays with distinct patterns
void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Byte pattern: alternating 0xAA and 0x55
        src1_bytes[i] = 0xAA;
        src2_bytes[i] = 0x55;
        
        // Short pattern: incrementing values
        src1_shorts[i] = i * 2;
        src2_shorts[i] = i * 3;
        
        // Int pattern: squares
        src1_ints[i] = i * i;
        src2_ints[i] = i * i * 2;
        
        // Long pattern: cubes
        src1_longs[i] = (int64_t)i * i * i;
        src2_longs[i] = (int64_t)i * i * i * 2;
        
        // Float pattern: sine-like
        src1_floats[i] = i * 0.1f;
        src2_floats[i] = i * 0.2f;
        
        // Double pattern: cosine-like
        src1_doubles[i] = i * 0.05;
        src2_doubles[i] = i * 0.15;
    }
    
    // Half-precision and brain-float patterns
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        src1_half[i] = i * 0x100;
        src2_half[i] = i * 0x200;
        src1_bfloat[i] = i * 0x300;
        src2_bfloat[i] = i * 0x400;
    }
}

// Multi-stage processing pipeline with blend operations
#ifdef __AVX512BW__
void test_avx512bw_blends() {
    printf("Testing AVX512BW blend operations...\n");
    
    // V64QImode blend - using _mm512_mask_blend_epi8
    {
        __m512i v1 = _mm512_loadu_si512((__m512i*)src1_bytes);
        __m512i v2 = _mm512_loadu_si512((__m512i*)src2_bytes);
        
        // First blend with constant mask
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        __m512i result1 = _mm512_mask_blend_epi8(mask1, v1, v2);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask64 loop_mask = (i % 3) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
            __m512i temp = _mm512_mask_blend_epi8(loop_mask, v1, v2);
            accum = _mm512_add_epi8(accum, temp);
            
            // Force compiler to keep the operation
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        // Multi-stage pipeline
        __m512i intermediate = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, result1, accum);
        __m512i final_result = _mm512_mask_blend_epi8(0xF0F0F0F0F0F0F0F0ULL, intermediate, v1);
        
        _mm512_storeu_si512((__m512i*)dst_bytes, final_result);
    }
    
    // V32HImode blend - using _mm512_mask_blend_epi16
    {
        __m512i v1 = _mm512_loadu_si512((__m512i*)src1_shorts);
        __m512i v2 = _mm512_loadu_si512((__m512i*)src2_shorts);
        
        // Constant mask blend
        __mmask32 mask1 = 0xAAAAAAAA;
        __m512i result1 = _mm512_mask_blend_epi16(mask1, v1, v2);
        
        // Loop with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 16; i++) {
            __mmask32 loop_mask = (i % 4) ? 0xFFFFFFFF : 0x55555555;
            __m512i temp = _mm512_mask_blend_epi16(loop_mask, v1, v2);
            accum = _mm512_add_epi16(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        // Multi-stage processing
        __m512i intermediate = _mm512_mask_blend_epi16(0xCCCCCCCC, result1, accum);
        __m512i final_result = _mm512_mask_blend_epi16(0xF0F0F0F0, intermediate, v1);
        
        _mm512_storeu_si512((__m512i*)dst_shorts, final_result);
    }
    
    // V32HFmode blend - using half-precision
    #ifdef __AVX512FP16__
    {
        __m512h v1 = _mm512_loadu_ph((__m512h*)src1_half);
        __m512h v2 = _mm512_loadu_ph((__m512h*)src2_half);
        
        __mmask32 mask1 = 0xAAAAAAAA;
        __m512h result1 = _mm512_mask_blend_ph(mask1, v1, v2);
        
        // Loop-based processing
        __m512h accum = _mm512_setzero_ph();
        for (int i = 0; i < 8; i++) {
            __mmask32 loop_mask = (i % 2) ? 0xFFFFFFFF : 0x55555555;
            __m512h temp = _mm512_mask_blend_ph(loop_mask, v1, v2);
            accum = _mm512_add_ph(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        _mm512_storeu_ph((__m512h*)dst_half, result1);
    }
    #else
    // Fallback using integer operations with casting
    {
        __m512i v1 = _mm512_loadu_si512((__m512i*)src1_half);
        __m512i v2 = _mm512_loadu_si512((__m512i*)src2_half);
        
        __mmask32 mask1 = 0xAAAAAAAA;
        __m512i result1 = _mm512_mask_blend_epi16(mask1, v1, v2);
        
        _mm512_storeu_si512((__m512i*)dst_half, result1);
    }
    #endif
}
#endif

#ifdef __AVX512F__
void test_avx512f_blends() {
    printf("Testing AVX512F blend operations...\n");
    
    // V16SImode blend - using _mm512_mask_blend_epi32
    {
        __m512i v1 = _mm512_loadu_si512((__m512i*)src1_ints);
        __m512i v2 = _mm512_loadu_si512((__m512i*)src2_ints);
        
        __mmask16 mask1 = 0xAAAA;
        __m512i result1 = _mm512_mask_blend_epi32(mask1, v1, v2);
        
        // Loop with data-dependent masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask16 loop_mask = 0;
            for (int j = 0; j < 16; j++) {
                if ((i + j) % 3 == 0) {
                    loop_mask |= (1 << j);
                }
            }
            __m512i temp = _mm512_mask_blend_epi32(loop_mask, v1, v2);
            accum = _mm512_add_epi32(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        // Multi-stage pipeline
        __m512i intermediate = _mm512_mask_blend_epi32(0xCCCC, result1, accum);
        __m512i final_result = _mm512_mask_blend_epi32(0xF0F0, intermediate, v1);
        
        _mm512_storeu_si512((__m512i*)dst_ints, final_result);
    }
    
    // V8DImode blend - using _mm512_mask_blend_epi64
    {
        __m512i v1 = _mm512_loadu_si512((__m512i*)src1_longs);
        __m512i v2 = _mm512_loadu_si512((__m512i*)src2_longs);
        
        __mmask8 mask1 = 0xAA;
        __m512i result1 = _mm512_mask_blend_epi64(mask1, v1, v2);
        
        // Loop-based processing
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            __mmask8 loop_mask = (i % 2) ? 0xFF : 0x55;
            __m512i temp = _mm512_mask_blend_epi64(loop_mask, v1, v2);
            accum = _mm512_add_epi64(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        _mm512_storeu_si512((__m512i*)dst_longs, result1);
    }
    
    // V16SFmode blend - using _mm512_mask_blend_ps
    {
        __m512 v1 = _mm512_loadu_ps(src1_floats);
        __m512 v2 = _mm512_loadu_ps(src2_floats);
        
        __mmask16 mask1 = 0xAAAA;
        __m512 result1 = _mm512_mask_blend_ps(mask1, v1, v2);
        
        // Multi-stage processing pipeline
        __m512 accum = _mm512_setzero_ps();
        for (int i = 0; i < 4; i++) {
            __mmask16 loop_mask = 0;
            for (int j = 0; j < 16; j++) {
                if ((i * 4 + j) % 5 < 2) {
                    loop_mask |= (1 << j);
                }
            }
            __m512 temp = _mm512_mask_blend_ps(loop_mask, v1, v2);
            accum = _mm512_add_ps(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        __m512 intermediate = _mm512_mask_blend_ps(0xCCCC, result1, accum);
        __m512 final_result = _mm512_mask_blend_ps(0xF0F0, intermediate, v1);
        
        _mm512_storeu_ps(dst_floats, final_result);
    }
    
    // V8DFmode blend - using _mm512_mask_blend_pd
    {
        __m512d v1 = _mm512_loadu_pd(src1_doubles);
        __m512d v2 = _mm512_loadu_pd(src2_doubles);
        
        __mmask8 mask1 = 0xAA;
        __m512d result1 = _mm512_mask_blend_pd(mask1, v1, v2);
        
        // Loop with varying masks
        __m512d accum = _mm512_setzero_pd();
        for (int i = 0; i < 4; i++) {
            __mmask8 loop_mask = (i % 3) ? 0xFF : 0x55;
            __m512d temp = _mm512_mask_blend_pd(loop_mask, v1, v2);
            accum = _mm512_add_pd(accum, temp);
            
            asm volatile("" : "+x"(accum) : : "memory");
        }
        
        _mm512_storeu_pd(dst_doubles, result1);
    }
}
#endif

#ifdef __AVX512BF16__
void test_avx512bf16_blends() {
    printf("Testing AVX512BF16 blend operations...\n");
    
    // V32BFmode blend - using brain-float
    {
        __m512bh v1 = _mm512_loadu_bh((__m512bh*)src1_bfloat);
        __m512bh v2 = _mm512_loadu_bh((__m512bh*)src2_bfloat);
        
        __mmask32 mask1 = 0xAAAAAAAA;
        __m512bh result1 = _mm512_mask_blend_bh(mask1, v1, v2);
        
        // Loop-based processing
        __m512bh accum = _mm512_setzero_bh();
        for (int i = 0; i < 8; i++) {
            __mmask32 loop_mask = (i % 2) ? 0xFFFFFFFF : 0x55555555;
            __m512bh temp = _mm512_mask_blend_bh(loop_mask, v1, v2);
            
            // Use inline assembly to force materialization
            asm volatile("" : "+x"(temp) : : "memory");
            
            // Store intermediate result
            if (i == 0) {
                _mm512_storeu_bh((__m512bh*)dst_bfloat, temp);
            }
        }
    }
}
#endif

// Compute checksums to prevent optimization
uint64_t compute_checksum() {
    uint64_t checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += dst_bytes[i];
        checksum += dst_shorts[i];
        checksum += dst_ints[i];
        checksum += dst_longs[i];
        checksum += (uint64_t)dst_floats[i];
        checksum += (uint64_t)dst_doubles[i];
    }
    
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        checksum += dst_half[i];
        checksum += dst_bfloat[i];
    }
    
    return checksum;
}

int main() {
    // Initialize source arrays
    init_arrays();
    
    // Clear destination arrays
    memset(dst_bytes, 0, sizeof(dst_bytes));
    memset(dst_shorts, 0, sizeof(dst_shorts));
    memset(dst_ints, 0, sizeof(dst_ints));
    memset(dst_longs, 0, sizeof(dst_longs));
    memset(dst_floats, 0, sizeof(dst_floats));
    memset(dst_doubles, 0, sizeof(dst_doubles));
    memset(dst_half, 0, sizeof(dst_half));
    memset(dst_bfloat, 0, sizeof(dst_bfloat));
    
    // Execute blend tests based on available ISA extensions
    #ifdef __AVX512BW__
    test_avx512bw_blends();
    #endif
    
    #ifdef __AVX512F__
    test_avx512f_blends();
    #endif
    
    #ifdef __AVX512BF16__
    test_avx512bf16_blends();
    #endif
    
    // Compute and print checksum to prevent dead code elimination
    uint64_t checksum = compute_checksum();
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
