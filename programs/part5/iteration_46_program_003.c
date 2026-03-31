/* test_optabs_many_args.c
 * 
 * This program is designed to trigger the 10-11 argument optab expansion
 * paths in GCC's optabs.cc (lines 8254-8263).
 * 
 * Compilation options for coverage:
 *   gcc -O2 -ftree-vectorize -fdump-rtl-expand -mavx2 -o test test_optabs_many_args.c
 *   gcc -O3 -mavx512f -fdump-rtl-combine -fdump-rtl-expand -o test test_optabs_many_args.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for reproducible results */
static uint32_t prng_state = 123456789;
static inline uint32_t prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(float* arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (float)(prng_next() % 1000) / 100.0f;
    }
}

/* Function to inhibit constant propagation and CSE */
static inline void inhibit_opt(volatile int* counter) {
    asm volatile("" : "+r"(*counter) : : "memory");
}

/* Complex expression with many temporaries - forces expander to handle many operands */
static inline __m256 complex_multi_op_expr(
    __m256 a, __m256 b, __m256 c, __m256 d,
    __m256 e, __m256 f, __m256 g, __m256 h,
    int imm1, int imm2, int imm3) {
    
    /* Create many intermediate values to force complex expansion */
    __m256 t1 = _mm256_add_ps(a, b);
    __m256 t2 = _mm256_sub_ps(c, d);
    __m256 t3 = _mm256_mul_ps(e, f);
    __m256 t4 = _mm256_div_ps(g, h);
    
    /* Mix operations with the immediate values */
    __m256 t5 = _mm256_add_ps(t1, _mm256_set1_ps((float)imm1));
    __m256 t6 = _mm256_sub_ps(t2, _mm256_set1_ps((float)imm2));
    __m256 t7 = _mm256_mul_ps(t3, _mm256_set1_ps((float)imm3));
    
    /* Final blend with many arguments conceptually */
    return _mm256_add_ps(_mm256_add_ps(t5, t6), t7);
}

#ifdef __AVX512F__
/* AVX-512 version with mask registers for more arguments */
__attribute__((target("avx512f")))
static inline __m512 avx512_complex_blend(
    __m512 a, __m512 b, __m512 c, __m512 d,
    __m512 e, __m512 f, __m512 g, __m512 h,
    __m512 i, __m512 j, __mmask16 mask) {
    
    /* This complex operation conceptually uses 11 arguments:
     * a, b, c, d, e, f, g, h, i, j, mask
     * The compiler might expand this into an optab with 11 operands
     */
    
    /* Multiple blends with mask - each could be expanded separately */
    __m512 t1 = _mm512_mask_blend_ps(mask, a, b);
    __m512 t2 = _mm512_mask_blend_ps(mask, c, d);
    __m512 t3 = _mm512_mask_blend_ps(mask, e, f);
    __m512 t4 = _mm512_mask_blend_ps(mask, g, h);
    
    /* Final operation combining all */
    __m512 r1 = _mm512_add_ps(t1, t2);
    __m512 r2 = _mm512_add_ps(t3, t4);
    __m512 r3 = _mm512_add_ps(r1, r2);
    
    /* Blend with the remaining vectors using the mask */
    return _mm512_mask_blend_ps(mask, r3, _mm512_add_ps(i, j));
}
#endif

/* Inline assembly with exactly 11 operands to trigger the optab path */
static inline void asm_11_operands(
    float* out, const float* in1, const float* in2, const float* in3,
    const float* in4, const float* in5, const float* in6,
    int imm1, int imm2, int imm3, int imm4) {
    
    /* Extended asm with 11 input operands (10 memory + 1 immediate would be 11 total) */
    asm volatile (
        /* Complex memory operations that might expand to many-argument optabs */
        "vmovups (%1), %%ymm0\n\t"
        "vmovups (%2), %%ymm1\n\t"
        "vmovups (%3), %%ymm2\n\t"
        "vmovups (%4), %%ymm3\n\t"
        "vaddps %%ymm0, %%ymm1, %%ymm4\n\t"
        "vaddps %%ymm2, %%ymm3, %%ymm5\n\t"
        /* Use immediates in operations */
        "vpermilps $0x%7, %%ymm4, %%ymm4\n\t"
        "vpermilps $0x%8, %%ymm5, %%ymm5\n\t"
        "vaddps %%ymm4, %%ymm5, %%ymm6\n\t"
        /* More operations with remaining inputs */
        "vmovups (%5), %%ymm7\n\t"
        "vmovups (%6), %%ymm8\n\t"
        "vblendps $0x%9, %%ymm7, %%ymm8, %%ymm9\n\t"
        "vaddps %%ymm6, %%ymm9, %%ymm10\n\t"
        "vmovups %%ymm10, (%0)\n\t"
        : 
        : "r"(out), "r"(in1), "r"(in2), "r"(in3), 
          "r"(in4), "r"(in5), "r"(in6),
          "i"(imm1), "i"(imm2), "i"(imm3), "i"(imm4)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", 
          "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10",
          "memory"
    );
}

