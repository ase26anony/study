#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr1, int32_t *arr2, int32_t *arr3, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = (int32_t)lcg_rand();
        arr2[i] = (int32_t)lcg_rand();
        arr3[i] = (int32_t)lcg_rand();
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask generation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (control & 1) ? (i * 3) % 16 : (i * 5) % 16;
        control = control >> 1;
        if (control == 0) control = 0x5555;
    }
    
    return mask;
}

/* Kernel 1: Complex shuffle with computed mask */
static void kernel1(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, volatile int control) {
    int32x16_t mask = compute_dynamic_mask(control);
    
    /* This shuffle with large vectors and computed mask may require
       many operands during RTL expansion */
    *result = __builtin_shuffle(*a, *b, mask);
}

/* Kernel 2: Chain of shuffles - output of one is input to next */
static void kernel2(int32x16_t *result, const int32x16_t *a, 
                    const int32x16_t *b, const int32x16_t *c,
                    volatile int control1, volatile int control2) {
    int32x16_t mask1 = compute_dynamic_mask(control1);
    int32x16_t mask2 = compute_dynamic_mask(control2);
    
    /* Chain shuffles - intermediate results increase operand count */
    int32x16_t temp1 = __builtin_shuffle(*a, *b, mask1);
    int32x16_t temp2 = __builtin_shuffle(temp1, *c, mask2);
    
    /* Another shuffle with mixed sources */
    int32x16_t mask3;
    int32_t *mask3_ptr = (int32_t*)&mask3;
    for (int i = 0; i < 16; i++) {
        mask3_ptr[i] = (control1 + i) % 24;
    }
    
    /* This may require handling many operands */
    *result = __builtin_shuffle(temp2, *a, mask3);
}

/* Kernel 3: Conditional vector permutation */
static void kernel3(int32x16_t *result, const int32x16_t *a,
                    const int32x16_t *b, const int32x16_t *c,
                    volatile int condition) {
    int32x16_t mask1, mask2;
    int32_t *m1 = (int32_t*)&mask1;
    int32_t *m2 = (int32_t*)&mask2;
    
    for (int i = 0; i < 16; i++) {
        m1[i] = (i * 7) % 16;
        m2[i] = (i * 11) % 16;
    }
    
    int32x16_t shuffle1 = __builtin_shuffle(*a, *b, mask1);
    int32x16_t shuffle2 = __builtin_shuffle(*b, *c, mask2);
    
    /* Conditional selection between two shuffle results */
    *result = condition ? shuffle1 : shuffle2;
}

/* Kernel 4: Mixed vector types and widths */
static void kernel4(float32x16_t *fresult, const float32x16_t *fa,
                    const int32x16_t *ia, volatile int control) {
    /* Convert int vector to float */
    float32x16_t fb;
    float *fb_ptr = (float*)&fb;
    int32_t *ia_ptr = (int32_t*)ia;
    
    for (int i = 0; i < 16; i++) {
        fb_ptr[i] = (float)ia_ptr[i];
    }
    
    /* Create complex mask spanning both vectors */
    int32x16_t mask;
    int32_t *mask_ptr = (int32_t*)&mask;
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (control + i * 3) % 32;
    }
    
    /* Shuffle with mixed sources - may require many operands */
    *fresult = __builtin_shuffle(*fa, fb, mask);
}

/* Kernel 5: Inline assembly with many vector operands */
static void kernel5_asm(int32x16_t *out, const int32x16_t *in1,
                        const int32x16_t *in2, const int32x16_t *in3) {
    /* Hypothetical multi-operand vector operation via inline asm
       This forces the compiler to handle many input/output operands */
    asm volatile (
        "# Complex vector operation with many operands\n"
        "vmovdqa %[in1], %%ymm0\n"
        "vmovdqa %[in2], %%ymm1\n"
        "vmovdqa %[in3], %%ymm2\n"
        "# Some hypothetical multi-operand operation\n"
        "vpalignr $4, %%ymm1, %%ymm0, %%ymm3\n"
        "vpshufd $0x1B, %%ymm2, %%ymm4\n"
        "vpaddd %%ymm3, %%ymm4, %%ymm5\n"
        "vmovdqa %%ymm5, %[out]\n"
        : [out] "=v" (*out)
        : [in1] "v" (*in1), [in2] "v" (*in2), [in3] "v" (*in3)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
    );
}

