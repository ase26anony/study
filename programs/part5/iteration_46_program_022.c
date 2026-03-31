#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

// Simple PRNG for reproducible results
static uint32_t seed = 123456789;
static inline uint32_t prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Initialize arrays with pseudo-random data
static void init_arrays(__m256i* arr_i32, __m256i* arr_i16, __m256i* arr_i8,
                       __m256* arr_f32, __m256d* arr_f64,
                       __m256i* mask_arr, __m256i* blend_arr) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Initialize integer arrays
        int32_t i32_data[8];
        int16_t i16_data[16];
        int8_t i8_data[32];
        
        for (int j = 0; j < 8; j++) i32_data[j] = (int32_t)prng();
        for (int j = 0; j < 16; j++) i16_data[j] = (int16_t)prng();
        for (int j = 0; j < 32; j++) i8_data[j] = (int8_t)prng();
        
        arr_i32[i] = _mm256_loadu_si256((const __m256i*)i32_data);
        arr_i16[i] = _mm256_loadu_si256((const __m256i*)i16_data);
        arr_i8[i] = _mm256_loadu_si256((const __m256i*)i8_data);
        
        // Initialize floating point arrays
        float f32_data[8];
        double f64_data[4];
        
        for (int j = 0; j < 8; j++) f32_data[j] = (float)prng() / (float)UINT32_MAX;
        for (int j = 0; j < 4; j++) f64_data[j] = (double)prng() / (double)UINT32_MAX;
        
        arr_f32[i] = _mm256_loadu_ps(f32_data);
        arr_f64[i] = _mm256_loadu_pd(f64_data);
        
        // Initialize mask and blend arrays
        mask_arr[i] = _mm256_set1_epi32(prng() & 0xFF);
        blend_arr[i] = _mm256_set1_epi32(prng() & 0x1);
    }
}

// Complex inline assembly with many operands - targeting 11 arguments
static inline __m256i complex_asm_11_args(
    __m256i a, __m256i b, __m256i c, __m256i d,
    __m256i e, __m256i f, __m256i g, __m256i h,
    int imm1, int imm2, int imm3) {
    
    __m256i result;
    
    // Extended asm with 11 input operands
    asm volatile (
        // Complex operation with many operands
        "vpaddd %0, %1, %2\n\t"
        "vpslld %0, %0, %11\n\t"
        "vpaddd %0, %0, %3\n\t"
        "vpsrld %0, %0, %12\n\t"
        "vpaddd %0, %0, %4\n\t"
        "vpslld %0, %0, %13\n\t"
        "vpaddd %0, %0, %5\n\t"
        "vpsrld %0, %0, %11\n\t"
        "vpaddd %0, %0, %6\n\t"
        "vpslld %0, %0, %12\n\t"
        "vpaddd %0, %0, %7\n\t"
        "vpsrld %0, %0, %13\n\t"
        "vpaddd %0, %0, %8\n\t"
        : "=x"(result)
        : "x"(a), "x"(b), "x"(c), "x"(d), 
          "x"(e), "x"(f), "x"(g), "x"(h),
          "i"(imm1), "i"(imm2), "i"(imm3)
        : "memory"
    );
    
    return result;
}

// Another inline assembly with 10 arguments
static inline __m256 complex_asm_10_args(
    __m256 a, __m256 b, __m256 c, __m256 d,
    __m256 e, __m256 f, __m256 g, __m256 h,
    int imm1, int imm2) {
    
    __m256 result;
    
    asm volatile (
        // Fused multiply-add chain with many operands
        "vfmadd132ps %0, %1, %2\n\t"
        "vfmadd231ps %0, %3, %4\n\t"
        "vfmadd132ps %0, %5, %6\n\t"
        "vfmadd231ps %0, %7, %8\n\t"
        "vpslld $%9, %0, %0\n\t"  // Using integer shift on float - will be converted
        "vpsrld $%10, %0, %0\n\t"
        : "=x"(result)
        : "0"(a), "x"(b), "x"(c), "x"(d),
          "x"(e), "x"(f), "x"(g), "x"(h),
          "i"(imm1), "i"(imm2)
        : "memory"
    );
    
    return result;
}