/* Vector shuffle with many lane indices - conceptually 10+ arguments */
static inline __m256i vector_shuffle_complex(
    __m256i a, __m256i b, __m256i c, __m256i d,
    int idx0, int idx1, int idx2, int idx3,
    int idx4, int idx5, int idx6) {
    
    /* This represents a complex shuffle operation that might need
     * 11 arguments: 4 source vectors + 7 lane indices
     */
    
    /* Multiple intermediate shuffles */
    __m256i t1 = _mm256_shuffle_epi32(a, _MM_SHUFFLE(idx0, idx1, idx2, idx3));
    __m256i t2 = _mm256_shuffle_epi32(b, _MM_SHUFFLE(idx1, idx2, idx3, idx4));
    __m256i t3 = _mm256_shuffle_epi32(c, _MM_SHUFFLE(idx2, idx3, idx4, idx5));
    __m256i t4 = _mm256_shuffle_epi32(d, _MM_SHUFFLE(idx3, idx4, idx5, idx6));
    
    /* Blend them together */
    __m256i r1 = _mm256_add_epi32(t1, t2);
    __m256i r2 = _mm256_add_epi32(t3, t4);
    
    return _mm256_add_epi32(r1, r2);
}

/* Main test function with target attribute - marked noinline to prevent optimization */
__attribute__((target("avx2"), noinline))
static void test_many_args(
    float* output, const float* input1, const float* input2,
    const float* input3, const float* input4, size_t n) {
    
    volatile int iter_counter = 0;  /* Prevent loop unrolling */
    
    for (size_t i = 0; i < n; i += 8) {
        inhibit_opt(&iter_counter);
        iter_counter++;
        
        /* Load 8 vectors (would be more with unrolling) */
        __m256 v1 = _mm256_loadu_ps(&input1[i]);
        __m256 v2 = _mm256_loadu_ps(&input1[i + 8]);
        __m256 v3 = _mm256_loadu_ps(&input2[i]);
        __m256 v4 = _mm256_loadu_ps(&input2[i + 8]);
        __m256 v5 = _mm256_loadu_ps(&input3[i]);
        __m256 v6 = _mm256_loadu_ps(&input3[i + 8]);
        __m256 v7 = _mm256_loadu_ps(&input4[i]);
        __m256 v8 = _mm256_loadu_ps(&input4[i + 8]);
        
        /* Complex operation with many arguments */
        __m256 result = complex_multi_op_expr(v1, v2, v3, v4, v5, v6, v7, v8,
                                             iter_counter, iter_counter + 1, iter_counter + 2);
        
        /* Store result */
        _mm256_storeu_ps(&output[i], result);
        
        /* Also test the inline assembly with many operands */
        if (i + 16 < n) {
            asm_11_operands(&output[i + 8], 
                           &input1[i], &input2[i], &input3[i],
                           &input4[i], &input1[i + 8], &input2[i + 8],
                           iter_counter & 0xF, (iter_counter + 1) & 0xF,
                           (iter_counter + 2) & 0xF, (iter_counter + 3) & 0xF);
        }
    }
}