/* Target-specific builtins */
#ifdef __x86_64__
static void kernel_x86(int32x8_t *result, const int32x8_t *a, 
                       const int32x8_t *b) {
    /* Use x86-specific builtins that may map to multi-operand instructions */
    __m256i va = *(__m256i*)a;
    __m256i vb = *(__m256i*)b;
    
    /* Complex sequence of x86 intrinsics */
    __m256i vc = _mm256_shuffle_epi32(va, _MM_SHUFFLE(1, 0, 3, 2));
    __m256i vd = _mm256_permute2x128_si256(va, vb, 0x21);
    __m256i ve = _mm256_add_epi32(vc, vd);
    
    *(__m256i*)result = ve;
}
#endif

#ifdef __aarch64__
static void kernel_arm(int32x4_t *result, const int32x4_t *a,
                       const int32x4_t *b) {
    /* ARM NEON builtins */
    int32x4_t va = *a;
    int32x4_t vb = *b;
    
    /* Complex NEON operations */
    int32x4_t vc = __builtin_neon_vrev64q_s32(va);
    int32x4_t vd = __builtin_neon_vextq_s32(va, vb, 2);
    int32x4_t ve = vc + vd;
    
    *result = ve;
}
#endif

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const int32_t *data, size_t size) {
    int64_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    const size_t VEC_COUNT = ARRAY_SIZE / 16;
    
    /* Allocate and initialize arrays */
    int32_t *array1 = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array2 = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *array3 = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *results = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    init_arrays(array1, array2, array3, ARRAY_SIZE);
    memset(results, 0, ARRAY_SIZE * sizeof(int32_t));
    
    volatile int control_var = 0x1234;
    int64_t total_checksum = 0;
    
    /* Main processing loop with data-dependent operations */
    for (size_t iter = 0; iter < 100; iter++) {
        control_var = (control_var * 1103515245 + 12345) & 0xFFFF;
        
        for (size_t i = 0; i < VEC_COUNT; i++) {
            int32x16_t *vec_a = (int32x16_t*)(array1 + i * 16);
            int32x16_t *vec_b = (int32x16_t*)(array2 + i * 16);
            int32x16_t *vec_c = (int32x16_t*)(array3 + i * 16);
            int32x16_t *vec_result = (int32x16_t*)(results + i * 16);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 1: Complex shuffle with dynamic mask */
            kernel1(vec_result, vec_a, vec_b, control_var + i);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 2: Chain of shuffles */
            int32x16_t temp_result;
            kernel2(&temp_result, vec_a, vec_b, vec_c, 
                   control_var + i, control_var + i + 1);
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 3: Conditional permutation */
            int32x16_t cond_result;
            kernel3(&cond_result, vec_a, vec_b, vec_c, (control_var >> i) & 1);
            
            /* Combine results */
            *vec_result = *vec_result + temp_result + cond_result;
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 4: Mixed types */
            float32x16_t float_result;
            float32x16_t *float_vec = (float32x16_t*)vec_a;
            kernel4(&float_result, float_vec, vec_b, control_var);
            
            /* Convert back and accumulate */
            int32x16_t int_from_float = *(__builtin_convertvector(float_result, int32x16_t));
            *vec_result = *vec_result + int_from_float;
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
            
            /* Kernel 5: Inline assembly */
            kernel5_asm(vec_result, vec_a, vec_b, vec_c);
        }
        
        /* Update control variable for next iteration */
        control_var = (control_var * 3 + 1) & 0xFFFF;
    }
    
    /* Compute final checksum */
    total_checksum = compute_checksum(results, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    printf("Final checksum: %ld\n", (long)total_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(results);
    
    return 0;
}
