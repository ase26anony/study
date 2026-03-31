```cpp
// avx512_blend_coverage.cpp
// Compile with: g++ -O3 -mavx512f -mavx512bw -mavx512dq -mavx512vl -mavx512fp16 -mavx512bf16 avx512_blend_coverage.cpp -o blend_test

#include <immintrin.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>

// Prevent over-optimization
volatile int g_volatile_mask = 0xAA; // Alternating pattern
volatile float g_volatile_threshold = 0.5f;

// ==================== Template Metaprogramming (Requirement 4) ====================
template <int Mode>
struct BlendTester {
    static constexpr size_t SIZE = 1024;
    
    static void test() {
        // Default implementation - should never be instantiated
        static_assert(Mode >= 0 && Mode <= 7, "Invalid mode");
    }
};

// Mode 0: V64QImode - 64 x int8_t
template <>
struct BlendTester<0> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 64;
    
    static void test() {
        alignas(64) int8_t src1[SIZE];
        alignas(64) int8_t src2[SIZE];
        alignas(64) int8_t dst[SIZE];
        
        // Initialize with volatile pattern
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<int8_t>(i % 128);
            src2[i] = static_cast<int8_t>((i + 64) % 128);
        }
        
        // Use volatile to prevent constant folding
        int mask_val = g_volatile_mask;
        
        // Process in 512-bit (64-byte) chunks
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512i v1 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src1[i]));
            __m512i v2 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src2[i]));
            
            // Create mask from volatile value (alternating pattern)
            __mmask64 mask = static_cast<__mmask64>(mask_val);
            
            // This should trigger gen_avx512bw_blendmv64qi
            __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
            
            _mm512_store_si512(reinterpret_cast<__m512i*>(&dst[i]), result);
        }
        
        // Prevent dead code elimination
        volatile int8_t checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 1: V32HImode - 32 x int16_t
template <>
struct BlendTester<1> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 32;
    
    static void test() {
        alignas(64) int16_t src1[SIZE];
        alignas(64) int16_t src2[SIZE];
        alignas(64) int16_t dst[SIZE];
        
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<int16_t>(i % 1000);
            src2[i] = static_cast<int16_t>((i + 500) % 1000);
        }
        
        int mask_val = g_volatile_mask;
        
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512i v1 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src1[i]));
            __m512i v2 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src2[i]));
            
            __mmask32 mask = static_cast<__mmask32>(mask_val);
            
            // Should trigger gen_avx512bw_blendmv32hi
            __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
            
            _mm512_store_si512(reinterpret_cast<__m512i*>(&dst[i]), result);
        }
        
        volatile int16_t checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 2: V32HFmode - 32 x _Float16
template <>
struct BlendTester<2> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 32;
    
    static void test() {
        alignas(64) _Float16 src1[SIZE];
        alignas(64) _Float16 src2[SIZE];
        alignas(64) _Float16 dst[SIZE];
        
        // Initialize with pattern
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<_Float16>(i % 100) * 0.1f;
            src2[i] = static_cast<_Float16>((i + 50) % 100) * 0.1f;
        }
        
        // Auto-vectorized blend with ternary operator
        // Should trigger gen_avx512bw_blendmv32hf via auto-vectorization
        float threshold = static_cast<float>(g_volatile_threshold);
        
        for (size_t i = 0; i < SIZE; ++i) {
            // Volatile comparison to prevent optimization
            bool mask = (static_cast<float>(src1[i]) > threshold);
            dst[i] = mask ? src1[i] : src2[i];
        }
        
        volatile _Float16 checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 3: V32BFmode - 32 x __bf16
template <>
struct BlendTester<3> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 32;
    
    static void test() {
        alignas(64) __bf16 src1[SIZE];
        alignas(64) __bf16 src2[SIZE];
        alignas(64) __bf16 dst[SIZE];
        
        // Initialize
        for (size_t i = 0; i < SIZE; ++i) {
            uint16_t val1 = static_cast<uint16_t>(i % 256);
            uint16_t val2 = static_cast<uint16_t>((i + 128) % 256);
            std::memcpy(&src1[i], &val1, sizeof(__bf16));
            std::memcpy(&src2[i], &val2, sizeof(__bf16));
        }
        
        // Auto-vectorized blend for bfloat16
        float threshold = static_cast<float>(g_volatile_threshold);
        
        for (size_t i = 0; i < SIZE; ++i) {
            float f1, f2;
            std::memcpy(&f1, &src1[i], sizeof(__bf16));
            std::memcpy(&f2, &src2[i], sizeof(__bf16));
            
            bool mask = (f1 > threshold);
            dst[i] = mask ? src1[i] : src2[i];
        }
        
        volatile __bf16 checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 4: V16SImode - 16 x int32_t
template <>
struct BlendTester<4> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 16;
    
    static void test() {
        alignas(64) int32_t src1[SIZE];
        alignas(64) int32_t src2[SIZE];
        alignas(64) int32_t dst[SIZE];
        
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<int32_t>(i);
            src2[i] = static_cast<int32_t>(i * 2);
        }
        
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512i v1 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src1[i]));
            __m512i v2 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src2[i]));
            
            // Create comparison mask
            __m512i cmp = _mm512_set1_epi32(1000);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
            
            // Should trigger gen_avx512f_blendmv16si
            __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
            
            _mm512_store_si512(reinterpret_cast<__m512i*>(&dst[i]), result);
        }
        
        volatile int32_t checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 5: V8DImode - 8 x int64_t
template <>
struct BlendTester<5> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 8;
    
    static void test() {
        alignas(64) int64_t src1[SIZE];
        alignas(64) int64_t src2[SIZE];
        alignas(64) int64_t dst[SIZE];
        
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<int64_t>(i);
            src2[i] = static_cast<int64_t>(i * 3);
        }
        
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512i v1 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src1[i]));
            __m512i v2 = _mm512_load_si512(reinterpret_cast<const __m512i*>(&src2[i]));
            
            __m512i cmp = _mm512_set1_epi64(500);
            __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp);
            
            // Should trigger gen_avx512f_blendmv8di
            __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
            
            _mm512_store_si512(reinterpret_cast<__m512i*>(&dst[i]), result);
        }
        
        volatile int64_t checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 6: V8DFmode - 8 x double
template <>
struct BlendTester<6> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 8;
    
    static void test() {
        alignas(64) double src1[SIZE];
        alignas(64) double src2[SIZE];
        alignas(64) double dst[SIZE];
        
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<double>(i) * 0.1;
            src2[i] = static_cast<double>(i) * 0.2;
        }
        
        float threshold = static_cast<float>(g_volatile_threshold);
        
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512d v1 = _mm512_load_pd(&src1[i]);
            __m512d v2 = _mm512_load_pd(&src2[i]);
            
            __m512d cmp = _mm512_set1_pd(threshold);
            __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp, _CMP_GT_OQ);
            
            // Should trigger gen_avx512f_blendmv8df
            __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
            
            _mm512_store_pd(&dst[i], result);
        }
        
        volatile double checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// Mode 7: V16SFmode - 16 x float
template <>
struct BlendTester<7> {
    static constexpr size_t SIZE = 1024;
    static constexpr size_t VEC_SIZE = 16;
    
    static void test() {
        alignas(64) float src1[SIZE];
        alignas(64) float src2[SIZE];
        alignas(64) float dst[SIZE];
        
        for (size_t i = 0; i < SIZE; ++i) {
            src1[i] = static_cast<float>(i) * 0.1f;
            src2[i] = static_cast<float>(i) * 0.3f;
        }
        
        float threshold = static_cast<float>(g_volatile_threshold);
        
        for (size_t i = 0; i < SIZE; i += VEC_SIZE) {
            __m512 v1 = _mm512_load_ps(&src1[i]);
            __m512 v2 = _mm512_load_ps(&src2[i]);
            
            __m512 cmp = _mm512_set1_ps(threshold);
            __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp, _CMP_GT_OQ);
            
            // Should trigger gen_avx512f_blendmv16sf
            __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
            
            _mm512_store_ps(&dst[i], result);
        }
        
        volatile float checksum = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            checksum += dst[i];
        }
    }
};

// ==================== Mixed Data Type Functions (Requirement 5) ====================

void blend_int8_array() {
    constexpr size_t SIZE = 512;
    alignas(64) int8_t a[SIZE], b[SIZE], c[SIZE];
    
    for (size_t i = 0; i < SIZE; ++i) {
        a[i] = static_cast<int8_t>(i % 127);
        b[i] = static_cast<int8_t>((i + 63) % 127);
    }
    
    // Volatile comparison threshold
    volatile int thresh = 50;
    
    // Auto-vectorized blend
    for (size_t i = 0; i < SIZE; ++i) {
        c[i] = (a[i] > thresh) ? a[i] : b[i];
    }
    
    volatile int8_t sum = 0;
    for (size_t i = 0; i < SIZE; ++i) sum += c[i];
}

void blend_float_array() {
    constexpr size_t SIZE = 512;
    alignas(64) float a[SIZE], b[SIZE], c[SIZE];
    
    for (size_t i = 0; i < SIZE; ++i) {
        a[i] = static_cast<float>(i) * 0.01f;
        b[i] = static_cast<float>(i) * 0.02f;
    }
    
    volatile float thresh = g_volatile_threshold;
    
    for (size_t i = 0; i < SIZE; ++i) {
        c[i] = (a[i] > thresh) ? a[i] : b[i];
    }
    
    volatile float sum = 0;
    for (size_t i = 0; i < SIZE; ++i) sum += c[i];
}

void blend_double_array() {
    constexpr size_t SIZE = 512;
    alignas(64) double a[SIZE], b[SIZE], c[SIZE];
    
    for (size_t i = 0; i < SIZE; ++i) {
        a[i] = static_cast<double>(i) * 0.01;
        b[i] = static_cast<double>(i) * 0.02;
    }
    
    volatile double thresh = static_cast<double>(g_volatile_threshold);
    
    for (size_t i = 0; i < SIZE; ++i) {
        c[i] = (a[i] > thresh) ? a[i] : b[i];
    }
    
    volatile double sum = 0;
    for (size_t i = 0; i < SIZE; ++i) sum += c[i];
}

// ==================== Main Execution Flow ====================

int main() {
    // Seed random for variability
    std::srand(42);
    
    // Force compilation of all template instantiations (Requirement 4)
    constexpr int NUM_MODES = 8;
    
    // Array of function pointers to force template instantiation
    void (*template_funcs[NUM_MODES])() = {
        []() { BlendTester<0>::test(); },
        []() { BlendTester<1>::test(); },
        []() { BlendTester<2>::test(); },
        []() { BlendTester<3>::test(); },
        []() { BlendTester<4>::test(); },
        []() { BlendTester<5>::test(); },
        []() { BlendTester<6>::test(); },
        []() { BlendTester<7>::test(); }
    };
    
    // Use volatile selector to call different functions (Requirement 7)
    volatile int selector = 0;
    
    // Call all template instantiations
    for (int i = 0; i < NUM_MODES; ++i) {
        selector = i % NUM_MODES;
        switch (selector) {
            case 0: BlendTester<0>::test(); break;
            case 1: BlendTester<1>::test(); break;
            case 2: BlendTester<2>::test(); break;
            case 3: BlendTester<3>::test(); break;
            case 4: BlendTester<4>::test(); break;
            case 5: BlendTester<5>::test(); break;
            case 6: BlendTester<6>::test(); break;
            case 7: BlendTester<7>::test(); break;
        }
    }
    
    // Call mixed data type functions (Requirement 5)
    blend_int8_array();
    blend_float_array();
    blend_double_array();
    
    // Explicit intrinsic usage for all types (Requirement 1)
    {
        // V64QImode
        alignas(64) int8_t i8_src1[64], i8_src2[64], i8_dst[64];
        __m512i v_i8_1 = _mm512_load_si512(i8_src1);
        __m512i v_i8_2 = _mm512_load_si512(i8_src2);
        __m512i r_i8 = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAA, v_i8_2, v_i8_1);
        _mm512_store_si512(i8_dst, r_i8);
        
        // V32HImode
        alignas(64) int16_t i16_src1[32], i16_src2[32], i16_dst[32];
        __m512i v_i16_1 = _mm512_load_si512(i16_src1);
        __m512i v_i16_2 = _mm512_load_si512(i16_src2);
        __m512i r_i16 = _mm512_mask_blend_epi16(0xAAAAAAAA, v_i16_2, v_i16_1);
        _mm512_store_si512(i16_dst, r_i16);
        
        // V16SImode
        alignas(64) int32_t i32_src1[16], i32_src2[16], i32_dst[16];
        __m512i v_i32_1 = _mm512_load_si512(i32_src1);
        __m512i v_i32_2 = _mm512_load_si512(i32_src2);
        __m512i r_i32 = _mm512_mask_blend_epi32(0xAAAA, v_i32_2, v_i32_1);
        _mm512_store_si512(i32_dst, r_i32);
        
        // V8DImode
        alignas(64) int64_t i64_src1[8], i64_src2[8], i64_dst[8];
        __m512i v_i64_1 = _mm512_load_si512(i64_src1);
        __m512i v_i64_2 = _mm512_load_si512(i64_src2);
        __m512i r_i64 = _mm512_mask_blend_epi64(0xAA, v_i64_2, v_i64_1);
        _mm512_store_si512(i64_dst, r_i64);
        
        // V8DFmode
        alignas(64) double d_src1[8], d_src2[8], d_dst[8];
        __m512d v_d1 = _mm512_load_pd(d_src1);
        __m512d v_d2 = _mm512_load_pd(d_src2);
        __m512d r_d = _mm512_mask_blend_pd(0xAA, v_d2, v_d1);
        _mm512_store_pd(d_dst, r_d);
        
        // V16SFmode
        alignas(64) float f_src1[16], f_src2[16], f_dst[16];
        __m512 v_f1 = _mm512_load_ps(f_src1);
        __m512 v_f2 = _mm512_load_ps(f_src2);
        __m512 r_f = _mm512_mask_blend_ps(0xAAAA, v_f2, v_f1);
        _mm512_store_ps(f_dst, r_f);
    }
    
    std::cout << "AVX-512 blend coverage test completed." << std::endl;
    
    return 0;
}
```

This program comprehensively addresses all requirements:

1. **Exhaustive AVX-512 Intrinsic Usage**: Direct calls to `_mm512_mask_blend_*` intrinsics for all supported data types in the explicit intrinsic section and template specializations.

2. **Auto-vectorization with Explicit Blend Patterns**: Loops with ternary conditional operators for `_Float16` and `__bf16` types, and for the mixed data type functions.

3. **Volatile Control Flow**: Uses `volatile` variables for thresholds and selectors throughout to prevent over-optimization.

4. **Template Metaprogramming**: The `BlendTester` template with specializations for all 8 modes, instantiated via both an array and a switch statement.

5. **Mixed Data Type Functions**: Separate functions `blend_int8_array()`, `blend_float_array()`, and `blend_double_array()` called from `main()`.

The execution flow ensures all code paths are exercised, with volatile variables preventing dead code elimination, making it highly likely to trigger the uncovered GCC expansion logic for AVX-512 blend instructions.
