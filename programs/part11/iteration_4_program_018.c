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

/* Complex mask computation that prevents constant propagation */
static inline int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (lcg_rand() ^ control) % 32;
    }
    
    /* Additional arithmetic to obscure pattern */
    for (int i = 0; i < 16; i++) {
        mask_ptr[i] = (mask_ptr[i] * 13 + 7) % 32;
        if (mask_ptr[i] < 0) mask_ptr[i] += 32;
    }
    
    return mask;
}

/* Chain of shuffles to accumulate operand count */
static inline int32x16_t shuffle_chain(int32x16_t a, int32x16_t b, 
                                       int32x16_t c, int32x16_t d,
                                       volatile int control) {
    /* First shuffle: 3 operands (a, b, mask1) */
    int32x16_t mask1 = compute_complex_mask(control);
    int32x16_t result1 = __builtin_shuffle(a, b, mask1);
    
    /* Second shuffle: 3 operands (c, d, mask2) */
    int32x16_t mask2 = compute_complex_mask(control + 1);
    int32x16_t result2 = __builtin_shuffle(c, d, mask2);
    
    /* Third shuffle mixing results: 3 operands */
    int32x16_t mask3 = compute_complex_mask(control + 2);
    int32x16_t result3 = __builtin_shuffle(result1, result2, mask3);
    
    /* Fourth shuffle with original inputs: 3 operands */
    int32x16_t mask4 = compute_complex_mask(control + 3);
    int32x16_t result4 = __builtin_shuffle(a, result3, mask4);
    
    /* Fifth shuffle creating dependency chain: 3 operands */
    int32x16_t mask5 = compute_complex_mask(control + 4);
    int32x16_t result5 = __builtin_shuffle(b, result4, mask5);
    
    /* Complex expression that may require many operands during expansion */
    return __builtin_shufflevector(result1, result2, result3, result4, result5,
                                   0, 17, 2, 19, 4, 21, 6, 23,
                                   8, 25, 10, 27, 12, 29, 14, 31);
}

/* Mixed-type permutation operations */
static inline float32x16_t mixed_type_permute(float32x16_t fa, float32x16_t fb,
                                              int32x16_t ia, int32x16_t ib,
                                              volatile int control) {
    /* Convert int vectors to float for mixing */
    float32x16_t fia = __builtin_convertvector(ia, float32x16_t);
    float32x16_t fib = __builtin_convertvector(ib, float32x16_t);
    
    /* Complex shuffle with mixed types */
    int32x16_t mask = compute_complex_mask(control);
    
    /* This complex expression may require many operands */
    float32x16_t result = __builtin_shuffle(fa, fb, mask);
    result = __builtin_shuffle(result, fia, compute_complex_mask(control + 100));
    result = __builtin_shuffle(result, fib, compute_complex_mask(control + 200));
    
    return result;
}

/* Conditional vector permutation */
static inline int32x16_t conditional_shuffle(int32x16_t a, int32x16_t b,
                                             int32x16_t c, int32x16_t d,
                                             volatile int selector) {
    int32x16_t mask1 = compute_complex_mask(selector);
    int32x16_t mask2 = compute_complex_mask(selector + 1000);
    
    int32x16_t shuffle1 = __builtin_shuffle(a, b, mask1);
    int32x16_t shuffle2 = __builtin_shuffle(c, d, mask2);
    
    /* Conditional selection between two shuffle results */
    int32x16_t condition = (a > b);
    return condition ? shuffle1 : shuffle2;
}

/* Inline assembly with many vector operands */
static inline void multi_operand_asm(int32x16_t *out, 
                                     int32x16_t in1, int32x16_t in2,
                                     int32x16_t in3, int32x16_t in4,
                                     int32x16_t in5, int32x16_t in6) {
    /* Hypothetical assembly with many vector operands */
    asm volatile (
        /* Multiple operations chained together */
        "vpaddd %%zmm0, %%zmm1, %%zmm2\n\t"
        "vpsubd %%zmm3, %%zmm2, %%zmm4\n\t"
        "vpmulld %%zmm5, %%zmm4, %%zmm6\n\t"
        "vpslld $2, %%zmm6, %0\n\t"
        : "=v" (*out)
        : "v" (in1), "v" (in2), "v" (in3), 
          "v" (in4), "v" (in5), "v" (in6)
        : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "memory"
    );
}

/* Target-specific builtins when available */
#ifdef __x86_64__
static inline int32x8_t x86_specific_shuffle(int32x8_t a, int32x8_t b) {
    /* Use x86-specific shuffle intrinsic */
    return __builtin_ia32_pshufd256(a, 0x1B);
}
#endif

#ifdef __ARM_NEON
static inline int32x4_t arm_specific_shuffle(int32x4_t a) {
    /* Use ARM-specific reversal intrinsic */
    return __builtin_neon_vrev64q_s32(a);
}
#endif

/* Horizontal sum for checksum */
static inline int64_t horizontal_sum_i32x16(int32x16_t v) {
    int64_t sum = 0;
    int32_t *ptr = (int32_t*)&v;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}

