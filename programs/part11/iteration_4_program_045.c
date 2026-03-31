#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x4_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] += control % 7;
        if (i % 5 == 0) mask_data[i] -= control % 5;
    }
    
    return mask;
}

/* Another mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control * 2 + i * 5) % 32;
        mask_data[i] ^= (control << 3);
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    int32_t results[256] = {0};
    
    /* Initialize with pseudo-random data */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 19;
    
    /* Cast array segments to vector types */
    int32x16_t *vec_a = (int32x16_t*)data_a;
    int32x16_t *vec_b = (int32x16_t*)data_b;
    int32x16_t *vec_c = (int32x16_t*)data_c;
    int32x16_t *vec_results = (int32x16_t*)results;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 1: Complex shuffle with computed mask */
    printf("Running Kernel 1...\n");
    for (int i = 0; i < 8; i++) {
        /* Compute data-dependent mask */
        int32x16_t mask = compute_complex_mask(control1 + i);
        
        /* Complex shuffle operation - may require many operands during expansion */
        vec_results[i] = __builtin_shuffle(vec_a[i], vec_b[i], mask);
        
        /* Modify control to change mask pattern */
        control1 += (i % 3);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    printf("Running Kernel 2...\n");
    int32x16_t chain_temp[4];
    
    for (int i = 0; i < 4; i++) {
        /* First shuffle */
        int32x16_t mask1 = compute_complex_mask(control2 + i * 2);
        chain_temp[0] = __builtin_shuffle(vec_a[i*2], vec_b[i*2], mask1);
        
        /* Second shuffle using result of first */
        int32x16_t mask2 = compute_alternate_mask(control2 + i * 2 + 1);
        chain_temp[1] = __builtin_shuffle(chain_temp[0], vec_c[i*2], mask2);
        
        /* Third shuffle chaining previous results */
        int32x16_t mask3 = compute_complex_mask(control2 + i * 3);
        chain_temp[2] = __builtin_shuffle(chain_temp[1], vec_a[i*2 + 1], mask3);
        
        /* Fourth shuffle with all previous inputs */
        int32x16_t mask4 = compute_alternate_mask(control2 + i * 4);
        vec_results[8 + i] = __builtin_shuffle(
            chain_temp[2], 
            chain_temp[0], 
            mask4
        );
        
        control2 += (i % 5);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* KERNEL 3: Conditional vector permutation */
    printf("Running Kernel 3...\n");
    for (int i = 0; i < 4; i++) {
        int32x16_t mask_a = compute_complex_mask(control3 + i * 7);
        int32x16_t mask_b = compute_alternate_mask(control3 + i * 11);
        
        int32x16_t shuffle_a = __builtin_shuffle(vec_b[i*3], vec_c[i*3], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_c[i*3], vec_a[i*3], mask_b);
        
        /* Conditional selection between two shuffle results */
        int32x16_t selector;
        int32_t *sel_data = (int32_t*)&selector;
        for (int j = 0; j < 16; j++) {
            sel_data[j] = (control3 + i + j) % 2 ? -1 : 0;
        }
        
        /* This conditional operation may expand to many operands */
        vec_results[12 + i] = selector ? shuffle_a : shuffle_b;
        
        control3 += (i % 7);
    }
    
    /* KERNEL 4: Mixed vector types and widths */
    printf("Running Kernel 4...\n");
    float32x8_t *float_vec_a = (float32x8_t*)data_a;
    float32x8_t *float_vec_b = (float32x8_t*)data_b;
    int32x8_t *int8_vec = (int32x8_t*)data_c;
    
    /* Create a mixed-type shuffle mask */
    int32x8_t mixed_mask;
    int32_t *mask_ptr = (int32_t*)&mixed_mask;
    for (int i = 0; i < 8; i++) {
        mask_ptr[i] = (control1 + control2 + i) % 16;
    }
    
    /* Shuffle with mixed vector types */
    for (int i = 0; i < 4; i++) {
        /* This may require special handling during expansion */
        float32x8_t temp = __builtin_shuffle(float_vec_a[i], float_vec_b[i], mixed_mask);
        
        /* Store back through integer pointer to force type conversion */
        int32x8_t *int_temp = (int32x8_t*)&temp;
        vec_results[16 + i*2] = __builtin_shuffle(int_temp[0], int8_vec[i], mixed_mask);
    }
    
#ifdef __x86_64__
    /* KERNEL 5: x86-specific builtins with many operands */
    printf("Running x86-specific Kernel 5...\n");
    for (int i = 0; i < 2; i++) {
        /* Use x86-specific shuffle builtins if available */
        #ifdef __SSE4_2__
        /* These builtins often map to complex machine instructions */
        __m128i sse_vec_a = _mm_loadu_si128((__m128i*)&data_a[i*4]);
        __m128i sse_vec_b = _mm_loadu_si128((__m128i*)&data_b[i*4]);
        
        /* Complex sequence of x86 intrinsics */
        __m128i shuffled = _mm_shuffle_epi8(sse_vec_a, sse_vec_b);
        __m128i blended = _mm_blendv_epi8(shuffled, sse_vec_a, sse_vec_b);
        
        _mm_storeu_si128((__m128i*)&results[128 + i*4], blended);
        #endif
        
        #ifdef __AVX2__
        /* AVX2 operations with 256-bit vectors */
        __m256i avx_vec_a = _mm256_loadu_si256((__m256i*)&data_a[i*8]);
        __m256i avx_vec_b = _mm256_loadu_si256((__m256i*)&data_b[i*8]);
        
        __m256i avx_shuffled = _mm256_shuffle_epi8(avx_vec_a, avx_vec_b);
        __m256i avx_blended = _mm256_blendv_epi8(avx_shuffled, avx_vec_a, avx_vec_b);
        
        _mm256_storeu_si256((__m256i*)&results[136 + i*8], avx_blended);
        #endif
    }
#endif

#ifdef __ARM_NEON
    /* KERNEL 6: ARM-specific builtins */
    printf("Running ARM-specific Kernel 6...\n");
    /* ARM NEON intrinsics for completeness */
    int32x4_t neon_vec_a = vld1q_s32(data_a);
    int32x4_t neon_vec_b = vld1q_s32(data_b);
    
    /* Complex ARM permutation */
    int32x4_t rev_a = vrev64q_s32(neon_vec_a);
    int32x4_t tbl_result = vqtbl1q_s8(vreinterpretq_s8_s32(neon_vec_a), 
                                      vreinterpretq_u8_s32(neon_vec_b));
    
    vst1q_s32(results[152], vreinterpretq_s32_s8(tbl_result));
#endif

    /* KERNEL 7: Inline assembly with many vector operands */
    printf("Running Kernel 7 (Inline Assembly)...\n");
    for (int i = 0; i < 2; i++) {
        /* Inline asm with multiple vector operands */
        int32x16_t asm_input1 = vec_a[i];
        int32x16_t asm_input2 = vec_b[i];
        int32x16_t asm_input3 = vec_c[i];
        int32x16_t asm_output;
        
        /* Hypothetical multi-operand vector operation */
        asm volatile (
            /* This template would be replaced with actual vector instructions */
            "# Multi-operand vector operation placeholder\n"
            "# Inputs: %1, %2, %3 -> Output: %0\n"
            "vmovdqa %1, %%ymm0\n"
            "vmovdqa %2, %%ymm1\n"
            "vmovdqa %3, %%ymm2\n"
            "# Complex multi-operand operation\n"
            "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm3\n"
            "vpshufd $0x1B, %%ymm2, %%ymm4\n"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n"
            "vmovdqa %%ymm5, %0\n"
            : "=v"(asm_output)
            : "v"(asm_input1), "v"(asm_input2), "v"(asm_input3)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        vec_results[18 + i] = asm_output;
    }
    
    /* Compute checksum to prevent dead code elimination */
    printf("Computing checksum...\n");
    int64_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += results[i];
    }
    
    /* Also compute horizontal sums of vector results */
    for (int i = 0; i < 20; i++) {
        int32_t *vec_data = (int32_t*)&vec_results[i];
        for (int j = 0; j < 16; j++) {
            checksum += vec_data[j];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
