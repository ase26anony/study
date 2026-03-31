Here's a C++ program designed to trigger the specific AVX-512 blend expansion cases in i386-expand.cc:

```cpp
#include <immintrin.h>
#include <iostream>
#include <cstdint>

// Helper functions for each blend mode
__m512i blend_64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

__m512i blend_32hi(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}

__m512i blend_16si(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

__m512i blend_8di(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

__m512 blend_16sf(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

__m512d blend_8df(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
__m512h blend_32hf(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

// Function with loop and conditional contexts
void process_blends_in_loop(int iterations) {
    // Initialize test data
    __m512i vec_i8_a = _mm512_set1_epi8(1);
    __m512i vec_i8_b = _mm512_set1_epi8(2);
    __m512i vec_i16_a = _mm512_set1_epi16(10);
    __m512i vec_i16_b = _mm512_set1_epi16(20);
    __m512i vec_i32_a = _mm512_set1_epi32(100);
    __m512i vec_i32_b = _mm512_set1_epi32(200);
    __m512i vec_i64_a = _mm512_set1_epi64(1000);
    __m512i vec_i64_b = _mm512_set1_epi64(2000);
    
    __m512 vec_f32_a = _mm512_set1_ps(1.5f);
    __m512 vec_f32_b = _mm512_set1_ps(2.5f);
    __m512d vec_f64_a = _mm512_set1_pd(3.14159);
    __m512d vec_f64_b = _mm512_set1_pd(2.71828);
    
    // Create masks using comparisons
    __m512i cmp_vec1 = _mm512_set1_epi8(1);
    __m512i cmp_vec2 = _mm512_set1_epi8(2);
    
    __mmask64 mask64 = _mm512_cmpeq_epi8_mask(cmp_vec1, cmp_vec2);
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(cmp_vec1, cmp_vec2);
    __mmask16 mask16 = _mm512_cmpeq_epi32_mask(cmp_vec1, cmp_vec2);
    __mmask8 mask8 = _mm512_cmpeq_epi64_mask(cmp_vec1, cmp_vec2);
    
    // Create masks for floating point
    __m512 cmp_f32_1 = _mm512_set1_ps(1.0f);
    __m512 cmp_f32_2 = _mm512_set1_ps(2.0f);
    __mmask16 mask_f32 = _mm512_cmp_ps_mask(cmp_f32_1, cmp_f32_2, _CMP_EQ_OQ);
    
    __m512d cmp_f64_1 = _mm512_set1_pd(1.0);
    __m512d cmp_f64_2 = _mm512_set1_pd(2.0);
    __mmask8 mask_f64 = _mm512_cmp_pd_mask(cmp_f64_1, cmp_f64_2, _CMP_EQ_OQ);
    
    // Combine masks using logical operations
    __mmask32 combined_mask32 = _kor_mask32(mask32, 0xAAAAAAAA);
    __mmask16 combined_mask16 = _kor_mask16(mask16, 0xAAAA);
    __mmask8 combined_mask8 = _kor_mask8(mask8, 0xAA);
    
    // Sequence of dependent blend operations
    __m512i result_i8 = blend_64qi(vec_i8_a, vec_i8_b, mask64);
    __m512i result_i16 = blend_32hi(vec_i16_a, vec_i16_b, combined_mask32);
    
    // Use results in subsequent blends (creating dependency chain)
    for (int i = 0; i < iterations; i++) {
        // Conditional blending based on loop index
        if (i % 2 == 0) {
            result_i8 = blend_64qi(result_i8, vec_i8_b, mask64 ^ 0xFF);
            result_i16 = blend_32hi(result_i16, vec_i16_b, combined_mask32 ^ 0xFFFF);
        } else {
            result_i8 = blend_64qi(vec_i8_a, result_i8, mask64);
            result_i16 = blend_32hi(vec_i16_a, result_i16, combined_mask32);
        }
        
        // Switch-like behavior for different modes
        switch (i % 4) {
            case 0:
                result_i8 = blend_64qi(result_i8, vec_i8_b, 0xFFFFFFFFFFFFFFFF);
                break;
            case 1:
                result_i16 = blend_32hi(result_i16, vec_i16_b, 0xFFFFFFFF);
                break;
            case 2: {
                __m512i temp = blend_16si(vec_i32_a, vec_i32_b, mask16);
                result_i8 = _mm512_add_epi8(result_i8, _mm512_and_si512(temp, _mm512_set1_epi8(1)));
                break;
            }
            case 3: {
                __m512i temp = blend_8di(vec_i64_a, vec_i64_b, mask8);
                result_i16 = _mm512_add_epi16(result_i16, _mm512_and_si512(temp, _mm512_set1_epi16(1)));
                break;
            }
        }
    }
    
    // Floating point blends in the loop
    __m512 result_f32 = blend_16sf(vec_f32_a, vec_f32_b, mask_f32);
    __m512d result_f64 = blend_8df(vec_f64_a, vec_f64_b, mask_f64);
    
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            result_f32 = blend_16sf(result_f32, vec_f32_b, mask_f32 ^ 0xFFFF);
        } else if (i % 3 == 1) {
            result_f64 = blend_8df(result_f64, vec_f64_b, combined_mask8);
        } else {
            // Cross-type dependency
            __m512i temp_i32 = blend_16si(vec_i32_a, vec_i32_b, mask16);
            result_f32 = _mm512_add_ps(result_f32, _mm512_cvtepi32_ps(temp_i32));
        }
    }
    
    // Prevent dead code elimination with reductions
    alignas(64) int8_t i8_result[64];
    alignas(64) int16_t i16_result[32];
    alignas(64) float f32_result[16];
    alignas(64) double f64_result[8];
    
    _mm512_store_si512(i8_result, result_i8);
    _mm512_store_si512(i16_result, result_i16);
    _mm512_store_ps(f32_result, result_f32);
    _mm512_store_pd(f64_result, result_f64);
    
    // Simple verification sums
    int64_t sum_i8 = 0;
    int64_t sum_i16 = 0;
    float sum_f32 = 0.0f;
    double sum_f64 = 0.0;
    
    for (int i = 0; i < 64; i++) sum_i8 += i8_result[i];
    for (int i = 0; i < 32; i++) sum_i16 += i16_result[i];
    for (int i = 0; i < 16; i++) sum_f32 += f32_result[i];
    for (int i = 0; i < 8; i++) sum_f64 += f64_result[i];
    
    std::cout << "Verification sums:" << std::endl;
    std::cout << "  int8: " << sum_i8 << std::endl;
    std::cout << "  int16: " << sum_i16 << std::endl;
    std::cout << "  float32: " << sum_f32 << std::endl;
    std::cout << "  float64: " << sum_f64 << std::endl;
}

// Specialized functions for each blend mode
void specialized_blend_functions() {
    // V64QImode
    {
        __m512i a = _mm512_setr_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        __m512i b = _mm512_set1_epi8(100);
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
        __m512i result = blend_64qi(a, b, mask);
        alignas(64) int8_t temp[64];
        _mm512_store_si512(temp, result);
    }
    
    // V32HImode
    {
        __m512i a = _mm512_setr_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512i b = _mm512_set1_epi16(200);
        __mmask32 mask = 0xAAAAAAAA;
        __m512i result = blend_32hi(a, b, mask);
    }
    
    // V16SImode
    {
        __m512i a = _mm512_setr_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        __m512i b = _mm512_set1_epi32(300);
        __mmask16 mask = 0xAAAA;
        __m512i result = blend_16si(a, b, mask);
    }
    
    // V8DImode
    {
        __m512i a = _mm512_setr_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set1_epi64(400);
        __mmask8 mask = 0xAA;
        __m512i result = blend_8di(a, b, mask);
    }
    
    // V16SFmode
    {
        __m512 a = _mm512_setr_ps(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
        );
        __m512 b = _mm512_set1_ps(500.0f);
        __mmask16 mask = 0xAAAA;
        __m512 result = blend_16sf(a, b, mask);
    }
    
    // V8DFmode
    {
        __m512d a = _mm512_setr_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        __m512d b = _mm512_set1_pd(600.0);
        __mmask8 mask = 0xAA;
        __m512d result = blend_8df(a, b, mask);
    }
    
    // V32HFmode (if supported)
    #ifdef __AVX512FP16__
    {
        __m512h a = _mm512_set1_ph(1.0f);
        __m512h b = _mm512_set1_ph(2.0f);
        __mmask32 mask = 0xAAAAAAAA;
        __m512h result = blend_32hf(a, b, mask);
    }
    #endif
}

int main() {
    std::cout << "Testing AVX-512 blend operations..." << std::endl;
    
    // Test with different loop iterations
    process_blends_in_loop(10);
    
    // Call specialized functions
    specialized_blend_functions();
    
    // Additional test with immediate masks
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(2);
        __m512i c = _mm512_mask_blend_epi8(0xFFFFFFFFFFFFFFFF, a, b);
        
        __m512 fa = _mm512_set1_ps(1.0f);
        __m512 fb = _mm512_set1_ps(2.0f);
        __m512 fc = _mm512_mask_blend_ps(0xFFFF, fa, fb);
        
        alignas(64) int8_t temp_i8[64];
        alignas(64) float temp_f32[16];
        _mm512_store_si512(temp_i8, c);
        _mm512_store_ps(temp_f32, fc);
        
        int sum_i8 = 0;
        float sum_f32 = 0.0f;
        for (int i = 0; i < 64; i++) sum_i8 += temp_i8[i];
        for (int i = 0; i < 16; i++) sum_f32 += temp_f32[i];
        
        std::cout << "Immediate mask test sums:" << std::endl;
        std::cout << "  int8: " << sum_i8 << std::endl;
        std::cout << "  float32: " << sum_f32 << std::endl;
    }
    
    std::cout << "Test completed." << std::endl;
    return 0;
}
```