static inline double horizontal_sum_f32x16(float32x16_t v) {
    double sum = 0.0;
    float *ptr = (float*)&v;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    #define ARRAY_SIZE 1024
    int32_t int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (int32_t)lcg_rand();
        float_data[i] = (float)(lcg_rand() % 1000) / 100.0f;
    }
    
    /* Volatile control variables to prevent optimization */
    volatile int control_var = 42;
    volatile int loop_counter = 0;
    
    /* Result accumulators */
    int64_t int_checksum = 0;
    double float_checksum = 0.0;
    
    /* Kernel 1: Complex shuffle with computed mask */
    for (int i = 0; i < ARRAY_SIZE - 32; i += 16) {
        int32x16_t vec_a = *(int32x16_t*)&int_data[i];
        int32x16_t vec_b = *(int32x16_t*)&int_data[i + 16];
        
        /* Complex mask prevents constant propagation */
        int32x16_t mask = compute_complex_mask(control_var + i);
        
        /* This shuffle may expand to many operands */
        int32x16_t result = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Store and accumulate */
        *(int32x16_t*)&int_data[i] = result;
        int_checksum += horizontal_sum_i32x16(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 2: Chain of shuffles */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        int32x16_t vec_a = *(int32x16_t*)&int_data[i];
        int32x16_t vec_b = *(int32x16_t*)&int_data[i + 16];
        int32x16_t vec_c = *(int32x16_t*)&int_data[i + 32];
        int32x16_t vec_d = *(int32x16_t*)&int_data[i + 48];
        
        /* Chain of shuffles accumulates operand count */
        int32x16_t result = shuffle_chain(vec_a, vec_b, vec_c, vec_d, 
                                         control_var + i);
        
        /* Store result */
        *(int32x16_t*)&int_data[i] = result;
        int_checksum += horizontal_sum_i32x16(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Mixed type permutations */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        float32x16_t fvec_a = *(float32x16_t*)&float_data[i];
        float32x16_t fvec_b = *(float32x16_t*)&float_data[i + 16];
        int32x16_t ivec_a = *(int32x16_t*)&int_data[i + 32];
        int32x16_t ivec_b = *(int32x16_t*)&int_data[i + 48];
        
        /* Mixed type operations */
        float32x16_t result = mixed_type_permute(fvec_a, fvec_b, 
                                                ivec_a, ivec_b,
                                                control_var + i);
        
        /* Store and accumulate */
        *(float32x16_t*)&float_data[i] = result;
        float_checksum += horizontal_sum_f32x16(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Conditional permutations */
    for (int i = 0; i < ARRAY_SIZE - 64; i += 16) {
        int32x16_t vec_a = *(int32x16_t*)&int_data[i];
        int32x16_t vec_b = *(int32x16_t*)&int_data[i + 16];
        int32x16_t vec_c = *(int32x16_t*)&int_data[i + 32];
        int32x16_t vec_d = *(int32x16_t*)&int_data[i + 48];
        
        /* Data-dependent conditional shuffle */
        int32x16_t result = conditional_shuffle(vec_a, vec_b, vec_c, vec_d,
                                               control_var + i);
        
        /* Store result */
        *(int32x16_t*)&int_data[i] = result;
        int_checksum += horizontal_sum_i32x16(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 5: Inline assembly with many operands */
    for (int i = 0; i < ARRAY_SIZE - 96; i += 16) {
        int32x16_t vec1 = *(int32x16_t*)&int_data[i];
        int32x16_t vec2 = *(int32x16_t*)&int_data[i + 16];
        int32x16_t vec3 = *(int32x16_t*)&int_data[i + 32];
        int32x16_t vec4 = *(int32x16_t*)&int_data[i + 48];
        int32x16_t vec5 = *(int32x16_t*)&int_data[i + 64];
        int32x16_t vec6 = *(int32x16_t*)&int_data[i + 80];
        int32x16_t result;
        
        /* Assembly with many vector operands */
        multi_operand_asm(&result, vec1, vec2, vec3, vec4, vec5, vec6);
        
        /* Store result */
        *(int32x16_t*)&int_data[i] = result;
        int_checksum += horizontal_sum_i32x16(result);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Use target-specific builtins when available */
    #ifdef __x86_64__
    for (int i = 0; i < ARRAY_SIZE - 8; i += 8) {
        int32x8_t vec = *(int32x8_t*)&int_data[i];
        int32x8_t result = x86_specific_shuffle(vec, vec);
        *(int32x8_t*)&int_data[i] = result;
    }
    #endif
    
    #ifdef __ARM_NEON
    for (int i = 0; i < ARRAY_SIZE - 4; i += 4) {
        int32x4_t vec = *(int32x4_t*)&int_data[i];
        int32x4_t result = arm_specific_shuffle(vec);
        *(int32x4_t*)&int_data[i] = result;
    }
    #endif
    
    /* Final checksum output to prevent dead code elimination */
    printf("Integer checksum: %ld\n", (long)int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    /* Additional volatile use to ensure operations aren't optimized away */
    volatile int32_t final_check = int_data[0] + int_data[ARRAY_SIZE-1];
    (void)final_check;
    
    return 0;
}