/* Alternative test using integer vectors and shuffles */
__attribute__((target("avx2"), noinline))
static void test_many_args_int(
    int32_t* output, const int32_t* input1, const int32_t* input2,
    const int32_t* input3, const int32_t* input4, size_t n) {
    
    volatile int iter_counter = 0;
    
    for (size_t i = 0; i < n; i += 8) {
        inhibit_opt(&iter_counter);
        iter_counter++;
        
        /* Load integer vectors */
        __m256i v1 = _mm256_loadu_si256((const __m256i*)&input1[i]);
        __m256i v2 = _mm256_loadu_si256((const __m256i*)&input2[i]);
        __m256i v3 = _mm256_loadu_si256((const __m256i*)&input3[i]);
        __m256i v4 = _mm256_loadu_si256((const __m256i*)&input4[i]);
        
        /* Complex shuffle with many arguments (4 vectors + 7 indices = 11 total) */
        __m256i result = vector_shuffle_complex(
            v1, v2, v3, v4,
            iter_counter & 3, (iter_counter + 1) & 3,
            (iter_counter + 2) & 3, (iter_counter + 3) & 3,
            (iter_counter + 4) & 3, (iter_counter + 5) & 3,
            (iter_counter + 6) & 3);
        
        _mm256_storeu_si256((__m256i*)&output[i], result);
    }
}

/* Compute checksum for validation */
static float compute_checksum(const float* data, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        sum += (double)data[i];
    }
    return (float)sum;
}

static int32_t compute_checksum_int(const int32_t* data, size_t n) {
    int64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += (int64_t)data[i];
    }
    return (int32_t)(sum & 0xFFFFFFFF);
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_SIZE = ARRAY_SIZE * 4;  /* Extra space for vector loads */
    
    /* Allocate and initialize arrays */
    float* input1 = (float*)aligned_alloc(32, VEC_SIZE * sizeof(float));
    float* input2 = (float*)aligned_alloc(32, VEC_SIZE * sizeof(float));
    float* input3 = (float*)aligned_alloc(32, VEC_SIZE * sizeof(float));
    float* input4 = (float*)aligned_alloc(32, VEC_SIZE * sizeof(float));
    float* output = (float*)aligned_alloc(32, VEC_SIZE * sizeof(float));
    
    int32_t* input1_int = (int32_t*)aligned_alloc(32, VEC_SIZE * sizeof(int32_t));
    int32_t* input2_int = (int32_t*)aligned_alloc(32, VEC_SIZE * sizeof(int32_t));
    int32_t* input3_int = (int32_t*)aligned_alloc(32, VEC_SIZE * sizeof(int32_t));
    int32_t* input4_int = (int32_t*)aligned_alloc(32, VEC_SIZE * sizeof(int32_t));
    int32_t* output_int = (int32_t*)aligned_alloc(32, VEC_SIZE * sizeof(int32_t));
    
    if (!input1 || !input2 || !input3 || !input4 || !output ||
        !input1_int || !input2_int || !input3_int || !input4_int || !output_int) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(input1, VEC_SIZE);
    init_arrays(input2, VEC_SIZE);
    init_arrays(input3, VEC_SIZE);
    init_arrays(input4, VEC_SIZE);
    
    /* Initialize integer arrays */
    for (size_t i = 0; i < VEC_SIZE; i++) {
        input1_int[i] = (int32_t)prng_next() % 1000;
        input2_int[i] = (int32_t)prng_next() % 1000;
        input3_int[i] = (int32_t)prng_next() % 1000;
        input4_int[i] = (int32_t)prng_next() % 1000;
    }
    
    /* Run tests */
    printf("Testing float operations with many arguments...\n");
    test_many_args(output, input1, input2, input3, input4, ARRAY_SIZE);
    
    printf("Testing integer operations with many arguments...\n");
    test_many_args_int(output_int, input1_int, input2_int, input3_int, input4_int, ARRAY_SIZE);
    
    /* Compute and print checksums */
    float checksum_float = compute_checksum(output, ARRAY_SIZE);
    int32_t checksum_int = compute_checksum_int(output_int, ARRAY_SIZE);
    
    printf("Float checksum: %f\n", checksum_float);
    printf("Integer checksum: %d\n", checksum_int);
    
    /* Cleanup */
    free(input1);
    free(input2);
    free(input3);
    free(input4);
    free(output);
    free(input1_int);
    free(input2_int);
    free(input3_int);
    free(input4_int);
    free(output_int);
    
    return 0;
}