This program is designed to trigger all the specific AVX-512 blend expansion cases:

1. **Covers all target modes**: V64QImode, V32HImode, V16SImode, V8DImode, V16SFmode, V8DFmode, and optionally V32HFmode.

2. **Uses various mask sources**:
   - Immediate integer masks (e.g., `0xFFFFFFFFFFFFFFFF`)
   - Mask variables from comparisons (`_mm512_cmpeq_*_mask`)
   - Combined masks using logical operations (`_kor_mask*`)

3. **Includes loop and conditional contexts**:
   - Blend operations inside loops with varying trip counts
   - Conditional branches (`if`, `switch`) affecting blend selection
   - Different execution paths based on loop index

4. **Creates dependent blend sequences**:
   - Results from one blend used as inputs to another
   - Cross-type dependencies (integer to float conversions)
   - Multiple blend operations in sequence

5. **Uses function specialization**:
   - Separate helper functions for each blend mode
   - Specialized test functions for each vector mode

6. **Prevents dead code elimination**:
   - Stores results to memory
   - Computes verification sums
   - Prints results to ensure execution

**Compilation recommendations**:
- For AVX-512F and AVX-512BW: `g++ -O3 -mavx512f -mavx512bw -mavx512vl -march=native test.cpp -o test`
- For AVX-512FP16 support: `g++ -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native test.cpp -o test`
- For aggressive optimization: `g++ -Ofast -mavx512f -mavx512bw -mavx512dq -mavx512vl -mavx512fp16 -march=native -funroll-loops test.cpp -o test`

The program should trigger the specific code generation functions (`gen_avx512*_blendmv*`) in i386-expand.cc when compiled with appropriate optimization flags.