// Complex expression with many temporaries - forcing expander to generate many operands
static inline __m256i complex_multi_statement_expr(
    __m256i* base, int idx1, int idx2, int idx3, int idx4,
    int idx5, int idx6, int idx7, int idx8,
    int shift1, int shift2) {
    
    // Create many intermediate values
    __m256i t1 = _mm256_loadu_si256(&base[idx1]);
    __m256i t2 = _mm256_loadu_si256(&base[idx2]);
    __m256i t3 = _mm256_loadu_si256(&base[idx3]);
    __m256i t4 = _mm256_loadu_si256(&base[idx4]);
    __m256i t5 = _mm256_loadu_si256(&base[idx5]);
    __m256i t6 = _mm256_loadu_si256(&base[idx6]);
    __m256i t7 = _mm256_loadu_si256(&base[idx7]);
    __m256i t8 = _mm256_loadu_si256(&base[idx8]);
    
    // Complex chain of operations
    __m256i r1 = _mm256_add_epi32(t1, t2);
    __m256i r2 = _mm256_slli_epi32(r1, shift1);
    __m256i r3 = _mm256_add_epi32(r2, t3);
    __m256i r4 = _mm256_srli_epi32(r3, shift2);
    __m256i r5 = _mm256_add_epi32(r4, t4);
    __m256i r6 = _mm256_slli_epi32(r5, shift1);
    __m256i r7 = _mm256_add_epi32(r6, t5);
    __m256i r8 = _mm256_srli_epi32(r7, shift2);
    __m256i r9 = _mm256_add_epi32(r8, t6);
    __m256i r10 = _mm256_slli_epi32(r9, shift1);
    __m256i r11 = _mm256_add_epi32(r10, t7);
    __m256i r12 = _mm256_srli_epi32(r11, shift2);
    __m256i result = _mm256_add_epi32(r12, t8);
    
    return result;
}

// Function with target attribute - marked noinline to prevent inlining
__attribute__((target("avx2"), noinline))
static void test_many_args(
    __m256i* arr_i32, __m256i* arr_i16, __m256i* arr_i8,
    __m256* arr_f32, __m256d* arr_f64,
    __m256i* mask_arr, __m256i* blend_arr,
    __m256i* out_i32, __m256* out_f32) {
    
    volatile int iter_counter = 0;  // Prevent loop unrolling
    
    for (int i = 0; i < ITERATIONS; i++) {
        iter_counter++;
        
        int idx = i % (ARRAY_SIZE - 8);
        
        // Load multiple vectors
        __m256i v1 = _mm256_loadu_si256(&arr_i32[idx]);
        __m256i v2 = _mm256_loadu_si256(&arr_i32[idx + 1]);
        __m256i v3 = _mm256_loadu_si256(&arr_i32[idx + 2]);
        __m256i v4 = _mm256_loadu_si256(&arr_i32[idx + 3]);
        __m256i v5 = _mm256_loadu_si256(&arr_i32[idx + 4]);
        __m256i v6 = _mm256_loadu_si256(&arr_i32[idx + 5]);
        __m256i v7 = _mm256_loadu_si256(&arr_i32[idx + 6]);
        __m256i v8 = _mm256_loadu_si256(&arr_i32[idx + 7]);
        
        // Call inline assembly with 11 arguments
        __m256i result1 = complex_asm_11_args(v1, v2, v3, v4, v5, v6, v7, v8,
                                             (i & 7) + 1, (i & 3) + 1, (i & 1) + 1);
        
        // Load floating point vectors
        __m256 f1 = _mm256_loadu_ps((float*)&arr_f32[idx]);
        __m256 f2 = _mm256_loadu_ps((float*)&arr_f32[idx + 1]);
        __m256 f3 = _mm256_loadu_ps((float*)&arr_f32[idx + 2]);
        __m256 f4 = _mm256_loadu_ps((float*)&arr_f32[idx + 3]);
        __m256 f5 = _mm256_loadu_ps((float*)&arr_f32[idx + 4]);
        __m256 f6 = _mm256_loadu_ps((float*)&arr_f32[idx + 5]);
        __m256 f7 = _mm256_loadu_ps((float*)&arr_f32[idx + 6]);
        __m256 f8 = _mm256_loadu_ps((float*)&arr_f32[idx + 7]);
        
        // Call inline assembly with 10 arguments
        __m256 result2 = complex_asm_10_args(f1, f2, f3, f4, f5, f6, f7, f8,
                                            (i & 3) + 1, (i & 7) + 1);
        
        // Complex multi-statement expression with many temporaries
        __m256i result3 = complex_multi_statement_expr(
            arr_i32, idx, idx + 1, idx + 2, idx + 3,
            idx + 4, idx + 5, idx + 6, idx + 7,
            (i & 3) + 1, (i & 7) + 1);
        
        // Store results
        _mm256_storeu_si256(&out_i32[i % ARRAY_SIZE], _mm256_add_epi32(result1, result3));
        _mm256_storeu_ps((float*)&out_f32[i % ARRAY_SIZE], result2);
        
        // Prevent CSE by adding fake dependency
        asm volatile("" : "+x"(v1), "+x"(f1));
    }
}

