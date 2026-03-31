#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create non-constant shuffle masks */
static inline int32x16_t create_complex_mask(volatile int offset) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask generation - prevents constant propagation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (i + offset) % 32;
        /* Add some complexity to prevent optimization */
        mask_data[i] ^= (lcg_rand() & 0x7);
    }
    return mask;
}

/* Function with inline assembly using many vector operands */
static void vector_asm_operation(int32x16_t *out, int32x16_t *in1, int32x16_t *in2,
                                 int32x16_t *in3, int32x16_t *in4) {
    /* Inline assembly with multiple vector operands */
    asm volatile (
        /* Hypothetical multi-operand vector operation */
        "# Multi-operand vector operation\n"
        "mov %[out], %[in1]\n\t"
        "vpaddd %[out], %[out], %[in2]\n\t"
        "vpsubd %[out], %[out], %[in3]\n\t"
        "vpxor %[out], %[out], %[in4]"
        : [out] "=v" (*out)
        : [in1] "v" (*in1),
          [in2] "v" (*in2),
          [in3] "v" (*in3),
          [in4] "v" (*in4)
        : "memory"
    );
}

int main() {
    /* Initialize large arrays with pseudo-random data */
    int32_t data_a[64] __attribute__((aligned(64)));
    int32_t data_b[64] __attribute__((aligned(64)));
    int32_t data_c[64] __attribute__((aligned(64)));
    int32_t data_d[64] __attribute__((aligned(64)));
    int32_t result_data[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = lcg_rand() & 0xFFFF;
        data_b[i] = lcg_rand() & 0xFFFF;
        data_c[i] = lcg_rand() & 0xFFFF;
        data_d[i] = lcg_rand() & 0xFFFF;
    }
    
    /* Cast to vector types */
    int32x16_t *vec_a = (int32x16_t *)data_a;
    int32x16_t *vec_b = (int32x16_t *)data_b;
    int32x16_t *vec_c = (int32x16_t *)data_c;
    int32x16_t *vec_d = (int32x16_t *)data_d;
    int32x16_t *vec_result = (int32x16_t *)result_data;
    
    /* Volatile variable to prevent compile-time computation */
    volatile int mask_offset = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    printf("Running Kernel 1: Complex shuffle with computed mask\n");
    for (int iter = 0; iter < 4; iter++) {
        /* Data-dependent mask calculation */
        mask_offset = iter * 3;
        int32x16_t mask = create_complex_mask(mask_offset);
        
        /* Complex shuffle operation that may need many operands during expansion */
        vec_result[iter] = __builtin_shuffle(vec_a[iter], vec_b[iter], mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    printf("Running Kernel 2: Chain of shuffles\n");
    int32x16_t temp1, temp2, temp3;
    
    for (int iter = 0; iter < 2; iter++) {
        mask_offset = iter * 5;
        int32x16_t mask1 = create_complex_mask(mask_offset);
        int32x16_t mask2 = create_complex_mask(mask_offset + 1);
        int32x16_t mask3 = create_complex_mask(mask_offset + 2);
        
        /* Chain of shuffles - each output feeds into next input */
        temp1 = __builtin_shuffle(vec_a[iter], vec_b[iter], mask1);
        temp2 = __builtin_shuffle(temp1, vec_c[iter], mask2);
        temp3 = __builtin_shuffle(temp2, vec_d[iter], mask3);
        
        /* Mix with arithmetic to prevent optimization */
        vec_result[iter + 4] = temp3 + (vec_a[iter] & vec_b[iter]);
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    printf("Running Kernel 3: Conditional vector permutation\n");
    for (int iter = 0; iter < 2; iter++) {
        mask_offset = iter * 7;
        int32x16_t mask_a = create_complex_mask(mask_offset);
        int32x16_t mask_b = create_complex_mask(mask_offset + 4);
        
        /* Conditional selection between two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec_a[iter], vec_c[iter], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_b[iter], vec_d[iter], mask_b);
        
        /* Data-dependent condition */
        int condition = (data_a[iter * 16] & 1);
        vec_result[iter + 6] = condition ? shuffle_a : shuffle_b;
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
    printf("Running Kernel 4: Inline assembly with many operands\n");
    for (int iter = 0; iter < 2; iter++) {
        vector_asm_operation(&vec_result[iter + 8], 
                            &vec_a[iter], 
                            &vec_b[iter], 
                            &vec_c[iter], 
                            &vec_d[iter]);
        asm volatile("" ::: "memory");
    }
    
    /* Target-specific builtins (conditional compilation) */
#ifdef __x86_64__
    printf("Running x86-specific vector operations\n");
    /* Use x86-specific shuffle builtins */
    for (int iter = 0; iter < 2; iter++) {
        int32x8_t x86_vec_a = *((int32x8_t*)&data_a[iter * 8]);
        int32x8_t x86_vec_b = *((int32x8_t*)&data_b[iter * 8]);
        
        /* Complex x86-specific shuffle with immediate operand */
        int32x8_t shuffled = __builtin_ia32_pshufd(x86_vec_a, 0x1B);
        shuffled = __builtin_ia32_paddd128(shuffled, x86_vec_b);
        
        /* Store back */
        *((int32x8_t*)&result_data[iter * 8 + 48]) = shuffled;
    }
#endif
    
#ifdef __ARM_NEON
    printf("Running ARM-specific vector operations\n");
    /* ARM NEON specific operations */
    for (int iter = 0; iter < 2; iter++) {
        int32x4_t neon_vec_a = *((int32x4_t*)&data_a[iter * 4]);
        int32x4_t neon_vec_b = *((int32x4_t*)&data_b[iter * 4]);
        
        /* Use ARM NEON builtins */
        int32x4_t rev = __builtin_neon_vrev64q_s32(neon_vec_a);
        int32x4_t result = __builtin_neon_vaddq_s32(rev, neon_vec_b);
        
        *((int32x4_t*)&result_data[iter * 4 + 56]) = result;
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint64_t)result_data[i];
        /* Mix in some data from inputs to ensure all are used */
        checksum ^= (uint64_t)data_a[i] * 3;
        checksum ^= (uint64_t)data_b[i] * 5;
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    /* Additional complex shuffle with mixed types */
    printf("Running mixed-type vector operations\n");
    {
        /* Mix float and int vectors */
        float32x16_t float_vec = {0};
        float *float_data = (float*)&float_vec;
        for (int i = 0; i < 16; i++) {
            float_data[i] = (float)data_a[i] / 256.0f;
        }
        
        /* Create a mixed-type operation chain */
        int32x16_t int_mask = create_complex_mask(10);
        
        /* This may trigger complex expansion paths */
        int32x16_t mixed_result = __builtin_shuffle(
            *((int32x16_t*)&float_vec),  /* Reinterpret float as int */
            vec_a[0],
            int_mask
        );
        
        /* Use the result */
        vec_result[3] = vec_result[3] + mixed_result;
    }
    
    /* Final compiler barrier */
    asm volatile("" ::: "memory");
    
    return 0;
}
