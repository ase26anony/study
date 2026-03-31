#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t* arr32, size_t n32, 
                       float* arrf32, size_t nf32,
                       double* arrf64, size_t nf64) {
    for (size_t i = 0; i < n32; i++) {
        arr32[i] = (int32_t)lcg_rand();
    }
    for (size_t i = 0; i < nf32; i++) {
        arrf32[i] = (float)lcg_rand() / 1000.0f;
    }
    for (size_t i = 0; i < nf64; i++) {
        arrf64[i] = (double)lcg_rand() / 1000.0;
    }
}

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int* control) {
    int32x16_t mask = {0};
    int base = *control;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask[i] = (base + i * 3) % 32;
        if (mask[i] < 0) mask[i] += 32;
    }
    
    /* Additional non-linear transformation */
    for (int i = 0; i < 16; i++) {
        mask[i] = (mask[i] * 13 + 7) & 31;
    }
    
    return mask;
}

/* Another mask with different computation pattern */
static int32x16_t compute_alternate_mask(volatile int* control) {
    int32x16_t mask = {0};
    int seed = *control ^ 0x55AA55AA;
    
    for (int i = 0; i < 16; i++) {
        seed = seed * 1664525 + 1013904223;
        mask[i] = (seed >> 16) & 31;
    }
    
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    #define ARRAY_SIZE 256
    int32_t arr32[ARRAY_SIZE];
    float arrf32[ARRAY_SIZE];
    double arrf64[ARRAY_SIZE];
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 42;
    volatile int control2 = 73;
    volatile int control3 = 19;
    
    /* Initialize with pseudo-random data */
    init_arrays(arr32, ARRAY_SIZE, arrf32, ARRAY_SIZE, arrf64, ARRAY_SIZE);
    
    /* Cast array segments to vector types */
    int32x16_t* vec16_i32 = (int32x16_t*)arr32;
    float32x16_t* vec16_f32 = (float32x16_t*)arrf32;
    float64x8_t* vec8_f64 = (float64x8_t*)arrf64;
    int32x8_t* vec8_i32 = (int32x8_t*)arr32;
    
    /* Result vectors */
    int32x16_t result1 = {0}, result2 = {0}, result3 = {0};
    float32x16_t fresult1 = {0}, fresult2 = {0};
    float64x8_t dresult1 = {0};
    
    /* KERNEL 1: Complex shuffle with computed mask */
    printf("Starting Kernel 1...\n");
    for (int iter = 0; iter < 100; iter++) {
        control1 = iter;
        
        /* Compute dynamic mask */
        int32x16_t mask = compute_complex_mask(&control1);
        
        /* Complex shuffle operation that may expand to many operands */
        result1 = __builtin_shuffle(vec16_i32[0], vec16_i32[1], mask);
        
        /* Chain another shuffle using the result */
        int32x16_t mask2 = compute_alternate_mask(&control1);
        result2 = __builtin_shuffle(result1, vec16_i32[2], mask2);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles with mixed types */
    printf("Starting Kernel 2...\n");
    for (int iter = 0; iter < 50; iter++) {
        control2 = iter * 2;
        
        /* First shuffle with floating-point vectors */
        int32x16_t fpmask = compute_complex_mask(&control2);
        fresult1 = __builtin_shuffle(vec16_f32[0], vec16_f32[1], fpmask);
        
        /* Convert and shuffle with integer vectors */
        int32x16_t temp_int = (int32x16_t)fresult1;
        int32x16_t mask3 = compute_alternate_mask(&control2);
        
        /* Chain multiple shuffle operations */
        result3 = __builtin_shuffle(temp_int, vec16_i32[3], mask3);
        
        /* Another shuffle in the chain */
        int32x16_t mask4 = compute_complex_mask(&control3);
        int32x16_t temp2 = __builtin_shuffle(result3, vec16_i32[4], mask4);
        
        /* Final shuffle with different source */
        int32x16_t mask5 = compute_alternate_mask(&control3);
        result1 = __builtin_shuffle(temp2, vec16_i32[5], mask5);
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    printf("Starting Kernel 3...\n");
    for (int iter = 0; iter < 30; iter++) {
        control3 = iter * 3;
        
        /* Compute two different masks */
        int32x16_t mask_a = compute_complex_mask(&control1);
        int32x16_t mask_b = compute_alternate_mask(&control2);
        
        /* Generate two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec16_i32[6], vec16_i32[7], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec16_i32[8], vec16_i32[9], mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selector = (vec16_i32[0] > vec16_i32[1]);
        result2 = selector ? shuffle_a : shuffle_b;
        
        /* Additional mixed-type operation */
        float32x16_t float_shuffle = __builtin_shuffle(vec16_f32[2], vec16_f32[3], mask_a);
        fresult2 = float_shuffle * (float32x16_t){2.0f};
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many operands */
    printf("Starting Kernel 4...\n");
    
    #ifdef __x86_64__
    /* x86-specific inline assembly with vector constraints */
    for (int iter = 0; iter < 10; iter++) {
        int32x8_t v1 = vec8_i32[0];
        int32x8_t v2 = vec8_i32[1];
        int32x8_t v3 = vec8_i32[2];
        int32x8_t v4 = vec8_i32[3];
        int32x8_t v5 = vec8_i32[4];
        int32x8_t v6 = vec8_i32[5];
        int32x8_t v7 = vec8_i32[6];
        int32x8_t v8 = vec8_i32[7];
        int32x8_t out1, out2, out3, out4;
        
        /* Inline asm with many vector operands */
        asm volatile (
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            "vmovdqa %[v5], %%ymm4\n\t"
            "vmovdqa %[v6], %%ymm5\n\t"
            "vmovdqa %[v7], %%ymm6\n\t"
            "vmovdqa %[v8], %%ymm7\n\t"
            "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm8\n\t"
            "vperm2i128 $0x21, %%ymm2, %%ymm3, %%ymm9\n\t"
            "vperm2i128 $0x21, %%ymm4, %%ymm5, %%ymm10\n\t"
            "vperm2i128 $0x21, %%ymm6, %%ymm7, %%ymm11\n\t"
            "vmovdqa %%ymm8, %[out1]\n\t"
            "vmovdqa %%ymm9, %[out2]\n\t"
            "vmovdqa %%ymm10, %[out3]\n\t"
            "vmovdqa %%ymm11, %[out4]\n\t"
            : [out1] "=v" (out1),
              [out2] "=v" (out2),
              [out3] "=v" (out3),
              [out4] "=v" (out4)
            : [v1] "v" (v1),
              [v2] "v" (v2),
              [v3] "v" (v3),
              [v4] "v" (v4),
              [v5] "v" (v5),
              [v6] "v" (v6),
              [v7] "v" (v7),
              [v8] "v" (v8)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
              "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
              "memory"
        );
        
        vec8_i32[8] = out1;
        vec8_i32[9] = out2;
        vec8_i32[10] = out3;
        vec8_i32[11] = out4;
    }
    #endif
    
    /* Target-specific builtins (conditional compilation) */
    #ifdef __SSE2__
    for (int iter = 0; iter < 5; iter++) {
        /* Use SSE/AVX builtins when available */
        int32x8_t sse_vec = vec8_i32[12];
        
        #ifdef __AVX2__
        /* AVX2 specific permutation */
        sse_vec = __builtin_ia32_pshufd256(sse_vec, (control1 & 0xFF));
        #elif defined(__SSSE3__)
        /* SSSE3 specific shuffle */
        int32x8_t shuffle_mask = {0, 2, 4, 6, 1, 3, 5, 7};
        sse_vec = __builtin_ia32_pshufd(sse_vec, (control2 & 0xFF));
        #endif
        
        vec8_i32[12] = sse_vec;
    }
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    
    /* Horizontal addition for result vectors */
    for (int i = 0; i < 16; i++) {
        checksum += result1[i];
        checksum += result2[i];
        checksum += result3[i];
        checksum += (int64_t)fresult1[i];
        checksum += (int64_t)fresult2[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (int64_t)dresult1[i];
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    /* Additional complex shuffle with __builtin_shufflevector */
    printf("Final complex shufflevector test...\n");
    
    /* Create vectors with explicit sizes for shufflevector */
    typedef int32_t int32x12_t __attribute__((vector_size(48)));
    typedef int32_t int32x24_t __attribute__((vector_size(96)));
    
    int32x12_t vec12_a = {0}, vec12_b = {0};
    int32x24_t vec24_result = {0};
    
    /* Initialize 12-element vectors */
    for (int i = 0; i < 12; i++) {
        vec12_a[i] = lcg_rand() % 100;
        vec12_b[i] = lcg_rand() % 100;
    }
    
    /* Complex shufflevector with many indices - may trigger 10+ operand expansion */
    vec24_result = __builtin_shufflevector(vec12_a, vec12_b,
        0, 12, 1, 13, 2, 14, 3, 15, 4, 16, 5, 17,
        6, 18, 7, 19, 8, 20, 9, 21, 10, 22, 11, 23);
    
    /* Add shufflevector result to checksum */
    for (int i = 0; i < 24; i++) {
        checksum += vec24_result[i];
    }
    
    printf("Final checksum with shufflevector: %ld\n", checksum);
    
    return 0;
}
