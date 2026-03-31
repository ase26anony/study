```cpp
// avx512_blend_coverage.cpp
// Compile with: g++ -O3 -mavx512f -mavx512bw -mavx512dq -mavx512vl -mavx512fp16 -mbfloat16 avx512_blend_coverage.cpp -o blend_test
// For GCC compatibility with bfloat16: may need -mavx512bf16

#include <immintrin.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

// Use volatile to prevent constant folding and ensure blend expansion
volatile int g_volatile_threshold = 42;

// ==================== Explicit Intrinsic Functions ====================

void blend_int8_explicit(int8_t* dest, const int8_t* src1, const int8_t* src2, size_t n) {
    const size_t chunk_size = 64; // 512 bits / 8 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512i v1 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src1 + i));
        __m512i v2 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src2 + i));
        
        // Create mask: src1[i] > threshold (using volatile to prevent optimization)
        __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, _mm512_set1_epi8(g_volatile_threshold));
        
        __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + i), result);
    }
}

void blend_int16_explicit(int16_t* dest, const int16_t* src1, const int16_t* src2, size_t n) {
    const size_t chunk_size = 32; // 512 bits / 16 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512i v1 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src1 + i));
        __m512i v2 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src2 + i));
        
        __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(g_volatile_threshold));
        __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + i), result);
    }
}

void blend_int32_explicit(int32_t* dest, const int32_t* src1, const int32_t* src2, size_t n) {
    const size_t chunk_size = 16; // 512 bits / 32 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512i v1 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src1 + i));
        __m512i v2 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src2 + i));
        
        __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(g_volatile_threshold));
        __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + i), result);
    }
}

void blend_int64_explicit(int64_t* dest, const int64_t* src1, const int64_t* src2, size_t n) {
    const size_t chunk_size = 8; // 512 bits / 64 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512i v1 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src1 + i));
        __m512i v2 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src2 + i));
        
        __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, _mm512_set1_epi64(g_volatile_threshold));
        __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + i), result);
    }
}

void blend_float_explicit(float* dest, const float* src1, const float* src2, size_t n) {
    const size_t chunk_size = 16; // 512 bits / 32 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512 v1 = _mm512_loadu_ps(src1 + i);
        __m512 v2 = _mm512_loadu_ps(src2 + i);
        
        __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(g_volatile_threshold), _CMP_GT_OQ);
        __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
        _mm512_storeu_ps(dest + i, result);
    }
}

void blend_double_explicit(double* dest, const double* src1, const double* src2, size_t n) {
    const size_t chunk_size = 8; // 512 bits / 64 bits per element
    for (size_t i = 0; i < n; i += chunk_size) {
        __m512d v1 = _mm512_loadu_pd(src1 + i);
        __m512d v2 = _mm512_loadu_pd(src2 + i);
        
        __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(g_volatile_threshold), _CMP_GT_OQ);
        __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
        _mm512_storeu_pd(dest + i, result);
    }
}

// ==================== Auto-vectorized Functions ====================

#ifdef __AVX512FP16__
void blend_half_auto(_Float16* dest, const _Float16* src1, const _Float16* src2, size_t n) {
    // Rely on auto-vectorization with ternary operator
    for (size_t i = 0; i < n; ++i) {
        dest[i] = (src1[i] > g_volatile_threshold) ? src1[i] : src2[i];
    }
}
#endif

#ifdef __AVX512BF16__
void blend_bfloat16_auto(__bf16* dest, const __bf16* src1, const __bf16* src2, size_t n) {
    // Rely on auto-vectorization with ternary operator
    for (size_t i = 0; i < n; ++i) {
        dest[i] = (src1[i] > g_volatile_threshold) ? src1[i] : src2[i];
    }
}
#endif

// ==================== Template Metaprogramming ====================

template <int Mode>
void test_blend_template() {
    constexpr size_t ARRAY_SIZE = 128;
    
    if constexpr (Mode == 0) { // V64QImode
        alignas(64) int8_t src1[ARRAY_SIZE];
        alignas(64) int8_t src2[ARRAY_SIZE];
        alignas(64) int8_t dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<int8_t>(rand() % 100);
            src2[i] = static_cast<int8_t>(rand() % 100);
        }
        
        blend_int8_explicit(dest, src1, src2, ARRAY_SIZE);
        
    } else if constexpr (Mode == 1) { // V32HImode
        alignas(64) int16_t src1[ARRAY_SIZE];
        alignas(64) int16_t src2[ARRAY_SIZE];
        alignas(64) int16_t dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<int16_t>(rand() % 100);
            src2[i] = static_cast<int16_t>(rand() % 100);
        }
        
        blend_int16_explicit(dest, src1, src2, ARRAY_SIZE);
        
    } else if constexpr (Mode == 2) { // V32HFmode
#ifdef __AVX512FP16__
        alignas(64) _Float16 src1[ARRAY_SIZE];
        alignas(64) _Float16 src2[ARRAY_SIZE];
        alignas(64) _Float16 dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<_Float16>(rand() % 100);
            src2[i] = static_cast<_Float16>(rand() % 100);
        }
        
        blend_half_auto(dest, src1, src2, ARRAY_SIZE);
#endif
        
    } else if constexpr (Mode == 3) { // V32BFmode
#ifdef __AVX512BF16__
        alignas(64) __bf16 src1[ARRAY_SIZE];
        alignas(64) __bf16 src2[ARRAY_SIZE];
        alignas(64) __bf16 dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<__bf16>(rand() % 100);
            src2[i] = static_cast<__bf16>(rand() % 100);
        }
        
        blend_bfloat16_auto(dest, src1, src2, ARRAY_SIZE);
#endif
        
    } else if constexpr (Mode == 4) { // V16SImode
        alignas(64) int32_t src1[ARRAY_SIZE];
        alignas(64) int32_t src2[ARRAY_SIZE];
        alignas(64) int32_t dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<int32_t>(rand() % 100);
            src2[i] = static_cast<int32_t>(rand() % 100);
        }
        
        blend_int32_explicit(dest, src1, src2, ARRAY_SIZE);
        
    } else if constexpr (Mode == 5) { // V8DImode
        alignas(64) int64_t src1[ARRAY_SIZE];
        alignas(64) int64_t src2[ARRAY_SIZE];
        alignas(64) int64_t dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<int64_t>(rand() % 100);
            src2[i] = static_cast<int64_t>(rand() % 100);
        }
        
        blend_int64_explicit(dest, src1, src2, ARRAY_SIZE);
        
    } else if constexpr (Mode == 6) { // V8DFmode
        alignas(64) double src1[ARRAY_SIZE];
        alignas(64) double src2[ARRAY_SIZE];
        alignas(64) double dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<double>(rand() % 100);
            src2[i] = static_cast<double>(rand() % 100);
        }
        
        blend_double_explicit(dest, src1, src2, ARRAY_SIZE);
        
    } else if constexpr (Mode == 7) { // V16SFmode
        alignas(64) float src1[ARRAY_SIZE];
        alignas(64) float src2[ARRAY_SIZE];
        alignas(64) float dest[ARRAY_SIZE];
        
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            src1[i] = static_cast<float>(rand() % 100);
            src2[i] = static_cast<float>(rand() % 100);
        }
        
        blend_float_explicit(dest, src1, src2, ARRAY_SIZE);
    }
}

// Force instantiation of all template modes
template void test_blend_template<0>();
template void test_blend_template<1>();
template void test_blend_template<2>();
template void test_blend_template<3>();
template void test_blend_template<4>();
template void test_blend_template<5>();
template void test_blend_template<6>();
template void test_blend_template<7>();

// ==================== Mixed Data Type Functions ====================

void blend_all_types(volatile int selector) {
    constexpr size_t SIZE = 256;
    
    // Use selector to ensure all paths are considered
    switch (selector % 8) {
        case 0: {
            alignas(64) int8_t src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_int8_explicit(dest, src1, src2, SIZE);
            break;
        }
        case 1: {
            alignas(64) int16_t src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_int16_explicit(dest, src1, src2, SIZE);
            break;
        }
        case 2:
#ifdef __AVX512FP16__
            {
                alignas(64) _Float16 src1[SIZE], src2[SIZE], dest[SIZE];
                for (size_t i = 0; i < SIZE; ++i) {
                    src1[i] = rand() % 100;
                    src2[i] = rand() % 100;
                }
                blend_half_auto(dest, src1, src2, SIZE);
            }
#endif
            break;
        case 3:
#ifdef __AVX512BF16__
            {
                alignas(64) __bf16 src1[SIZE], src2[SIZE], dest[SIZE];
                for (size_t i = 0; i < SIZE; ++i) {
                    src1[i] = rand() % 100;
                    src2[i] = rand() % 100;
                }
                blend_bfloat16_auto(dest, src1, src2, SIZE);
            }
#endif
            break;
        case 4: {
            alignas(64) int32_t src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_int32_explicit(dest, src1, src2, SIZE);
            break;
        }
        case 5: {
            alignas(64) int64_t src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_int64_explicit(dest, src1, src2, SIZE);
            break;
        }
        case 6: {
            alignas(64) double src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_double_explicit(dest, src1, src2, SIZE);
            break;
        }
        case 7: {
            alignas(64) float src1[SIZE], src2[SIZE], dest[SIZE];
            for (size_t i = 0; i < SIZE; ++i) {
                src1[i] = rand() % 100;
                src2[i] = rand() % 100;
            }
            blend_float_explicit(dest, src1, src2, SIZE);
            break;
        }
    }
}

// ==================== Main Function ====================

int main() {
    srand(42);
    
    // Force compilation of all template instantiations
    constexpr int modes[] = {0, 1, 2, 3, 4, 5, 6, 7};
    volatile int unused = 0;
    for (int mode : modes) {
        unused += mode; // Prevent optimization
    }
    
    // Call mixed type functions with volatile selector
    volatile int selector = 0;
    for (int i = 0; i < 8; ++i) {
        selector = i;
        blend_all_types(selector);
    }
    
    // Explicitly call template functions to ensure they're used
    test_blend_template<0>();
    test_blend_template<1>();
    test_blend_template<2>();
    test_blend_template<3>();
    test_blend_template<4>();
    test_blend_template<5>();
    test_blend_template<6>();
    test_blend_template<7>();
    
    std::cout << "AVX-512 blend coverage test completed." << std::endl;
    return 0;
}
```