// AVX-512 specific version for more complex operations
#ifdef __AVX512F__
__attribute__((target("avx512f"), noinline))
static void test_avx512_many_args(
    __m512i* arr_i32, __m512* arr_f32,
    __m512i* out_i32, __m512* out_f32) {
    
    volatile int iter_counter = 0;
    
    for (int i = 0; i < ITERATIONS / 2; i++) {
        iter_counter++;
        
        int idx = i % (ARRAY_SIZE - 4);
        
        // Load AVX-512 vectors
        __m512i v1 = _mm512_loadu_si512(&arr_i32[idx]);
        __m512i v2 = _mm512_loadu_si512(&arr_i32[idx + 1]);
        __m512i v3 = _mm512_loadu_si512(&arr_i32[idx + 2]);
        __m512i v4 = _mm512_loadu_si512(&arr_i32[idx + 3]);
        
        __m512 f1 = _mm512_loadu_ps((float*)&arr_f32[idx]);
        __m512 f2 = _mm512_loadu_ps((float*)&arr_f32[idx + 1]);
        __m512 f3 = _mm512_loadu_ps((float*)&arr_f32[idx + 2]);
        __m512 f4 = _mm512_loadu_ps((float*)&arr_f32[idx + 3]);
        
        // Complex AVX-512 operation with many arguments via builtin
        __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_GT);
        
        // Shuffle with many arguments - potentially triggering optab expansion
        __m512i shuffled = _mm512_permutex2var_epi32(v1, v3, v4);
        
        // Blend with mask and multiple sources
        __m512i blended = _mm512_mask_blend_epi32(mask, v2, v3);
        
        // Store results
        _mm512_storeu_si512(&out_i32[i % ARRAY_SIZE], 
                           _mm512_add_epi32(shuffled, blended));
        _mm512_storeu_ps((float*)&out_f32[i % ARRAY_SIZE],
                        _mm512_add_ps(f1, f2));
    }
}
#endif

int main() {
    // Allocate aligned memory for better performance
    __m256i* arr_i32 = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i16 = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* arr_i8 = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256* arr_f32 = (__m256*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256));
    __m256d* arr_f64 = (__m256d*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256d));
    __m256i* mask_arr = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256i* blend_arr = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    
    __m256i* out_i32 = (__m256i*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256i));
    __m256* out_f32 = (__m256*)aligned_alloc(32, ARRAY_SIZE * sizeof(__m256));
    
    if (!arr_i32 || !arr_i16 || !arr_i8 || !arr_f32 || !arr_f64 || 
        !mask_arr || !blend_arr || !out_i32 || !out_f32) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(arr_i32, arr_i16, arr_i8, arr_f32, arr_f64, mask_arr, blend_arr);
    
    // Run test with many arguments
    test_many_args(arr_i32, arr_i16, arr_i8, arr_f32, arr_f64, 
                   mask_arr, blend_arr, out_i32, out_f32);
    
#ifdef __AVX512F__
    // AVX-512 specific test
    __m512i* arr_i32_512 = (__m512i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m512i));
    __m512* arr_f32_512 = (__m512*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m512));
    __m512i* out_i32_512 = (__m512i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m512i));
    __m512* out_f32_512 = (__m512*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m512));
    
    if (arr_i32_512 && arr_f32_512 && out_i32_512 && out_f32_512) {
        // Initialize AVX-512 arrays
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int32_t data[16];
            float fdata[16];
            for (int j = 0; j < 16; j++) {
                data[j] = (int32_t)prng();
                fdata[j] = (float)prng() / (float)UINT32_MAX;
            }
            _mm512_storeu_si512(&arr_i32_512[i], _mm512_loadu_si512((const __m512i*)data));
            _mm512_storeu_ps(&arr_f32_512[i], _mm512_loadu_ps(fdata));
        }
        
        test_avx512_many_args(arr_i32_512, arr_f32_512, out_i32_512, out_f32_512);
        
        free(arr_i32_512);
        free(arr_f32_512);
        free(out_i32_512);
        free(out_f32_512);
    }
#endif
    
    // Compute checksum
    uint64_t checksum_i32 = 0;
    uint64_t checksum_f32 = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int32_t* i32_data = (int32_t*)&out_i32[i];
        float* f32_data = (float*)&out_f32[i];
        
        for (int j = 0; j < 8; j++) {
            checksum_i32 += (uint64_t)(i32_data[j] & 0xFFFFFFFF);
            checksum_f32 += (uint64_t)(*(uint32_t*)&f32_data[j]);
        }
    }
    
    printf("Checksum i32: 0x%016llx\n", checksum_i32);
    printf("Checksum f32: 0x%016llx\n", checksum_f32);
    
    // Cleanup
    free(arr_i32);
    free(arr_i16);
    free(arr_i8);
    free(arr_f32);
    free(arr_f64);
    free(mask_arr);
    free(blend_arr);
    free(out_i32);
    free(out_f32);
    
    return 0;
}