This program comprehensively targets the uncovered AVX-512 blend expansion code by:

1. **Explicit Intrinsic Usage**: Direct calls to `_mm512_mask_blend_*` intrinsics for all standard types (epi8, epi16, epi32, epi64, ps, pd).

2. **Auto-vectorization**: For half-precision (`_Float16`) and bfloat16 types, uses ternary operators in loops that should auto-vectorize to AVX-512 blend instructions.

3. **Volatile Control Flow**: Uses `g_volatile_threshold` and `selector` variables to prevent constant folding and ensure all code paths are generated.

4. **Template Metaprogramming**: The `test_blend_template<Mode>` function uses `if constexpr` to handle all 8 modes, with explicit template instantiations for each.

5. **Mixed Data Type Functions**: `blend_all_types()` contains a switch statement covering all blend modes, called with a volatile selector.

6. **Execution Flow**: The `main()` function calls all template instantiations, iterates through the mixed-type function with different selectors, and uses alignment hints to ensure optimal code generation.

**Compilation notes**:
- The `-mavx512fp16` flag enables half-precision support
- The `-mbfloat16` flag enables bfloat16 support (GCC 12+)
- For older GCC versions, you may need `-mavx512bf16` specifically for bfloat16
- The `-O3` flag ensures aggressive optimization and vectorization
- The alignment hints (`alignas(64)`) help generate optimal load/store instructions

This code should trigger the exact switch statement in i386-expand.cc for all 8 machine modes when compiled with AVX-512 support.
